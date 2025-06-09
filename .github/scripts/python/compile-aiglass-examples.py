import re
import os
import subprocess

PROJECT_DIR = "project/realtek_amebapro2_v0_example"
BUILD_DIR = os.path.join(PROJECT_DIR, "GCC-RELEASE", "build")
TOOLCHAIN_FILE = "../toolchain.cmake"

# Step 1: Modify fatfs_sdcard_api.c
def modify_fatfs_sdcard_api_c(file_path):
    with open(file_path, 'r', encoding='utf-8') as file:
        lines = file.readlines()

    brace_count = 0      # To track the number of braces to know when we exit the function
    in_init_func = False # To track if we are inside the fatfs_sd_init function
    is_modified = False  # To check if any changes were made 

    # Iterate through each line to find the fatfs_sd_init function
    for i, line in enumerate(lines):
        # Check if we are entering the fatfs_sd_init function
        if not in_init_func and line.strip().startswith('int fatfs_sd_init('):
            in_init_func = True
            brace_count += line.count('{') - line.count('}')
            continue

        if in_init_func:
            brace_count += line.count('{') - line.count('}')
            if 'fatfs_sd_close();' in line.strip():
                lines[i] = line.replace('fatfs_sd_close();', '// fatfs_sd_close();')
                is_modified = True
    
            # If we reach the end of the function, exit the loop
            if brace_count == 0:
                break

    if is_modified:
        # Write the modified lines back to the file
        with open(file_path, 'w', encoding='utf-8') as file:
            file.writelines(lines)
        print(f"Successfully updated {file_path}")
    else:
        print("fatfs_sd_close() not found inside fatfs_sd_init()")

# Step 2: Modify module_mp4.c
def modify_module_mp4_c(file_path):
    with open(file_path, 'r', encoding='utf-8') as f:
        content = f.read()

    # 1. Update define macros
    content = re.sub(r'^\s*#define\s+FATFS_SD_CARD', r'//#define FATFS_SD_CARD', content, flags=re.MULTILINE)
    content = re.sub(r'^\s*#define\s+FATFS_RAM', r'//#define FATFS_RAM', content, flags=re.MULTILINE)
    content = re.sub(r'^\s*//#define\s+VFS_ENABLE', r'#define VFS_ENABLE', content, flags=re.MULTILINE)

    # 2. Comment out vfs_user_unregister in mp4_destroy
    content = re.sub(r'^\s*vfs_user_unregister\("sd", VFS_FATFS, VFS_INF_SD\);',
                    r'//vfs_user_unregister("sd", VFS_FATFS, VFS_INF_SD);',
                    content, flags=re.MULTILINE)

    # 3. Comment out vfs_init and vfs_user_register
    content = re.sub(r'^\s*vfs_init\(NULL\);', r'//vfs_init(NULL);', content, flags=re.MULTILINE)
    content = re.sub(
        r'^\s*if\s*\(vfs_user_register\("sd",\s*VFS_FATFS,\s*VFS_INF_SD\)\s*<\s*0\)\s*{\s*[\r\n]+'
        r'^\s*goto\s+mp4_create_fail;\s*[\r\n]+^\s*}', 
        lambda m: '\n'.join('//'+line if line.strip() != '' else line for line in m.group().splitlines()),
        content, flags=re.MULTILINE
    )

    # Write the modified content back to the file
    with open(file_path, 'w', encoding='utf-8') as f:
        f.write(content)

    print(f"Successfully updated {file_path}")


# Step 3: Modify user_boot.c to disable bl_log_cust_ctrl
def modify_user_boot_c(file_path):
    with open(file_path, 'r', encoding='utf-8') as f:
        content = f.read()

    content = re.sub(r'\bbl_log_cust_ctrl\s*=\s*ENABLE\b', 'bl_log_cust_ctrl = DISABLE', content)
    
    # Write modified content back to the file
    with open(file_path, 'w', encoding='utf-8') as f:
        f.write(content)

    print(f"Successfully updated {file_path}")

