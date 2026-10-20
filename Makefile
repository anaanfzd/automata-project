CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -Iinclude
LDFLAGS = -lsfml-graphics -lsfml-window -lsfml-system

BIN_DIR = bin
BUILD_DIR = build_obj

CORE_SRCS = src/Regex/RegexValidator.cpp \
            src/Regex/RegexParser.cpp \
            src/Automata/NFA.cpp \
            src/Automata/DFA.cpp \
            src/Algorithms/ThompsonConstruction.cpp \
            src/Algorithms/SubsetConstruction.cpp \
            src/Algorithms/DFAMinimization.cpp \
            src/Algorithms/StringSimulator.cpp

APP_SRCS = src/main.cpp \
           src/Visualization/GraphLayout.cpp \
           src/Visualization/GraphRenderer.cpp \
           src/Visualization/Animation.cpp \
           src/UI/Controls.cpp \
           src/UI/InfoPanel.cpp \
           src/UI/TableView.cpp \
           src/UI/MainWindow.cpp

TEST_SRCS = tests/test_runner.cpp

CORE_OBJS = $(patsubst src/%.cpp, $(BUILD_DIR)/%.o, $(CORE_SRCS))
APP_OBJS  = $(patsubst src/%.cpp, $(BUILD_DIR)/%.o, $(APP_SRCS))
TEST_OBJS = $(BUILD_DIR)/test_runner.o

TARGET_APP = $(BIN_DIR)/AutomataViz.exe
TARGET_TEST = $(BIN_DIR)/AutomataViz_Tests.exe

all: $(TARGET_APP) $(TARGET_TEST)

$(TARGET_APP): $(CORE_OBJS) $(APP_OBJS)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CORE_OBJS) $(APP_OBJS) $(LDFLAGS) -o $@
	@echo "Build complete: $(TARGET_APP)"

$(TARGET_TEST): $(CORE_OBJS) $(TEST_OBJS)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CORE_OBJS) $(TEST_OBJS) -o $@
	@echo "Build complete: $(TARGET_TEST)"

$(BUILD_DIR)/%.o: src/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/test_runner.o: tests/test_runner.cpp
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

test: $(TARGET_TEST)
	./$(TARGET_TEST)

clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)

.PHONY: all test clean
