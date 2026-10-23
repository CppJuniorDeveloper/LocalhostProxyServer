# Boost.Asio Proxy & Log Server

A high-performance, asynchronous TCP Proxy and Log Server implemented in modern C++ (C++20) using **Boost.Asio** coroutines.

## Features

*   **Asynchronous Architecture:** Built entirely on C++20 coroutines (`co_await`, `co_spawn`) for non-blocking I/O.
*   **TCP Proxy:** Forwards traffic between clients and a target server. Supports graceful shutdown and connection monitoring.
*   **Log Server:** A dedicated server that accepts client connections and records incoming messages into separate log files.
*   **Robust Error Handling:** Distinguishes between normal client disconnects (`eof`, `connection_reset`) and actual network errors.
*   **Thread-Safe Logging:** (Optional) Custom header-only logger with ANSI colors and millisecond precision timestamps.
*   **Target Monitoring:** The proxy continuously pings the target server and shuts down automatically if the target becomes unreachable.

## Technologies Used

*   **Language:** C++20
*   **Networking:** Boost.Asio
*   **Build System:** CMake
*   **Logging:** `std::format` (C++20), `std::filesystem`
*   **IDE:** Visual Studio Code / CLion
*   **Testing:** Python 3 (for the test client)

## 📂 Project Structure

```text
├── include/
│   ├── core/           # ProxyServer, ProxySession, ClientLogger
│   ├── utils/          # Header-only utilities (Logger, Parse)
│   └── BoostHeaders.hpp
├── src/
│   ├── core/           # Implementation of ProxyServer & Session
│   └── apps/           # Entry points (main.cpp for proxy, main.cpp for server)
├── test_client/        # Python scripts for testing
│   └── client.py       # Test client that connects to the server
├── CMakeLists.txt
└── README.md
