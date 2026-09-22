CXX = g++
CXXFLAGS = -std=c++17 -O2 -Wall -Iinclude

SRCS = src/Models.cpp src/StorageManager.cpp src/IndexManager.cpp src/Logger.cpp src/QueryAnalyzer.cpp src/Parser.cpp src/ExecutionEngine.cpp src/NlpToSql.cpp

all: filedb test_runner

filedb: $(SRCS) src/main.cpp
	$(CXX) $(CXXFLAGS) $(SRCS) src/main.cpp -o filedb

test_runner: $(SRCS) src/test_runner.cpp
	$(CXX) $(CXXFLAGS) $(SRCS) src/test_runner.cpp -o test_runner

test: test_runner
	./test_runner

clean:
	rm -f filedb filedb.exe test_runner test_runner.exe
