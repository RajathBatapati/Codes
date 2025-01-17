
# UART-to-EEPROM Storage and Retrieval System

This project demonstrates how to receive data over UART, store it in EEPROM (using NVS), and retrieve it for transmission when required. It uses FreeRTOS tasks to manage UART communication and EEPROM storage efficiently.

---

## **Features**
1. **UART Communication**:
   - Configured with customizable baud rate, data bits, stop bits, and parity.
   - Receives data over UART, processes it, and stores it in EEPROM.

2. **EEPROM (NVS) Storage**:
   - Splits received data into fixed-size chunks.
   - Stores each chunk in EEPROM with a unique key.

3. **Data Retrieval**:
   - Retrieves stored data from EEPROM.
   - Sends the retrieved data back over UART.

4. **Event-Driven Design**:
   - Uses a semaphore to signal when new data is received and ready for retrieval.

---

## **How It Works**

## Use uart_data.py to send and receive data from your PC to ESP32.

### **1. System Initialization**
- **UART Initialization**:
  - Configures UART using the `init_uart` function.
  - UART is set up with the following parameters:
    - **Baud rate**: 2400
    - **Data bits**: 8
    - **Parity**: None
    - **Stop bits**: 1
- **EEPROM Initialization**:
  - Initializes the ESP32's NVS flash storage.
  - Erases the flash storage if no free pages are available.

### **2. Data Reception**
- **Receive Task (`uart_receive_task`)**:
  - Reads data from UART using `uart_read_bytes`.
  - Logs the received data.
  - Stores the data in EEPROM by splitting it into chunks (defined by `CHUNK_SIZE`).
  - Signals the retrieval task using a FreeRTOS semaphore.

### **3. Data Storage**
- **Chunked Storage in EEPROM**:
  - Data is divided into fixed-size chunks (`CHUNK_SIZE = 101` bytes).
  - Each chunk is assigned a unique key (e.g., `uart_data_0`, `uart_data_1`).
  - The `eeprom_store_chunks` function handles the storage:
    - Logs the chunk being stored.
    - Writes the chunk to EEPROM using `nvs_set_blob`.

### **4. Data Retrieval and Transmission**
- **Retrieve and Send Task (`uart_retrieve_and_send_task`)**:
  - Waits for the semaphore signal indicating new data is available.
  - Retrieves stored chunks from EEPROM using the `retrieve_and_send_chunks` function.
  - Logs and sends the retrieved data back over UART.

---

## **How to Use**

### **1. Hardware Setup**
- Connect the ESP32's UART pins:
  - **TX Pin**: GPIO 17
  - **RX Pin**: GPIO 16
- Connect the ESP32 to a serial terminal for debugging and testing.

### **2. Software Setup**
- Clone the repository or copy the source files to your ESP32 project directory.
- Make sure the ESP-IDF environment is properly set up.

### **3. Flash the Code**
- Compile and flash the code:
  ```bash
  idf.py build flash monitor
  ```

### **4. Test the System**
- Send data to the ESP32 over UART using a serial terminal or another device.
- Observe the logs in the monitor:
  - Data received via UART.
  - Data stored in EEPROM in chunks.
  - Data retrieved and sent back over UART.

---

## **Code Explanation**

### **Key Functions**

1. **`init_uart()`**:
   - Initializes UART with predefined configurations.

2. **`init_eeprom()`**:
   - Initializes NVS flash for storing data.

3. **`eeprom_store_chunks()`**:
   - Stores received data in fixed-size chunks in EEPROM.

4. **`retrieve_and_send_chunks()`**:
   - Retrieves stored data from EEPROM and sends it back over UART.

5. **`uart_receive_task()`**:
   - Continuously receives data from UART and stores it in EEPROM.
   - Signals the retrieval task when new data is ready.

6. **`uart_retrieve_and_send_task()`**:
   - Waits for the semaphore signal and retrieves and sends data.

---

## **Logs**

### **Receiving Data**
```
I (1234) UART_COMM: Received 512 bytes from UART
I (1235) UART_COMM: Storing chunk under key 'uart_data_0': Hello, this is a test.
I (1236) UART_COMM: Stored chunk under key 'uart_data_0' (101 bytes)
```

### **Retrieving and Sending Data**
```
I (1237) UART_COMM: Retrieved chunk 'uart_data_0': Hello, this is a test.
I (1238) UART_COMM: Sent chunk 'uart_data_0'
```

---

## **Configuration**

### **UART Settings**
- **Baud Rate**: 2400
- **Data Bits**: 8
- **Parity**: None
- **Stop Bits**: 1

### **Chunk Size**
- Defined as `CHUNK_SIZE = 101` bytes.

### **NVS Storage**
- Each chunk is stored under a unique key in NVS.

---

## **Customization**
- **Adjust Chunk Size**:
  - Modify `#define CHUNK_SIZE` to set a different chunk size.

- **UART Settings**:
  - Adjust the `uart_config_t` structure in `init_uart()` for different baud rates or configurations.

