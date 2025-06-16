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
        build()

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