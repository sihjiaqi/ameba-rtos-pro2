import re
import os
import sys
import subprocess

PROJECT_DIR = os.path.abspath(os.path.join(os.path.abspath(__file__), "..", "..", "..", "..", "project", "realtek_amebapro2_v0_example"))
SRC_DIR = os.path.join(PROJECT_DIR, "src", "fast_inf_example")
GCC_RELEASE_DIR = os.path.join(PROJECT_DIR, "GCC-RELEASE")
BUILD_DIR = os.path.join(GCC_RELEASE_DIR, "build")
BIN_OUTPUT_DIR = os.path.join(PROJECT_DIR, "bin_outputs")
TOOLCHAIN_FILE = os.path.join(GCC_RELEASE_DIR, "toolchain.cmake")
SRC_FILE = os.path.join(SRC_DIR, "fast_inf_example.c")

# Utility functions for file operations
def read_file(path):
    with open(path, 'r', encoding='utf-8') as f:
        return f.read()

def write_file(path, content):
    with open(path, 'w', encoding='utf-8') as f:
        f.write(content)

def read_lines(path):
    with open(path, 'r', encoding='utf-8') as f:
        return f.readlines()

def write_lines(path, lines):
    with open(path, 'w', encoding='utf-8') as f:
        f.writelines(lines)

# Functions to modify specific files
# Step 1: Enable FCS mode
def enable_fcs_in_sensor_h(path):
    print("Enabling FCS mode in sensor.h...")
    content = read_file(path)
    content = re.sub(r'#define\s+ENABLE_FCS\s+\d+', '#define ENABLE_FCS\t1', content)
    write_file(path, content)
    
# Step 2: Enable ch0 FCS mode and modify resolution settings
def update_nested_struct_block(content, struct_name, sub_struct_key, updates):
    print(f"Updating nested struct block for {struct_name} with key {sub_struct_key}...")
    pattern = re.compile(
        rf'({re.escape(struct_name)}\s*=\s*\{{.*?\.{re.escape(sub_struct_key)}\s*=\s*\{{)(.*?)(\}}\s*,)', 
        re.DOTALL
    )

    def replacer(match):
        before = match.group(1)
        block = match.group(2)
        after = match.group(3)

        for key, val in updates.items():
            block = re.sub(
                rf'(\.{re.escape(key)}\s*=\s*)[^,]+',
                rf'\1{val}',
                block
            )

        return before + block + after

    return pattern.sub(replacer, content)


def update_video_user_boot(path):
    print("Updating video_user_boot.c...")
    content = read_file(path)

    # Updates inside video_params[STREAM_Vx]
    stream_updates = {
        'STREAM_V1': {
            'width': '1920',
            'height': '1080',
            'fps': '20',
            'fcs': '1',
        },
        'STREAM_V3': {
            'width': '128',
            'height': '128',
            'fps': '20',
            'fcs': '1',
        },
        'STREAM_V4': {
            'width': '320',
            'height': '320',
            'fps': '20',
            'fcs': '1',
        }
    }

    for stream_key, updates in stream_updates.items():
        content = update_nested_struct_block(content, 'video_boot_stream', f'video_params[{stream_key}]', updates)

    # Other flat params (not inside a nested struct)
    flat_updates = {
        '.video_drop_frame[STREAM_V1]': '3',
        '.video_drop_frame[STREAM_V4]': '3',
        '.fcs_channel': '3',
        '.extra_video_enable': '1',
    }

    for param, val in flat_updates.items():
        content = re.sub(
            rf'({re.escape(param)}\s*=\s*)[^,;]+',
            rf'\1{val}',
            content
        )

    write_file(path, content)

# Step 3: Modify video slot settings of ch2 and ch4
def update_video_boot(path):
    print("Updating video_boot.c...")
    content = read_file(path)
    content = re.sub(r'static\s+unsigned\s+char\s+video_boot_slot_num\[[0-9]+\]\s*=\s*\{[^}]+\}',
                     'static unsigned char video_boot_slot_num[5] = {2, 2, 3, 2, 3};',
                     content)
    write_file(path, content)
    
# Step 4: Disable wifi connection
def disable_wifi_connection(file_path):
    print("Disabling WiFi connection in fast_inf_example.c...")
    lines = read_lines(file_path)
    in_setup = False
    updated_lines = []

    for line in lines:
        stripped = line.strip()

        # Detect start of setup function
        if not in_setup and re.match(r'^void\s+setup\s*\(\s*\)\s*{', stripped):
            in_setup = True
            updated_lines.append(line)
            continue
        # Exit when hitting closing brace of setup
        if in_setup and stripped == '}':
            in_setup = False
            updated_lines.append(line)
            continue
        # Inside setup: comment if, else, endif
        if in_setup and re.match(r'^\s*#(if|else|endif)', stripped):
            if not stripped.startswith('//'):
                updated_lines.append('// ' + line)
            else:
                updated_lines.append(line)
        else:
            updated_lines.append(line)

    write_lines(file_path, updated_lines)

