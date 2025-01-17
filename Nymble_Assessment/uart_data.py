"""
Author: Rajath B T
Date: 17/01/2025

"""

import serial
import time

def send_data(port: str, file_path: str, baud_rate: int = 2400, timeout: int = 5, chunk_size: int = 512) -> None:
    """
    Send the contents of a text file over the specified UART port in chunks.

    This function reads the content of a text file and transmits it via UART in
    defined chunk sizes. During transmission, it displays the real-time speed of
    data transfer in bits per second (bps).

    Args:
        port (str): The UART port to use for transmission (e.g., "COM4").
        file_path (str): Path to the text file containing the data to send.
        baud_rate (int): The UART communication baud rate (default: 2400).
        timeout (int): Timeout in seconds for the serial connection (default: 5).
        chunk_size (int): Size of each data chunk to send in bytes (default: 512).

    Returns:
        None
    """
    try:
        # Open the file and read its content
        with open(file_path, 'r', encoding='utf-8') as file:
            data_to_send = file.read()

        if not data_to_send:
            print("Error: The file is empty. Nothing to send.")
            return

        # Open the UART connection
        with serial.Serial(port, baud_rate, timeout=timeout) as ser:
            print(f"Connected to {port} at {baud_rate} baud rate.")

            total_bytes = len(data_to_send.encode('utf-8'))
            bytes_sent = 0
            start_time = time.time()

            for i in range(0, total_bytes, chunk_size):
                # Get the chunk of data to send
                chunk = data_to_send[i:i + chunk_size].encode('utf-8')
                ser.write(chunk)
                bytes_sent += len(chunk)

                # Calculate and display real-time transmission speed
                elapsed_time = time.time() - start_time
                if elapsed_time > 0:
                    transmission_speed_bps = (bytes_sent * 8) / elapsed_time
                    print(f"Sent {bytes_sent}/{total_bytes} bytes, Speed: {transmission_speed_bps:.2f} bits/s")

            ser.flush()  # Ensure all data is sent before closing
            print(f"Data from {file_path} sent successfully.")

            # Check for echo and suppress it
            response = ser.read_all().decode('utf-8', errors='ignore')
            if response.strip() == data_to_send.strip():
                print("Echo received and suppressed.")

    except FileNotFoundError:
        print(f"Error: File not found at {file_path}. Please check the path and try again.")
    except serial.SerialException as e:
        print(f"Serial Exception: {e}")
    except Exception as e:
        print(f"Unexpected error: {e}")
    finally:
        print("Disconnected.")

def receive_data(port: str, baud_rate: int = 2400, timeout: int = 5, output_file: str = "received.txt") -> None:
    """
    Continuously receive data from UART and append it to a file.

    This function listens for incoming data on the specified UART port and stores
    it in the specified output file. During the process, it displays the real-time
    data reception speed in bits per second (bps).

    Args:
        port (str): The UART port to use for data reception (e.g., "COM4").
        baud_rate (int): The UART communication baud rate (default: 2400).
        timeout (int): Timeout in seconds for reading data (default: 5).
        output_file (str): Path to the file where received data will be stored.

    Returns:
        None
    """
    try:
        # Open the UART connection
        with serial.Serial(port, baud_rate, timeout=timeout) as ser:
            print(f"Connected to {port} at {baud_rate} baud rate.")

            byte_count = 0
            start_time = time.time()

            while True:
                # Read available data
                if ser.in_waiting > 0:
                    received_data = ser.read(ser.in_waiting).decode('utf-8', errors='ignore')
                    byte_count += len(received_data.encode('utf-8'))

                    # Append received data to the file
                    with open(output_file, 'a', encoding='utf-8') as file:
                        file.write(received_data)

                    # Calculate and display reception speed
                    elapsed_time = time.time() - start_time
                    if elapsed_time > 0:
                        reception_speed_bps = (byte_count * 8) / elapsed_time
                        print(f"{received_data.strip()}   Speed: {reception_speed_bps:.2f} bits/s")

    except serial.SerialException as e:
        print(f"Serial Exception: {e}")
    except KeyboardInterrupt:
        print("\nStopped receiving data.")
    except Exception as e:
        print(f"Unexpected error: {e}")
    finally:
        print("Disconnected.")

if __name__ == "__main__":
    """
    Entry point for UART file transmission and data reception operations.

    Configuration:
        - Replace 'COM4' with the UART port for your device.
        - Specify the file path for the data to send and the output file for received data.
    """
    UART_PORT = "COM4"            # Replace with your UART port
    FILE_PATH = "send.txt"        # File to send over UART
    OUTPUT_FILE = "received.txt"  # File to store received data
    UART_BAUD_RATE = 2400         # Default UART baud rate
    UART_TIMEOUT = 5              # Default timeout for UART

    # Transmit file content via UART
    send_data(UART_PORT, FILE_PATH, UART_BAUD_RATE, UART_TIMEOUT)

    time.sleep(1)

    # Receive data continuously and store it in a file
    receive_data(UART_PORT, UART_BAUD_RATE, UART_TIMEOUT, OUTPUT_FILE)
