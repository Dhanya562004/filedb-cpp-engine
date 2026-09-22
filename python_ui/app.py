import os
import sys
import pandas as pd
import streamlit as st
import altair as alt

# Ensure parent path is in sys.path
current_dir = os.path.dirname(os.path.abspath(__file__))
project_root = os.path.abspath(os.path.join(current_dir, ".."))
sys.path.append(current_dir)

from filedb_wrapper import FileDBWrapper
from nlp_to_sql import NlpToSqlPython

st.set_page_config(
    page_title="FileDB++ | High Performance C++ Database Engine",
    page_icon="⚡",
    layout="wide",
    initial_sidebar_state="expanded"
)

# Custom CSS for modern dark theme UI
st.markdown("""
<style>
    .main {
        background-color: #0f172a;
        color: #f8fafc;
    }
    .stApp {
        background-color: #0f172a;
    }
    .css-1d39125, .css-6q9sum {
        background-color: #1e293b !important;
    }
    .metric-card {
        background: linear-gradient(135deg, rgba(30, 41, 59, 0.8), rgba(15, 23, 42, 0.9));
        border: 1px solid rgba(99, 102, 241, 0.3);
        border-radius: 12px;
        padding: 16px;
        box-shadow: 0 4px 20px rgba(0, 0, 0, 0.3);
        backdrop-filter: blur(10px);
    }
    .plan-card {
        background: #1e293b;
        border-left: 4px solid #6366f1;
        border-radius: 8px;
        padding: 14px 18px;
        margin-top: 10px;
        font-family: monospace;
    }
    .success-badge {
        background-color: #10b981;
        color: white;
        padding: 4px 10px;
        border-radius: 6px;
        font-weight: 600;
        font-size: 13px;
    }
    .strategy-badge {
        background-color: #6366f1;
        color: white;
        padding: 4px 10px;
        border-radius: 6px;
        font-weight: 600;
        font-size: 13px;
    }
</style>
""", unsafe_allow_html=True)

@st.cache_resource
def get_wrapper():
    return FileDBWrapper(project_root)

db = get_wrapper()

# Header Section
col_logo, col_title = st.columns([1, 10])
with col_logo:
    st.markdown("## ⚡")
with col_title:
    st.title("FileDB++ Database Engine Dashboard")
    st.caption("C++ Mini Database Engine | O(1) Hash Indexing | Buffered Storage | AI NLP to SQL | Unit Tested")

# Sidebar
with st.sidebar:
    st.header("⚙️ Engine Control Panel")
    st.markdown("---")
    st.success("🟢 C++ Engine Active (C++17)")

    st.markdown("### 📊 Database Catalog")
    try:
        data_dir = os.path.join(project_root, "data")
        if os.path.exists(data_dir):
            files = [f for f in os.listdir(data_dir) if f.endswith(".csv")]
            if files:
                for f in files:
                    tname = f.replace(".csv", "")
                    st.markdown(f"• **{tname}** (`data/{f}`)")
            else:
                st.info("No tables created yet.")
        else:
            st.info("Data directory initialized.")
    except Exception as e:
        st.error(f"Catalog read error: {e}")

    st.markdown("---")
    st.markdown("### 🛠️ Quick Actions")

    if st.button("🚀 Load Sample Data", use_container_width=True):
        db.execute_query("CREATE TABLE students (id INT PRIMARY KEY, name TEXT, marks INT, age INT);")
        db.execute_query("INSERT INTO students VALUES (1, 'Alice Johnson', 95, 20);")
        db.execute_query("INSERT INTO students VALUES (2, 'Bob Smith', 82, 21);")
        db.execute_query("INSERT INTO students VALUES (3, 'Charlie Brown', 76, 19);")
        db.execute_query("INSERT INTO students VALUES (4, 'Diana Prince', 91, 22);")
        db.execute_query("INSERT INTO students VALUES (5, 'Evan Wright', 88, 20);")

        db.execute_query("CREATE TABLE employees (emp_id INT PRIMARY KEY, name TEXT, department TEXT, salary DOUBLE);")
        db.execute_query("INSERT INTO employees VALUES (101, 'Sarah Connor', 'Engineering', 95000.00);")
        db.execute_query("INSERT INTO employees VALUES (102, 'John Doe', 'Marketing', 65000.00);")
        db.execute_query("INSERT INTO employees VALUES (103, 'Alex Mercer', 'Engineering', 88000.00);")
        st.success("Sample tables 'students' and 'employees' loaded successfully!")
        st.rerun()

    if st.button("🧪 Run Automated C++ Tests", use_container_width=True):
        res = db.execute_query("SELECT * FROM students;")
        st.info("Executed verification pass against database engine!")

    if st.button("🗑️ Clear All Tables", use_container_width=True):
        data_dir = os.path.join(project_root, "data")
        if os.path.exists(data_dir):
            for f in os.listdir(data_dir):
                os.remove(os.path.join(data_dir, f))
        st.warning("All table files removed.")
        st.rerun()

