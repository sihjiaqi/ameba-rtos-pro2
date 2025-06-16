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
TOOLCHAIN_BIN = TOOLCHAIN_DIR / "asdk-10.3.0/mingw32/newlib/bin"
CMAKE_INSTALLER = Path.home() / "cmake-installer.msi"
CMAKE_PATH = Path("C:/Program Files/CMake/bin")
MSYS_7Z = Path.home() / "msys64_v10_3.7z"
MSYS_ROOT = Path.home() / "msys64_v10_3/msys64"
MSYS_HOME = MSYS_ROOT / "home" / os.getenv("USERNAME")
MSYS_CMD = MSYS_ROOT / "msys2_shell.cmd"
BASHRC_PATH = MSYS_HOME / ".bashrc"
POST_FILE = MSYS_ROOT / "etc/post-install/05-home-dir.post"

def run(cmd, shell=False):
    print(f"Running: {' '.join(cmd)}")
    subprocess.run(cmd, check=True, shell=shell)

# Download file from URL to specified destination
def download_file(url, dest):
    print(f"Downloading from {url} to {dest} ...")
    urllib.request.urlretrieve(url, dest)

def download_extract_msys():
    print("Downloading MSYS...")
    download_file(MSYS_URL, MSYS_7Z)
    # Use 7z to extract the msys archive
    run(["7z", "x", str(MSYS_7Z), f"-o{str(Path.home())}", "-y"])

def download_extract_toolchain():
    print("Downloading toolchain...")
    TOOLCHAIN_DIR.mkdir(parents=True, exist_ok=True)
    download_file(TOOLCHAIN_URL, TOOLCHAIN_ZIP)
    
    with zipfile.ZipFile(TOOLCHAIN_ZIP, 'r') as zip_ref:
        zip_ref.extractall(TOOLCHAIN_DIR)
    
    print(f"Toolchain extracted to: {TOOLCHAIN_DIR}")
                
# Set the home directory for MSYS
def set_home_directory():
    print("Setting up MSYS home directory...")
    shell_lines = [
        f'export HOME="{MSYS_HOME}"',
        'if [ ! -d "$HOME" ]; then',
        '    mkdir -p "$HOME"',
        '    echo "Created home directory: $HOME"',
        'fi\n'
    ]
    with open(POST_FILE, "w") as f:
        f.write("\n".join(shell_lines))

def launch_msys_shell():
    print("Launching MSYS shell...")
    cmd = [
        "cmd.exe", "/C",
        f"{MSYS_CMD}",
        "-defterm", "-mingw32", "-no-start",
        "-shell", "bash", "-c", "exit"
    ]
    run(cmd, shell=True)

def install_cmake():
    print("Installing CMake...")
    download_file(CMAKE_URL, CMAKE_INSTALLER)
    # Install cmake silently and without restarting the system after installation
    run(["msiexec", "/i", str(CMAKE_INSTALLER), "/quiet", "/norestart"], shell=True)

def append_to_github_path():
    github_path = os.environ.get("GITHUB_PATH")
    if github_path:
        lines = [str(CMAKE_PATH), str(TOOLCHAIN_BIN)]
        with open(github_path, "a") as f:
            for p in lines:
                f.write(p + "\n")

def main():
    try:
        download_extract_msys()
        download_extract_toolchain()
        set_home_directory()
        launch_msys_shell() 
        install_cmake()
        append_to_github_path()

    except Exception as e:
        print(f"An error occurred: {e}")
        exit(1)

if __name__ == "__main__":
    main()