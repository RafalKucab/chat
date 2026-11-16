CC = gcc

CFLAGS = -Wall -Wextra -Werror -std=c17 -g
INCLUDES = -Iinclude

BUILD_DIR = build
SRC_DIR = src

# Shared modules
COMMON_SRC = $(wildcard $(SRC_DIR)/common/*.c)
COMMON_OBJ = $(patsubst $(SRC_DIR)/common/%.c,$(BUILD_DIR)/common/%.o,$(COMMON_SRC))

# Server modules
SERVER_SRC = $(wildcard $(SRC_DIR)/server/*.c)
SERVER_OBJ = $(patsubst $(SRC_DIR)/server/%.c,$(BUILD_DIR)/server/%.o,$(SERVER_SRC))

# Client modules
CLIENT_SRC = $(wildcard $(SRC_DIR)/client/*.c)
CLIENT_OBJ = $(patsubst $(SRC_DIR)/client/%.c,$(BUILD_DIR)/client/%.o,$(CLIENT_SRC))

SERVER = $(BUILD_DIR)/chat-server
CLIENT = $(BUILD_DIR)/chat-client

.PHONY: all server client clean

all: $(SERVER) $(CLIENT)

server: $(SERVER)
client: $(CLIENT)

$(SERVER): $(SERVER_OBJ) # $(COMMON_OBJ)
	$(CC) $^ -o $@

$(CLIENT): $(CLIENT_OBJ) # $(COMMON_OBJ)
	$(CC) $^ -o $@

# Compile source files
$(BUILD_DIR)/common/%.o: $(SRC_DIR)/common/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(BUILD_DIR)/server/%.o: $(SRC_DIR)/server/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(BUILD_DIR)/client/%.o: $(SRC_DIR)/client/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)

