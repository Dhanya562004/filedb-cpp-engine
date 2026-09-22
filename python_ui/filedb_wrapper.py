import os
import sys
import json
import subprocess
import pandas as pd

class FileDBWrapper:
    def __init__(self, project_root=None):
        if project_root is None:
            # Determine directory containing filedb
            current_dir = os.path.dirname(os.path.abspath(__file__))
            project_root = os.path.abspath(os.path.join(current_dir, ".."))
        self.project_root = project_root
        self.exe_path = self._find_or_build_executable()

    def _find_or_build_executable(self):
        is_win = sys.platform.startswith("win")
        exe_name = "filedb.exe" if is_win else "filedb"
        exe_path = os.path.join(self.project_root, exe_name)

        if os.path.exists(exe_path):
            return exe_path

        # If not compiled, attempt automatic compilation
        print(f"FileDB executable not found at {exe_path}. Attempting automatic compilation...")
        self._compile_executable(exe_path)
        return exe_path

    def _compile_executable(self, target_path):
        is_win = sys.platform.startswith("win")
        cpp_files = [
            os.path.join(self.project_root, "src", "Models.cpp"),
            os.path.join(self.project_root, "src", "StorageManager.cpp"),
            os.path.join(self.project_root, "src", "IndexManager.cpp"),
            os.path.join(self.project_root, "src", "Logger.cpp"),
            os.path.join(self.project_root, "src", "QueryAnalyzer.cpp"),
            os.path.join(self.project_root, "src", "Parser.cpp"),
            os.path.join(self.project_root, "src", "ExecutionEngine.cpp"),
            os.path.join(self.project_root, "src", "NlpToSql.cpp"),
            os.path.join(self.project_root, "src", "main.cpp"),
        ]

        compiler = "g++"
        if is_win and os.path.exists(r"C:\Program Files\CodeBlocks\MinGW\bin\g++.exe"):
            compiler = r"C:\Program Files\CodeBlocks\MinGW\bin\g++.exe"

        include_dir = os.path.join(self.project_root, "include")
        cmd = [compiler, "-std=c++17", "-O2", f"-I{include_dir}"] + cpp_files + ["-o", target_path]

        env = os.environ.copy()
        if is_win:
            env["PATH"] += r";C:\Program Files\CodeBlocks\MinGW\bin"

        try:
            res = subprocess.run(cmd, cwd=self.project_root, capture_output=True, text=True, env=env)
            if res.returncode != 0:
                raise RuntimeError(f"Compilation failed:\n{res.stderr}")
            print("Automatic compilation successful!")
        except Exception as e:
            print(f"Compilation error: {e}")

    def execute_query(self, sql_query, index_enabled=True):
        if not os.path.exists(self.exe_path):
            self.exe_path = self._find_or_build_executable()

        flag = "--json" if index_enabled else "--no-index"
        cmd = [self.exe_path, flag, sql_query]

        env = os.environ.copy()
        if sys.platform.startswith("win"):
            env["PATH"] += r";C:\Program Files\CodeBlocks\MinGW\bin"

        try:
            res = subprocess.run(cmd, cwd=self.project_root, capture_output=True, text=True, env=env)
            output = res.stdout.strip()

            if not output:
                return {
                    "success": False,
                    "error_message": res.stderr.strip() or "No output returned from database engine.",
                    "dataframe": pd.DataFrame(),
                    "raw_json": {}
                }

            # Parse JSON result from C++ engine
            data = json.loads(output)
            cols = data.get("columns", [])
            rows = data.get("rows", [])

            df = pd.DataFrame(rows, columns=cols) if (cols and rows) else pd.DataFrame()

            return {
                "success": data.get("success", False),
                "error_message": data.get("error_message", ""),
                "info_message": data.get("info_message", ""),
                "execution_time_ms": data.get("execution_time_ms", 0.0),
                "rows_scanned": data.get("rows_scanned", 0),
                "rows_affected": data.get("rows_affected", 0),
                "columns": cols,
                "rows": rows,
                "dataframe": df,
                "plan": data.get("plan", {}),
                "raw_json": data
            }

        except json.JSONDecodeError as je:
            return {
                "success": False,
                "error_message": f"Failed to parse JSON response from database engine: {je}\nRaw output: {res.stdout}",
                "dataframe": pd.DataFrame(),
                "raw_json": {}
            }
        except Exception as e:
            return {
                "success": False,
                "error_message": str(e),
                "dataframe": pd.DataFrame(),
                "raw_json": {}
            }

    def compare_performance(self, sql_query):
        res_index = self.execute_query(sql_query, index_enabled=True)
        res_scan = self.execute_query(sql_query, index_enabled=False)
        return {
            "indexed": res_index,
            "full_scan": res_scan
        }
