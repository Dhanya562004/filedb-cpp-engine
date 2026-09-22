# 🚀 FileDB++ – High-Performance C++ Database Engine with AI Interface

## 📌 Overview

FileDB++ is a high-performance file-based database engine built in C++, enhanced with a Python + Streamlit interface for interactive querying and AI-assisted execution.

It supports SQL-like queries, indexing for fast lookups, execution plan analysis, and natural language to SQL conversion.

---

## 🌐 Live Demo

👉 https://filedb-cpp-engine-vgmavcfzfwdxue9fkbvlne.streamlit.app/

---

## ✨ Key Features

- ⚡ C++ Core Engine with optimized file-based storage
- 🧠 SQL-like Query Processing (SELECT, INSERT, UPDATE, DELETE)
- 🚀 Indexing System (O(1) lookups using unordered_map)
- 📊 Execution Plan Analyzer (FULL_TABLE_SCAN vs INDEX_LOOKUP)
- ⏱️ High-resolution Performance Tracking
- 🪵 Query Logging (logs.txt)
- 🧪 Unit Testing Suite (test_runner.exe)
- 🤖 Natural Language → SQL (AI-based)
- 🌐 Streamlit Web Interface

---

## 🏗️ Architecture

```
User Input (NL / SQL)
        ↓
NLP to SQL Converter
        ↓
Query Parser
        ↓
Execution Engine
   ├── Index Manager
   ├── Storage Manager
   └── Query Analyzer
        ↓
Results + Logs
        ↓
Streamlit UI
```

---

## 🛠️ Tech Stack

- C++
- Python
- Streamlit
- STL (unordered_map, vector)
- CMake / Makefile
- Git

---

## 📂 Project Structure

```
FileDB++/
├── src/
├── include/
├── python_ui/
├── data/
├── logs.txt
├── filedb.exe
├── test_runner.exe
├── requirements.txt
└── README.md
```

---

## ▶️ Run Locally

```bash
git clone https://github.com/Dhanya562004/filedb-cpp-engine.git
cd filedb-cpp-engine
pip install -r requirements.txt
streamlit run python_ui/app.py
```

---

## 🧪 Run C++ Engine

```bash
.\filedb.exe
```

Run tests:

```bash
.\test_runner.exe
```

---

## 📊 Sample Queries

SQL:
```sql
SELECT * FROM students;
SELECT * FROM students WHERE marks > 80;
```

Natural Language:
```
show students with marks greater than 80
```

---

## 📈 Resume Highlight

Built a high-performance file-based database engine in C++ with SQL-like query execution, indexing for O(1) lookups, execution plan analysis, and an AI-powered natural language interface deployed via Streamlit.

---

## 📬 Author

Dhanya K  
B.Tech AIML (2026)  
https://github.com/Dhanya562004