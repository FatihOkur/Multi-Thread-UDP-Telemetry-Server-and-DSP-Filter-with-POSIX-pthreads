# Multi-Threaded UDP Telemetry Server & DSP Filter

A high-performance, POSIX-compliant C application designed to receive, process, and log telemetry data in real-time over UDP. The project implements a multi-threaded architecture using a thread-safe circular buffer (Ring Buffer) and features a Digital Signal Processing (DSP) moving average filter to clean noisy sensor data.

## 🚀 Key Features

* **Strict Standard C (C99):** Built entirely in C99 without utilizing C++ references. All data manipulation is handled via standard C pointers.
* **Multi-Threading (POSIX pthreads):**
  * **Receiver Thread:** Actively listens for UDP datagrams (`recvfrom`) on port 8080 and pushes the parsed data into the ring buffer.
  * **Worker Thread:** Pops data from the ring buffer, applies a DSP filter, prints the results, and logs them to a CSV file.
* **Thread-Safe Ring Buffer:** Implemented using `pthread_mutex_t` and `pthread_cond_t`. Follows wait-for graph principles to completely avoid deadlocks and handles spurious wakeups safely without busy-waiting.
* **DSP Filter:** A Simple Moving Average (SMA/FIR) filter designed to smooth out high-frequency noise from incoming sensor signals.
* **Real-time Logging:** Automatically records timestamps, raw data, and filtered data into a `telemetry_log.csv` file for post-processing and analysis.
* **Mock Telemetry Client:** Includes a Python-based simulator (`mock_client.py`) that generates and sends a noisy 1Hz sine wave over UDP at a rate of 50 packets per second (50 Hz).

## 📁 Project Structure

```text
.
├── Makefile                # Compilation configuration (GCC, -pthread, -std=c99)
├── README.md               # Project documentation
├── include/
│   ├── dsp_filter.h        # DSP filter definitions
│   └── udp_server.h        # Ring buffer & UDP server definitions
├── scripts/
│   └── mock_client.py      # Python simulator (Noisy sine wave generator)
└── src/
    ├── dsp_filter.c        # DSP Moving Average implementation
    ├── main.c              # Application entry point & Thread management
    └── udp_server.c        # Thread-safe buffer & Socket initialization
```

## 🛠️ Prerequisites

* **OS:** Linux (Ubuntu, Debian, etc.) or Windows Subsystem for Linux (WSL)
* **Compiler:** GCC (GNU Compiler Collection)
* **Build Tool:** Make
* **Scripting:** Python 3.x (for the mock client)

## ⚙️ Build and Run Instructions

### 1. Build the Server
Navigate to the project root directory and run `make`:
```bash
make
```
This will compile the source code and generate the `telemetry_server` executable.

### 2. Start the Telemetry Server
Run the compiled executable:
```bash
./telemetry_server
```
*The server will start listening for incoming UDP packets on port 8080.*

### 3. Start the Mock Client
Open a **new terminal tab/window**, navigate to the project root, and run the Python simulator:
```bash
python3 scripts/mock_client.py
```
*The client will begin sending 50 packets per second to `127.0.0.1:8080`.*

## 📊 Output & Logging

Once the client is running, the server terminal will display the real-time processing of the data:
```text
[DSP] TS: 1788705138094 | Raw:   0.303 | Filtered:   0.250
[DSP] TS: 1788705138114 | Raw:  -0.039 | Filtered:   0.153
[DSP] TS: 1788705138135 | Raw:   0.277 | Filtered:   0.148
```

Simultaneously, the data is appended to `telemetry_log.csv` in the project root:
```csv
Timestamp,Raw,Filtered
1788706545966,1.084003,1.184337
1788706545989,1.319119,1.179994
1788706546011,0.825094,1.186479
```

## 🧹 Cleanup

To remove the compiled object files and the executable, run:
```bash
make clean
```