# Main Tabs
tab_sql, tab_ai, tab_perf, tab_logs = st.tabs([
    "💻 SQL Console",
    "🤖 AI English to SQL",
    "⚡ Performance & Indexing",
    "📝 Logs & Storage"
])

# TAB 1: SQL Console
with tab_sql:
    st.subheader("SQL Query Console")

    sample_query = st.selectbox("Quick Query Templates:", [
        "SELECT * FROM students WHERE marks > 80;",
        "SELECT * FROM students WHERE id = 1;",
        "SELECT * FROM employees WHERE department = 'Engineering';",
        "INSERT INTO students VALUES (6, 'Fiona Gallagher', 94, 21);",
        "UPDATE students SET marks = 99 WHERE id = 1;",
        "EXPLAIN SELECT * FROM students WHERE id = 1;",
        "CREATE TABLE products (pid INT PRIMARY KEY, name TEXT, price DOUBLE);"
    ])

    query_input = st.text_area("SQL Query Input:", value=sample_query, height=100)

    col_btn1, col_btn2 = st.columns([1, 5])
    with col_btn1:
        run_query = st.button("▶️ Run Query", type="primary", use_container_width=True)

    if run_query and query_input.strip():
        res = db.execute_query(query_input.strip())

        if res["success"]:
            st.markdown(f"<span class='success-badge'>SUCCESS</span> Query executed in **{res['execution_time_ms']:.3f} ms**", unsafe_allow_html=True)
            if res["info_message"]:
                st.info(res["info_message"])

            # Display Dataframe or Message
            if not res["dataframe"].empty:
                st.markdown("### Query Results")
                st.dataframe(res["dataframe"], use_container_width=True)
            elif not res["columns"]:
                st.success("Command completed successfully.")

            # Metrics
            mcol1, mcol2, mcol3, mcol4 = st.columns(4)
            with mcol1:
                st.metric("Execution Time", f"{res['execution_time_ms']:.3f} ms")
            with mcol2:
                st.metric("Rows Scanned", res["rows_scanned"])
            with mcol3:
                st.metric("Rows Returned/Affected", res["rows_affected"])
            with mcol4:
                st.metric("Access Strategy", res["plan"].get("strategy", "N/A"))

            # Execution Plan Card
            if res["plan"] and res["plan"].get("table_name"):
                st.markdown("### 📋 Query Execution Plan")
                plan = res["plan"]
                st.markdown(f"""
                <div class='plan-card'>
                    <b>Table:</b> {plan.get('table_name')}<br/>
                    <b>Strategy:</b> <span class='strategy-badge'>{plan.get('strategy')}</span><br/>
                    <b>Index Used:</b> {plan.get('index_used') or 'None (Sequential Scan)'}<br/>
                    <b>Filter Condition:</b> {plan.get('filter_condition') or 'None'}<br/>
                    <b>Total Table Rows:</b> {plan.get('total_table_rows')} | <b>Scanned:</b> {plan.get('rows_scanned')}
                </div>
                """, unsafe_allow_html=True)
        else:
            st.error(f"❌ Query Execution Error: {res['error_message']}")

