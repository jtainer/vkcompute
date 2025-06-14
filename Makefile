UNAME_S := $(shell uname -s)

ifeq ($(UNAME_S), Linux)
	LDFLAGS = -lvulkan
endif

ifeq ($(UNAME_S), Windows_NT)
	LDFLAGS = -lvulkan-1 -L$(LIB_DIR)
endif

CC = gcc
CFLAGS = -std=c99 -I$(INC_DIR) -Wall
SRC_DIR = src
INC_DIR = include
LIB_DIR = 
BUILD_DIR = build
SHADER_DIR = shaders
SRC = $(wildcard $(SRC_DIR)/*.c)
OBJ = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SRC))
COMP_SHADERS = $(wildcard $(SHADER_DIR)/*.comp)
SPV_SHADERS = $(patsubst $(SHADER_DIR)/%.comp, $(SHADER_DIR)/%.spv, $(COMP_SHADERS))
TARGET = vkcompute

all: app shaders

app: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET) $(LDFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

shaders: $(SPV_SHADERS)

$(SHADER_DIR)/%.spv: $(SHADER_DIR)/%.comp
	glslc $< -o $@

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(OBJ) $(TARGET)
	rm -rf $(BUILD_DIR)
	rm -f $(SHADER_DIR)/*.spv
