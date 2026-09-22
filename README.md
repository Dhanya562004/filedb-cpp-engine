# FileDB++ | High Performance C++ Mini Database Engine

[![C++17](https://img.shields.io/badge/C++-17-blue.svg)](https://en.cppreference.com/w/cpp/17)
[![Build Status](https://img.shields.io/badge/build-passing-brightgreen.svg)]()
[![Streamlit UI](https://img.shields.io/badge/Streamlit-UI-FF4B4B.svg)](https://streamlit.io/)
[![License](https://img.shields.io/badge/License-MIT-green.svg)](LICENSE)

**FileDB++** is an enterprise-grade, highly performant, modular C++ Mini Database Engine designed to demonstrate core Software Engineering, Systems Programming, Data Structures & Algorithms, and AI Integration concepts.

It features **$O(1)$ Primary Key and Secondary Hash Indexing**, **Buffered Disk Stream I/O (64KB Buffer)**, a **Query Execution Plan Analyzer**, **Structured File Logging to `logs.txt`**, an **Automated C++ Unit Test Suite**, a **Rule-Based AI Natural Language-to-SQL Converter**, and a **Python Streamlit Web Interface**.

---

## 🏛️ System Architecture

```mermaid
graph TD
    UI[Python Streamlit Web UI / CLI REPL] -->|SQL / English Query| NLP[Rule-Based AI NLP Converter]
    NLP -->|Transliterated SQL| PARSER[SQL Tokenizer & AST Parser]
    PARSER -->|AST Node| ENG[Query Execution Engine]
    ENG -->|Query Timing & Cost| QA[Query Analyzer & Planner]
    QA -->|Choose Strategy| STRAT{Access Strategy}
    STRAT -->|Primary Key / Col Hash| IDX[Index Manager - O(1) Hash Map]
    STRAT -->|Sequential Scan| SCAN[Full Table Row Scanner]
    IDX -->|Row Offset| SM[Storage Manager - 64KB Buffered Stream]
    SCAN -->|Scan Rows| SM
    SM <-->|Read / Write CSV & Meta| DISK[(Data Storage - data/)]
    ENG -->|Log Events & Timings| LOG[Structured Logger - logs.txt]
```

---

## 🚀 Key Features & Performance Metrics

### 1. ⚡ Performance & Memory Optimizations
- **High-Resolution Timing**: Measures execution time down to sub-millisecond precision (`std::chrono::high_resolution_clock`).
- **Stream Buffering**: Employs custom 64 KB memory stream buffers (`rdbuf()->pubsetbuf`) for rapid disk read/write operations.
- **Zero-Copy Architecture**: Uses move semantics (`std::move`), `const` references, and string views to avoid unnecessary memory allocations during row filtering.
- **Efficient Memory Containers**: Utilizes `std::unordered_map` and vector reservation (`std::vector::reserve`) to minimize dynamic memory reallocation overhead.

### 2. 🔑 Hash Indexing System
- **Primary Key Indexing**: Maintains an $O(1)$ `std::unordered_map` index for table primary keys, drastically accelerating `SELECT`, `UPDATE`, and `DELETE` queries.
- **Secondary Column Indexing**: Supports single-column hash indices for instant filtering on frequently queried non-primary key attributes.
- **Index Benchmarking**: Allows real-time toggling (`INDEX ON` / `INDEX OFF`) to directly compare and evaluate execution speedups.

| Query Strategy | Complexity | Average Execution Time | Rows Scanned |
| :--- | :--- | :--- | :--- |
| **Primary Key Hash Index** | **$O(1)$** | **$< 0.05$ ms** | **1 row** |
| **Secondary Hash Index** | **$O(k)$** | **$< 0.10$ ms** | **$k$ matched rows** |
| **Full Table Scan** | **$O(N)$** | **$> 1.50$ ms** | **$N$ total rows** |

### 3. 🔍 Query Execution Plan & Analyzer
- Automatically determines access paths: `PRIMARY_KEY_HASH_INDEX`, `SECONDARY_HASH_INDEX`, or `FULL_TABLE_SCAN`.
- Tracks exact rows scanned vs rows returned/affected.
- Supports `EXPLAIN <SQL>` query command.

### 4. 📝 Structured File Logging (`logs.txt`)
- Writes timestamped execution logs (`YYYY-MM-DD HH:MM:SS.fff`) containing query status (`SUCCESS` / `ERROR`), duration in ms, scanned row counts, and detailed error messages.

### 5. 🤖 AI Natural Language Extension
- Translates natural language queries into SQL without external APIs:
  - `"show students with marks > 80"` $\rightarrow$ `SELECT * FROM students WHERE marks > 80;`
  - `"find employees where salary >= 50000"` $\rightarrow$ `SELECT * FROM employees WHERE salary >= 50000;`
  - `"delete student with id = 5"` $\rightarrow$ `DELETE FROM student WHERE id = 5;`

### 6. 🧪 Automated Unit Test Suite
- Comprehensive C++ unit tests covering `CREATE`, `INSERT`, `SELECT`, `UPDATE`, `DELETE`, Primary Key Uniqueness constraint enforcement, and Indexing benchmark validation.

### 7. 🌐 Python + Streamlit Web Interface
- Interactive query console with SQL syntax highlighting.
- Plain English AI Assistant tab.
- Query plan cards and real-time execution timing graphs.
- Live `logs.txt` and raw table file inspector.

---

## 📁 Repository Folder Structure

```
filedb-cpp-engine/
├── include/
│   └── db/
│       ├── Models.hpp           # Core Data Models: Value, Row, Column, Table, AST, Catalog
│       ├── StorageManager.hpp   # Buffered file I/O & CSV table persistence
│       ├── IndexManager.hpp     # O(1) Hash & PK Indexing System
│       ├── Logger.hpp           # Structured Logger writing to logs.txt
│       ├── QueryAnalyzer.hpp    # Query execution planner & strategy generator
│       ├── Parser.hpp           # SQL Tokenizer & AST Parser
│       ├── ExecutionEngine.hpp  # Execution Engine, timing & JSON serialization
│       └── NlpToSql.hpp         # Rule-based AI NLP to SQL converter
├── src/
│   ├── Models.cpp
│   ├── StorageManager.cpp
│   ├── IndexManager.cpp
│   ├── Logger.cpp
│   ├── QueryAnalyzer.cpp
│   ├── Parser.cpp
│   ├── ExecutionEngine.cpp
│   ├── NlpToSql.cpp
│   ├── main.cpp                 # Interactive CLI REPL & JSON IPC mode
│   └── test_runner.cpp          # Automated C++ Unit Test Runner
├── python_ui/
│   ├── app.py                   # Streamlit Web UI application
│   ├── filedb_wrapper.py        # Python IPC wrapper for C++ executable
│   └── nlp_to_sql.py            # Python rule-based NLP helper
├── data/                        # Persistent table CSV files & catalog metadata
├── logs.txt                     # Execution & Error log file
├── CMakeLists.txt               # Cross-platform CMake build file
├── Makefile                     # Standard GCC Makefile
├── requirements.txt             # Python dependencies
└── README.md                    # Project documentation
```

---

## 🛠️ Compilation & Execution Guide

### Option A: Using `make` (Linux / macOS / MinGW)

```bash
# Build main database engine and unit test runner
make

# Run automated unit test suite
make test

# Launch interactive CLI REPL
./filedb
```

### Option B: Direct Compilation with `g++` (Windows & Linux)

**Windows (MinGW g++):**
```powershell
g++ -std=c++17 -O2 -Iinclude src/Models.cpp src/StorageManager.cpp src/IndexManager.cpp src/Logger.cpp src/QueryAnalyzer.cpp src/Parser.cpp src/ExecutionEngine.cpp src/NlpToSql.cpp src/main.cpp -o filedb.exe
g++ -std=c++17 -O2 -Iinclude src/Models.cpp src/StorageManager.cpp src/IndexManager.cpp src/Logger.cpp src/QueryAnalyzer.cpp src/Parser.cpp src/ExecutionEngine.cpp src/NlpToSql.cpp src/test_runner.cpp -o test_runner.exe

# Run unit tests
.\test_runner.exe

# Run interactive CLI
.\filedb.exe
```

**Linux / macOS:**
```bash
g++ -std=c++17 -O2 -Iinclude src/Models.cpp src/StorageManager.cpp src/IndexManager.cpp src/Logger.cpp src/QueryAnalyzer.cpp src/Parser.cpp src/ExecutionEngine.cpp src/NlpToSql.cpp src/main.cpp -o filedb
g++ -std=c++17 -O2 -Iinclude src/Models.cpp src/StorageManager.cpp src/IndexManager.cpp src/Logger.cpp src/QueryAnalyzer.cpp src/Parser.cpp src/ExecutionEngine.cpp src/NlpToSql.cpp src/test_runner.cpp -o test_runner

# Run unit tests
./test_runner

# Run interactive CLI
./filedb
```

---

## 🌐 Running & Deploying Streamlit UI

1. **Install Python dependencies:**
   ```bash
   pip install -r requirements.txt
   ```

2. **Launch Streamlit Web App:**
   ```bash
   streamlit run python_ui/app.py
   ```

3. **Streamlit Cloud Deployment:**
   - Push this repository to GitHub.
   - Connect your repository to [Streamlit Community Cloud](https://streamlit.io/cloud).
   - Set the main file path to `python_ui/app.py`.
   - The Python wrapper (`filedb_wrapper.py`) will automatically compile the native C++ binary on the host environment upon initial startup!

---

## 📄 License
This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.