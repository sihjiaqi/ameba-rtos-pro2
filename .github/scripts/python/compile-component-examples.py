import os
import sys
import subprocess
import shutil

PROJECT_DIR = os.path.abspath(os.path.join(os.path.abspath(__file__), "..", "..", "..", "..", "project", "realtek_amebapro2_v0_example"))
GCC_RELEASE_DIR = os.path.join(PROJECT_DIR, "GCC-RELEASE")
BUILD_DIR = os.path.join(GCC_RELEASE_DIR, "build")
BIN_OUTPUT_DIR = os.path.join(PROJECT_DIR, "bin_outputs")

def run(cmd, cwd=None):
    print(f"Running: {cmd}")
    result = subprocess.run(cmd, shell=True, text=True, capture_output=True, check=True, cwd=cwd)
    print(result.stdout)
    if result.stderr:
        print(result.stderr, file=sys.stderr)

def build_example(example):
    print(f"Building {example}...")

    build_dir = os.path.join(GCC_RELEASE_DIR, f"build_{example}")
    os.makedirs(build_dir, exist_ok=True)
    os.chdir(build_dir)

    # Run cmake config
    run(f'cmake .. -G"Unix Makefiles" -DCMAKE_TOOLCHAIN_FILE="../toolchain.cmake" -DEXAMPLE={example}', cwd=build_dir)

    # Build target
    run('cmake --build . --target flash -j4', cwd=build_dir)
    
    # Copy built binary file to output directory
    built_bin_name = "flash_ntz.bin"
    built_bin_path = os.path.join(build_dir, built_bin_name)
    output_bin_dir = os.path.join(BIN_OUTPUT_DIR, f"{example}")
    output_bin_path = os.path.join(output_bin_dir, built_bin_name)
    os.makedirs(output_bin_dir, exist_ok=True)
    shutil.copyfile(built_bin_path, output_bin_path)
    
    # Copy firmware.bin and firmware_isp_iq.bin to output directory
    for firmware_file in ["firmware.bin", "firmware_isp_iq.bin"]:
        firmware_path = os.path.join(build_dir, firmware_file)
        if os.path.isfile(firmware_path):
            shutil.copyfile(firmware_path, os.path.join(output_bin_dir, firmware_file))
        else:
            print(f"Warning: {firmware_file} not found in {build_dir}")

    # Clean for next build
    run('make clean', cwd=build_dir)

def main():
    try:
        # Get the list of examples passed
        examples_to_build = sys.argv[1:]

        # Build all the examples sequentially within the same batch
        for example in examples_to_build:
            build_example(example)

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