# Step 5: Enable copying the NV12 image to the MD queue
def enable_nv12_copy(path):
    print("Enabling NV12 copy in fast_inf_example.c...")
    content = read_file(path)
    pattern = re.compile(
    r'static\s+video_params_t\s+video_v3_params\s*=\s*\{.*?\};',
    re.DOTALL
    )
    replacement = 'static video_params_t video_v3_params = {\n\t.use_static_addr = 0,\n};'
    content = pattern.sub(replacement, content)
    write_file(path, content)
    
# Step 6: Enable waiting MD result functioN
def enable_wait_md_result(path):
    print("Enabling WAIT_MD_RESULT in fast_inf_example.c...")
    content = read_file(path)
    content = re.sub(r'//\s*#define WAIT_MD_RESULT\s+1', '#define WAIT_MD_RESULT 1', content)
    write_file(path, content)

# Step 7: Apply lightweight NN models
# def write_nn_config(path):
#     config = {
#         "msg_level": 3,
#         "PROFILE": ["FWFS"],
#         "FWFS": {
#             "files": ["yolo_fastest_320x320"]
#         },
#         "yolo_fastest_320x320": {
#             "name": "yolo_fastest.nb",
#             "source": "binary",
#             "file": "yolo_fastest_1.1_320x320_u8.nb"
#         }
#     }
#     with open(path, 'w', encoding='utf-8') as f:
#         json.dump(config, f, indent=4)

# Step 8: Modify baud rate settings
def set_uart_baudrate(path):
    print("Setting UART baud rate to 3000000 in main.c...")
    content = read_file(path)
    content = re.sub(r'baud_rate\s*=\s*\d+;', 'baud_rate = 3000000;', content)
    write_file(path, content)
    
# Step 9: Adjust flash speed settings
def set_flash_speed(path):
    print("Setting flash speed to 125MHz in hal_spic.h...")
    content = read_file(path)
    content = re.sub(r'#define\s+HIGH_SPEED_FLASH\s+\w+', '#define HIGH_SPEED_FLASH FLASH_SPEED_125MHz', content)
    write_file(path, content)
  
def setup():
    enable_fcs_in_sensor_h("project/realtek_amebapro2_v0_example/inc/sensor.h")
    update_video_user_boot("component/video/driver/RTL8735B/video_user_boot.c")
    update_video_boot("component/video/driver/RTL8735B/video_boot.c")
    disable_wifi_connection("project/realtek_amebapro2_v0_example/src/fast_inf_example/fast_inf_example.c")
    enable_nv12_copy("project/realtek_amebapro2_v0_example/src/fast_inf_example/fast_inf_example.c")
    enable_wait_md_result("project/realtek_amebapro2_v0_example/src/fast_inf_example/fast_inf_example.c")
    set_uart_baudrate("project/realtek_amebapro2_v0_example/src/main.c")
    set_flash_speed("component/soc/8735b/fwlib/rtl8735b/include/hal_spic.h")

def run(cmd):
    print(f"Running: {cmd}")
    result = subprocess.run(cmd, shell=True, text=True, capture_output=True, check=True)
    print(result.stdout)
    if result.stderr:
        print(result.stderr, file=sys.stderr)

def build():
    os.makedirs(BUILD_DIR, exist_ok=True)
    os.chdir(BUILD_DIR)

    # Run cmake config
    run(f'cmake .. -G"Unix Makefiles" -DCMAKE_TOOLCHAIN_FILE={TOOLCHAIN_FILE} -DFAST_INF_EXAMPLE=on')

    # Build target flash
    run('cmake --build . --target flash -j4')

    # Clean after build
    run('make clean')

def main():
    try:
        print("Starting setup...")
        setup()
        print("Setup completed.")

        print("Starting build...")
        build()
        print("Build completed successfully.")

    except FileNotFoundError as e:
        print(f"File error: {e.filename} not found.")
        exit(1)

    except subprocess.CalledProcessError as e:
        print(f"Subprocess failed with return code {e.returncode}")
        print(f"Command: {e.cmd}")
        print(f"Output:\n{e.output}")
        print(f"Error Output:\n{e.stderr}")
        exit(e.returncode)

    except Exception as e:
        print(f"Unexpected error: {str(e)}")
        exit(1)

if __name__ == "__main__":
    main()