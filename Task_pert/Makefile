CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -O2

TARGET := cpm_test
BUILD_DIR := build
BINARY := $(BUILD_DIR)/$(TARGET)

INCLUDE_DIRS := . CPM PERT Data_Loader Result_printer
CPPFLAGS := $(addprefix -I,$(INCLUDE_DIRS)) -MMD -MP

SRCS := \
	main.cpp \
	CPM/CPMCalculator.cpp \
	CPM/Task.cpp \
	PERT/PERTCalculator.cpp \
	PERT/Task_pert.cpp \
	Result_printer/ResultPrinter.cpp \
	Data_Loader/DataLoader.cpp \
	Data_Loader/DataLoader_pert.cpp

OBJS := $(SRCS:%.cpp=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

.PHONY: all clean run

all: $(BINARY)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BINARY): $(OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@

$(BUILD_DIR)/%.o: %.cpp | $(BUILD_DIR)
	mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

run: $(BINARY)
	./$(BINARY)

clean:
	rm -rf $(BUILD_DIR)

-include $(DEPS)
