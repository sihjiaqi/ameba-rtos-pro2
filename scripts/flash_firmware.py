#!/usr/bin/env python3
import subprocess
import sys
import platform
import argparse

def main():
    parser = argparse.ArgumentParser(description="Flash Ameba firmware and check logs for faults.")
    parser.add_argument('--image_exe', required=True, help='Path to image executable')
    parser.add_argument('--tools_path', required=True, help='Path to tools folder')
    parser.add_argument('--com_port', required=True, help='COM port')
    parser.add_argument('--board', required=True, help='Board name')
    args = parser.parse_args()

    cmd = [
        args.image_exe,
        args.tools_path,
        args.com_port,
        args.board,
        'Enable',
        'Disable',
        '2000000',
    ]

    system_name = platform.system().lower()
    if system_name == 'windows':
        cmd.extend([
            'uartfwburn.exe',
            'Auto_Flash_Pro2_V3.3_win.exe'
        ])
    elif system_name == 'darwin':
        cmd.extend([
            'uartfwburn.darwin',
            'Auto_Flash_Pro2_V3.3_macos.exe'
        ])
    elif system_name == 'linux':
        cmd.extend([
            'uartfwburn.linux',
            'Auto_Flash_Pro2_V3.3_linux.exe'
        ])
    else:
        raise RuntimeError(f"Unsupported OS: {system_name}")

    # Append the remaining args
    cmd.extend([
        '0x60000',
        '0x460000',
        '0x530000'
    ])

    print(f"Running: {' '.join(cmd)}")

    try:
        result = subprocess.run(
            cmd,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
            check=False
        )
    except Exception as e:
        print(f"Error running flash command: {e}")
        sys.exit(1)

    output = result.stdout
    print("--- Flashing log start ---")
    print(output)
    print("--- Flashing log end ---")

    # If the command returned non-zero, fail immediately
    if result.returncode != 0:
        print(f"Flashing tool returned non-zero exit code: {result.returncode}")
        sys.exit(result.returncode)

    # If the log contains a hard fault marker, fail too
    if "Bus Fault" in output or "bus fault" in output.lower():
        print("Detected HardFault in flashing log. Marking as failure.")
        sys.exit(1)

    print("Flashing completed successfully with no hard fault detected.")

if __name__ == '__main__':
    main()