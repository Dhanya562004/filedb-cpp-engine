import re

class NlpToSqlPython:
    @staticmethod
    def is_natural_language(text):
        clean = text.strip().lower()
        prefixes = ("show", "get", "find", "list", "fetch", "display", "remove", "count")
        return any(clean.startswith(p) for p in prefixes)

    @staticmethod
    def convert_to_sql(nl_query):
        clean = nl_query.strip()
        if not clean:
            return ""

        # Remove trailing dot or semicolon
        if clean.endswith(".") or clean.endswith(";"):
            clean = clean[:-1].strip()

        # Pattern 1: "show/get/find/list/fetch [all] <table> with/where <col> <op> <val>"
        # Example: "show students with marks > 80" -> "SELECT * FROM students WHERE marks > 80;"
        m1 = re.match(r"^(show|get|find|list|fetch|display)\s+(all\s+)?([a-zA-Z0-9_]+)\s+(with|where|having)\s+([a-zA-Z0-9_]+)\s*(=|!=|>|<|>=|<=)\s*(.+)$", clean, re.IGNORECASE)
        if m1:
            tbl, col, op, val = m1.group(3), m1.group(5), m1.group(6), m1.group(7)
            return f"SELECT * FROM {tbl} WHERE {col} {op} {val};"

        # Pattern 2: "show/get/find [all] <table>"
        # Example: "show all students" -> "SELECT * FROM students;"
        m2 = re.match(r"^(show|get|find|list|fetch|display)\s+(all\s+)?([a-zA-Z0-9_]+)$", clean, re.IGNORECASE)
        if m2:
            tbl = m2.group(3)
            return f"SELECT * FROM {tbl};"

        # Pattern 3: "delete/remove <table> with/where <col> <op> <val>"
        # Example: "delete student with id = 5" -> "DELETE FROM student WHERE id = 5;"
        m3 = re.match(r"^(delete|remove)\s+(from\s+)?([a-zA-Z0-9_]+)\s+(with|where)\s+([a-zA-Z0-9_]+)\s*(=|!=|>|<|>=|<=)\s*(.+)$", clean, re.IGNORECASE)
        if m3:
            tbl, col, op, val = m3.group(3), m3.group(5), m3.group(6), m3.group(7)
            return f"DELETE FROM {tbl} WHERE {col} {op} {val};"

        # Pattern 4: "insert into <table> <val1>, <val2>..."
        m4 = re.match(r"^insert\s+into\s+([a-zA-Z0-9_]+)\s+(values\s*)?\(?(.+)\)?$", clean, re.IGNORECASE)
        if m4:
            tbl, vals = m4.group(1), m4.group(3)
            if not vals.startswith("("):
                vals = f"({vals})"
            return f"INSERT INTO {tbl} VALUES {vals};"

        # If already standard SQL, add semicolon if missing
        return clean + ("" if clean.endswith(";") else ";")