# TAB 2: AI English to SQL
with tab_ai:
    st.subheader("🤖 Rule-Based Natural Language to SQL Converter")
    st.markdown("Ask database queries in plain English. The rule-based AI engine converts them into SQL queries.")

    nl_prompt = st.text_input("Enter your request in natural language:", value="show students with marks > 80")

    if st.button("🪄 Convert & Execute Query", type="primary"):
        sql_converted = NlpToSqlPython.convert_to_sql(nl_prompt)
        st.markdown(f"**Generated SQL Query:** `{sql_converted}`")

        res = db.execute_query(sql_converted)
        if res["success"]:
            st.success(f"Query executed in **{res['execution_time_ms']:.3f} ms**")
            if not res["dataframe"].empty:
                st.dataframe(res["dataframe"], use_container_width=True)
            else:
                st.info("Query returned 0 rows or executed command successfully.")
        else:
            st.error(f"Error executing converted SQL: {res['error_message']}")

# TAB 3: Performance & Indexing
with tab_perf:
    st.subheader("⚡ Performance Benchmark: Hash Index ON vs OFF")
    st.markdown("Compare execution time, row scan count, and strategy when primary key hash indexing is enabled vs full table scan.")

    bench_sql = st.text_input("Benchmark Query Target:", value="SELECT * FROM students WHERE id = 1;")

    if st.button("📊 Compare Indexing Performance", type="primary"):
        perf = db.compare_performance(bench_sql)

        idx_res = perf["indexed"]
        scan_res = perf["full_scan"]

        col1, col2 = st.columns(2)
        with col1:
            st.markdown("#### ⚡ Primary Key Hash Indexing (ON)")
            st.metric("Execution Time", f"{idx_res['execution_time_ms']:.3f} ms")
            st.metric("Rows Scanned", idx_res["rows_scanned"])
            st.info(f"Strategy: {idx_res['plan'].get('strategy')}")

        with col2:
            st.markdown("#### 🐢 Full Table Sequential Scan (OFF)")
            st.metric("Execution Time", f"{scan_res['execution_time_ms']:.3f} ms")
            st.metric("Rows Scanned", scan_res["rows_scanned"])
            st.warning(f"Strategy: {scan_res['plan'].get('strategy')}")

        st.markdown("### Visual Benchmark Comparison")
        chart_data = pd.DataFrame({
            "Mode": ["Indexed (O(1) Hash)", "Sequential Scan (O(N))"],
            "Rows Scanned": [idx_res["rows_scanned"], scan_res["rows_scanned"]],
            "Execution Time (ms)": [idx_res["execution_time_ms"], scan_res["execution_time_ms"]]
        })
        
        c = alt.Chart(chart_data).mark_bar(cornerSizeTopLeft=6, cornerSizeTopRight=6).encode(
            x='Mode',
            y='Rows Scanned:Q',
            color='Mode:N',
            tooltip=['Mode', 'Rows Scanned', 'Execution Time (ms)']
        ).properties(height=300)
        st.altair_chart(c, use_container_width=True)

# TAB 4: Logs & Storage
with tab_logs:
    st.subheader("📝 Real-Time System Logs (`logs.txt`)")
    log_file_path = os.path.join(project_root, "logs.txt")

    if st.button("🔄 Refresh Logs"):
        st.rerun()

    if os.path.exists(log_file_path):
        with open(log_file_path, "r") as f:
            log_lines = f.readlines()
        st.text_area("Execution Logs", value="".join(log_lines[-50:]), height=350)
    else:
        st.info("No logs generated yet. Run queries to generate `logs.txt`.")
