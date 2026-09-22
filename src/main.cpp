#include <iostream>
#include <string>
#include <algorithm>
#include "../include/db/ExecutionEngine.hpp"
#include "../include/db/Parser.hpp"
#include "../include/db/Logger.hpp"

using namespace filedb;

void show_help() {
    std::cout << "\n=======================================================\n"
              << "       FileDB++ Mini Database Engine CLI (v2.0)        \n"
              << "=======================================================\n"
              << " Supported SQL Statements:\n"
              << "   CREATE TABLE <table_name> (<col1> <type> [PRIMARY KEY], ...);\n"
              << "   INSERT INTO <table_name> VALUES (<val1>, <val2>, ...);\n"
              << "   SELECT <col1, col2 | *> FROM <table_name> [WHERE <col> <op> <val>];\n"
              << "   UPDATE <table_name> SET <col> = <val> [WHERE <col> <op> <val>];\n"
              << "   DELETE FROM <table_name> [WHERE <col> <op> <val>];\n\n"
              << " Special Commands:\n"
              << "   EXPLAIN <query>      : View query execution plan & scan strategy\n"
              << "   NLP <english phrase> : AI natural language to SQL converter\n"
              << "   INDEX ON / OFF       : Toggle primary/secondary hash indexing\n"
              << "   HELP / ?             : Show available commands\n"
              << "   EXIT / QUIT          : Exit REPL shell\n"
              << "=======================================================\n\n";
}

int main(int argc, char *argv[]) {
    ExecutionEngine engine;

    // Check command line arguments for non-interactive modes
    if (argc >= 2) {
        std::string arg1 = argv[1];
        if (arg1 == "--json" && argc >= 3) {
            std::string sql = argv[2];
            QueryResult res = engine.execute_query(sql);
            std::cout << res.to_json() << std::endl;
            return res.success ? 0 : 1;
        } else if (arg1 == "--no-index" && argc >= 3) {
            engine.set_indexing_enabled(false);
            std::string sql = argv[2];
            QueryResult res = engine.execute_query(sql);
            std::cout << res.to_json() << std::endl;
            return res.success ? 0 : 1;
        }
    }

    std::cout << "\nWelcome to FileDB++ (C++ High Performance Mini Database Engine)\n"
              << "Type 'HELP' for commands or 'EXIT' to quit.\n\n";

    std::string line;
    std::cout << "SQL> ";
    while (std::getline(std::cin, line)) {
        std::string cmd = Parser::trim(line);
        if (cmd.empty()) {
            std::cout << "SQL> ";
            continue;
        }

        std::string lower = Parser::to_lower(cmd);
        if (lower == "exit" || lower == "quit") {
            std::cout << "Goodbye!\n";
            break;
        }
        if (lower == "help" || lower == "?") {
            show_help();
            std::cout << "SQL> ";
            continue;
        }
        if (lower == "index on") {
            engine.set_indexing_enabled(true);
            std::cout << "Indexing system ENABLED.\nSQL> ";
            continue;
        }
        if (lower == "index off") {
            engine.set_indexing_enabled(false);
            std::cout << "Indexing system DISABLED (Sequential scan mode).\nSQL> ";
            continue;
        }

        QueryResult res = engine.execute_query(cmd);
        if (!res.info_message.empty()) {
            std::cout << res.info_message << "\n";
        }
        std::cout << res.to_cli_table() << "\n\n";
        std::cout << "SQL> ";
    }

    return 0;
}
