# Compiler and flags
CXX = g++
CXXFLAGS = -Wall -Wextra -g -std=c++20 -O3
LDFLAGS = -Wl,-stack_size,16000000

# Directories
OUTPUT_DIR = output

# Source files
SOURCES = main.cpp \
          dataGenerator/dataGenerator.cpp \
          Dijkstra/dijkstra_algorithm.cpp \
          Bellman-Ford/BellmanFord.cpp \
          aStar/aStar.cpp

BENCHMARK_SOURCES = benchmark.cpp \
                    dataGenerator/dataGenerator.cpp \
                    Dijkstra/dijkstra_algorithm.cpp \
                    Bellman-Ford/BellmanFord.cpp \
                    aStar/aStar.cpp

MAXFLOW_SOURCES = main_maxflow.cpp \
                  dataGenerator/dataGenerator.cpp \
                  Ford-Fulkerson/FordFulkerson.cpp \
                  Edmonds-Karp/EdmondsKarp.cpp

# Output executables
TARGET = $(OUTPUT_DIR)/main
BENCHMARK_TARGET = $(OUTPUT_DIR)/benchmark
MAXFLOW_TARGET = $(OUTPUT_DIR)/main_maxflow

# Default target
all: $(TARGET)

# Main executable (shortest paths)
$(TARGET): $(SOURCES)
	@mkdir -p $(OUTPUT_DIR)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(SOURCES) -o $(TARGET)
	@echo "Build complete: $(TARGET)"

# Benchmark executable
benchmark: $(BENCHMARK_TARGET)

$(BENCHMARK_TARGET): $(BENCHMARK_SOURCES)
	@mkdir -p $(OUTPUT_DIR)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(BENCHMARK_SOURCES) -o $(BENCHMARK_TARGET)
	@echo "Build complete: $(BENCHMARK_TARGET)"

# Max flow executable
maxflow: $(MAXFLOW_TARGET)

$(MAXFLOW_TARGET): $(MAXFLOW_SOURCES)
	@mkdir -p $(OUTPUT_DIR)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(MAXFLOW_SOURCES) -o $(MAXFLOW_TARGET)
	@echo "Build complete: $(MAXFLOW_TARGET)"

# Run main program
run: $(TARGET)
	./$(TARGET)

# Run benchmark
run-benchmark: $(BENCHMARK_TARGET)
	./$(BENCHMARK_TARGET)

# Run max flow program
run-maxflow: $(MAXFLOW_TARGET)
	./$(MAXFLOW_TARGET)

# Clean build artifacts
clean:
	rm -rf $(OUTPUT_DIR)/*
	@echo "Clean complete"

# Rebuild everything
rebuild: clean all

# Build all targets
all-targets: $(TARGET) $(BENCHMARK_TARGET) $(MAXFLOW_TARGET)
	@echo "All targets built successfully"

.PHONY: all benchmark maxflow run run-benchmark run-maxflow clean rebuild all-targets