# Step 4: Modify video_user_boot.c
def modify_video_user_boot_c(file_path):
    # Read the content
    with open(file_path, "r", encoding="utf-8") as f:
        content = f.read()

    # 1. Uncomment ISP_CONTROL_TEST
    content = re.sub(r'//\s*(#define ISP_CONTROL_TEST)', r'\1', content)

    # 2. Modify STREAM_V1
    content = re.sub(
        r'(?<=video_params\[STREAM_V1\] = \{)(.*?)(?=\},)', 
        r"""
            \n\t\t.stream_id = STREAM_ID_V1,
            .type = CODEC_H264,
            .resolution = 0,
            .width  = 176,
            .height = 144,
            .bps = 1024 * 1024,
            .fps = 15,
            .gop = 15,
            .rc_mode = 2,
            .minQp = 25,
            .maxQp = 48,
            .jpeg_qlevel = 0,
            .rotation = 0,
            .out_buf_size = V1_ENC_BUF_SIZE,
            .out_rsvd_size = 0,
            .direct_output = 0,
            .use_static_addr = 0,
            .fcs = 1
        """, 
        content, flags=re.DOTALL
    )

    # 3. Modify STREAM_V2 and video_snapshot
    content = re.sub(
        r'video_snapshot\[STREAM_V2\] = \d+;',
        'video_snapshot[STREAM_V2] = 1;',
        content
    )

    content = re.sub(
        r'(?<=video_params\[STREAM_V2\] = \{)(.*?)(?=\},)',
        r"""
            \n\t\t.stream_id = STREAM_ID_V2,
            .type = CODEC_H264,
            .resolution = 0,
            .width = sensor_params[USE_SENSOR].sensor_width,
            .height = sensor_params[USE_SENSOR].sensor_height,
            .bps = 12 * 1024 * 1024,
            .fps = sensor_params[USE_SENSOR].sensor_fps,
            .gop = sensor_params[USE_SENSOR].sensor_fps,
            .rc_mode = 2,
            .minQp = 25,
            .maxQp = 48,
            .jpeg_qlevel = 0,
            .rotation = 0,
            .out_buf_size = V2_ENC_BUF_SIZE,
            .out_rsvd_size = 0,
            .direct_output = 0,
            .use_static_addr = 0,
            .fcs = 0
        """,
        content, flags=re.DOTALL
    )

    # 4. Set STREAM_V4 to 0
    content = re.sub(
        r'video_enable\[STREAM_V4\] = \d+;',
        'video_enable[STREAM_V4] = 0;',
        content
    )

    # 5. Set ISP control block
    old_isp_block = r'(video_boot_stream\.init_isp_items\..*?;)+'
    new_isp_block = """
    video_boot_stream.init_isp_items.enable = 1;
    video_boot_stream.init_isp_items.init_brightness = 0;
    video_boot_stream.init_isp_items.init_contrast = 50;
    video_boot_stream.init_isp_items.init_flicker = 1;
    video_boot_stream.init_isp_items.init_hdr_mode = 0;
    video_boot_stream.init_isp_items.init_mirrorflip = 0xf0;
    video_boot_stream.init_isp_items.init_saturation = 50;
    video_boot_stream.init_isp_items.init_wdr_level = 50;
    video_boot_stream.init_isp_items.init_wdr_mode = 2;
    video_boot_stream.init_isp_items.init_mipi_mode = 0;
    """
    content = re.sub(old_isp_block, new_isp_block, content, flags=re.DOTALL)

    # Write modified content back to the file
    with open(file_path, "w", encoding="utf-8") as f:
        f.write(content)

    print(f"Successfully updated {file_path}")

# Step 5: Modify sensor.h for SENSOR_SC5356
def modify_sensor_h(file_path):
    with open(file_path, 'r', encoding='utf-8') as f:
        lines = f.readlines()

    new_lines = []
    in_sen_id = False
    in_manual_iq = False

    # Flags to ensure each update happens only once
    sensor_params_modified = False
    sen_id_modified = False
    use_sensor_modified = False
    manual_iq_modified = False
    enable_fcs_modified = False

    for line in lines:
        # 1. Modify sensor_params for SENSOR_SC5356
        if not sensor_params_modified and line.strip().startswith('[SENSOR_SC5356]'):
            line = re.sub(r'\{.*?\}', '{2592, 1944, 24}', line)
            sensor_params_modified = True

        # 2. Replace SENSOR_GC2053 by SENSOR_SC5356 in sen_id array
        if not sen_id_modified:
            if line.strip().startswith('static const unsigned char sen_id'):
                in_sen_id = True
            if in_sen_id:
                if 'SENSOR_GC2053' in line:
                    line = line.replace('SENSOR_GC2053', 'SENSOR_SC5356')
                    sen_id_modified = True
                if '};' in line:
                    in_sen_id = False

        # 3. Set USE_SENSOR to SENSOR_SC5356
        if not use_sensor_modified and line.strip().startswith('#define USE_SENSOR'):
            line = '#define USE_SENSOR      	SENSOR_SC5356'
            use_sensor_modified = True

        # 4. Replace iq_gc2053 by iq_sc5356 in manual_iq
        if not manual_iq_modified:
            if line.strip().startswith('static const      char manual_iq'):
                in_manual_iq = True
            if in_manual_iq:
                if 'iq_gc2053' in line:
                    line = line.replace('iq_gc2053', 'iq_sc5356')
                    manual_iq_modified = True
                if '};' in line:
                    in_manual_iq = False

        # 5. Set ENABLE_FCS to 1
        if not enable_fcs_modified and line.strip().startswith('#define ENABLE_FCS'):
            line = '#define ENABLE_FCS      	1\n'
            enable_fcs_modified = True

        new_lines.append(line)

    with open(file_path, 'w', encoding='utf-8') as f:
        f.writelines(new_lines)

    print(f"Successfully updated {file_path}")

def setup():
    modify_fatfs_sdcard_api_c("component/file_system/fatfs/fatfs_sdcard_api.c")
    modify_module_mp4_c("component/media/mmfv2/module_mp4.c")
    modify_user_boot_c("component/soc/8735b/misc/platform/user_boot.c")
    modify_video_user_boot_c("component/video/driver/RTL8735B/video_user_boot.c")
    modify_sensor_h("project/realtek_amebapro2_v0_example/inc/sensor.h")

def run(cmd):
    print(f"Running: {cmd}")
    result = subprocess.run(cmd, shell=True, text=True, capture_output=True)
    print(result.stdout)
    if result.returncode != 0:
        print("ERROR:")
        print(result.stderr)
        raise subprocess.CalledProcessError(result.returncode, cmd)

def build():
    os.makedirs(BUILD_DIR, exist_ok=True)
    os.chdir(BUILD_DIR)

    # Fix permissions for all .linux files
    mp_dir = os.path.join(PROJECT_DIR, "GCC-RELEASE", "mp")
    try:
        run(f'find {mp_dir} -name "*.linux" -exec chmod +x {{}} \\;')
        print("Fixed permissions for all .linux files")
    except subprocess.CalledProcessError:
        print("Warning: Could not fix permissions for some build tools")

    # Run cmake config with ai_glass scenario
    run(f'cmake .. -G"Unix Makefiles" -DCMAKE_TOOLCHAIN_FILE={TOOLCHAIN_FILE} -DSCENARIO=ai_glass')

    # Build target flash
    run('cmake --build . --target flash -j4')

    # Clean after build
    run('make clean')

if __name__ == "__main__":
    setup()
    build()