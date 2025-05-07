SOURCE_DIR=src
BUILD_DIR=build

CXX := g++
CXXFLAGS ?= 
CXXFLAGS += -O1 -std=c++17 # -fsanitize=address 

TARGET := sidump
SRCS := $(shell find $(SOURCE_DIR) -name '*.cpp')
OBJS := $(patsubst $(SOURCE_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(SRCS))

CXXFLAGS += -I$(SOURCE_DIR) -I$(SOURCE_DIR)/core

all: $(TARGET)

$(TARGET): $(OBJS) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS)

$(BUILD_DIR)/%.o: $(SOURCE_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	@rm -rf $(BUILD_DIR)
	@rm -f $(TARGET) 

.PHONY: all clean
