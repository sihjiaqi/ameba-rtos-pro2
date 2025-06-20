import os
import zipfile
import subprocess
import urllib.request
from pathlib import Path

MSYS_URL = "https://github.com/Ameba-AIoT/ameba-tool-rtos-pro2/releases/download/msys64_v10_3/msys64_v10_3.7z"
CMAKE_URL = "https://github.com/Kitware/CMake/releases/download/v3.20.0-rc1/cmake-3.20.0-rc1-windows-x86_64.msi"
TOOLCHAIN_URL = "https://github.com/Ameba-AIoT/ameba-toolchain/releases/download/V10.3.0-amebe-rtos-pro2/asdk-10.3.0-mingw32-newlib-build-3633-x86_64.zip"

TOOLCHAIN_DIR = Path.home() / "toolchain"
TOOLCHAIN_ZIP = TOOLCHAIN_DIR / "toolchain.zip"
TOOLCHAIN_BIN = TOOLCHAIN_DIR / "asdk-10.3.0" / "mingw32" / "newlib" / "bin"
CMAKE_INSTALLER = Path.home() / "cmake-installer.msi"
# Use Windows path for CMake install location (usually default)
CMAKE_PATH_WIN = Path("C:/Program Files/CMake/bin")

MSYS_7Z = Path.home() / "msys64_v10_3.7z"
MSYS_ROOT = Path.home() / "msys64_v10_3" / "msys64"
MSYS_HOME = MSYS_ROOT / "home" / os.getenv("USERNAME", "defaultuser")
MSYS_CMD = MSYS_ROOT / "msys2_shell.cmd"
BASHRC_PATH = MSYS_HOME / ".bashrc"
POST_FILE = MSYS_ROOT / "etc" / "post-install" / "05-home-dir.post"
BASH_PATH = MSYS_ROOT / "usr/bin" / "bash.exe"

def run(cmd, shell=False):
    print(f"Running: {' '.join(cmd) if isinstance(cmd, list) else cmd}")
    subprocess.run(cmd, check=True, shell=shell)

def download_file(url, dest):
    if dest.exists():
        print(f"File already exists at {dest}, skipping download.")
        return
    print(f"Downloading from {url} to {dest} ...")
    urllib.request.urlretrieve(url, dest)

def download_extract_msys():
    print("Downloading MSYS...")
    download_file(MSYS_URL, MSYS_7Z)
    print("Extracting MSYS archive with 7z...")
    run(["7z", "x", str(MSYS_7Z), f"-o{str(Path.home())}", "-y"])

def download_extract_toolchain():
    print("Downloading toolchain...")
    TOOLCHAIN_DIR.mkdir(parents=True, exist_ok=True)
    download_file(TOOLCHAIN_URL, TOOLCHAIN_ZIP)
    print("Extracting toolchain archive...")
    with zipfile.ZipFile(TOOLCHAIN_ZIP, 'r') as zip_ref:
        zip_ref.extractall(TOOLCHAIN_DIR)
    print(f"Toolchain extracted to: {TOOLCHAIN_DIR}")

def windows_path_to_msys(win_path: Path) -> str:
    win_path_str = str(win_path.resolve())
    drive = win_path_str[0].lower()
    path_rest = win_path_str[2:].replace("\\", "/")
    return f"/{drive}{path_rest}"

def set_home_directory():
    print("Setting up MSYS home directory...")
    shell_lines = [
        f'export HOME="{windows_path_to_msys(MSYS_HOME)}"',
        'if [ ! -d "$HOME" ]; then',
        '    mkdir -p "$HOME"',
        '    echo "Created home directory: $HOME"',
        'fi',
        ''
    ]
    POST_FILE.parent.mkdir(parents=True, exist_ok=True)
    with open(POST_FILE, "w") as f:
        f.write("\n".join(shell_lines))

def set_cmake_path():
    print("Appending CMake path to .bashrc...")
    cmake_path_msys = windows_path_to_msys(CMAKE_PATH_WIN)
    lines = [
        f'export PATH="{cmake_path_msys}:$PATH"',
        ''
    ]
    BASHRC_PATH.parent.mkdir(parents=True, exist_ok=True)
    with open(BASHRC_PATH, "a") as f:
        f.write("\n".join(lines))

def set_toolchain_path():
    print("Appending toolchain path to .bashrc...")
    toolchain_path_msys = windows_path_to_msys(TOOLCHAIN_BIN)
    lines = [
        f'if [ -d "{toolchain_path_msys}" ]; then',
        '    echo "Toolchain directory exists, adding to PATH"',
        f'    export PATH="{toolchain_path_msys}:$PATH"',
        'fi',
        ''
    ]
    with open(BASHRC_PATH, "a") as f:
        f.write("\n".join(lines))

def install_cmake():
    print("Installing CMake silently...")
    download_file(CMAKE_URL, CMAKE_INSTALLER)
    run(["msiexec", "/i", str(CMAKE_INSTALLER), "/quiet", "/norestart"], shell=True)

def launch_msys_shell_test():
    print("Launching MSYS shell to verify environment...")
    # Run bash and print PATH and cmake version
    cmd = [
        "cmd.exe", "/C",
        f'"{MSYS_CMD}" -defterm -mingw32 -no-start -shell bash -c "echo PATH=$PATH && cmake --version"'
    ]
    subprocess.run(" ".join(cmd), shell=True)

def main():
    try:
        download_extract_msys()
        download_extract_toolchain()
        set_home_directory()
        install_cmake()
        set_cmake_path()
        set_toolchain_path()
        launch_msys_shell_test()
        print(f"Bootstrap completed successfully.")
    except Exception as e:
        print(f"An error occurred: {e}")
        exit(1)

if __name__ == "__main__":
    main()