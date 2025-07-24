#!/usr/bin/env python3
import subprocess
import platform
import argparse
import serial
import time
import sys

def main():
    parser = argparse.ArgumentParser(description="Flash Ameba firmware and check logs for faults.")
    parser.add_argument('--image_exe', required=True, help='Path to image executable')
    parser.add_argument('--tools_path', required=True, help='Path to tools folder')
    parser.add_argument('--com_port', required=True, help='COM port')
    parser.add_argument('--board', required=True, help='Board name')
    parser.add_argument('--baud_rate', default=115200, type=int, help='Baud rate for serial monitor (default: 115200)')
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

    if result.returncode != 0:
        print(f"Flashing tool returned non-zero exit code: {result.returncode}")
        sys.exit(result.returncode)

    if "Bus Fault" in output or "bus fault" in output.lower():
        print("Detected HardFault in flashing log. Marking as failure.")
        sys.exit(1)

    print("Flashing completed successfully with no hard fault detected.")

    # === Start serial monitor ===
    print(f"Opening serial port {args.com_port} at {args.baud_rate} baud...")
    try:
        ser = serial.Serial(args.com_port, args.baud_rate, timeout=1)
        time.sleep(2)  # Give MCU time to reset after flash
        print("--- Serial monitor --- (Press CTRL+C to stop)")

        while True:
            line = ser.readline().decode('utf-8', errors='ignore').strip()
            if line:
                print(line)

    except KeyboardInterrupt:
        print("\nSerial monitor stopped by user.")

    except serial.SerialException as e:
        print(f"Serial error: {e}")
        sys.exit(1)

if __name__ == '__main__':
    main()