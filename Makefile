SOURCE_DIR=src
BUILD_DIR=build

CXX := g++
CXXFLAGS ?= 
CXXFLAGS += -std=c++17 -Wall -Wextra -Werror -Wno-unused-parameter

TARGET := sidump
SRCS := $(wildcard $(SOURCE_DIR)/*.cpp)
OBJS := $(SRCS:$(SOURCE_DIR)/%.cpp=$(BUILD_DIR)/%.o)

all: $(TARGET)

$(TARGET): $(OBJS) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS)

$(BUILD_DIR)/%.o: $(SOURCE_DIR)/%.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(BUILD_DIR):
	mkdir -p $@

clean:
	rm -f $(TARGET) $(BUILD_DIR)/*.o

.PHONY: all clean
