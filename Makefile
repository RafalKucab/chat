CC = gcc

CFLAGS = -Wall -Wextra -Werror -std=c17 -g
INCLUDES = -Iinclude

BUILD_DIR = build
SRC_DIR = src

# Shared modules
COMMON_SRC = $(filter-out %_test.c,$(wildcard $(SRC_DIR)/common/*.c))
COMMON_OBJ = $(patsubst $(SRC_DIR)/common/%.c,$(BUILD_DIR)/common/%.o,$(COMMON_SRC))

# Server modules
SERVER_SRC = $(filter-out %_test.c,$(wildcard $(SRC_DIR)/server/*.c))
SERVER_OBJ = $(patsubst $(SRC_DIR)/server/%.c,$(BUILD_DIR)/server/%.o,$(SERVER_SRC))

# Client modules
CLIENT_SRC = $(filter-out %_test.c,$(wildcard $(SRC_DIR)/client/*.c))
CLIENT_OBJ = $(patsubst $(SRC_DIR)/client/%.c,$(BUILD_DIR)/client/%.o,$(CLIENT_SRC))

# Tests
TEST_SRC = $(shell find $(SRC_DIR) -name '*_test.c')
TEST_OBJ = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/tests/%.o,$(TEST_SRC))

MUNIT_SRC = vendor/munit/munit.c
MUNIT_OBJ = $(BUILD_DIR)/tests/munit.o

SERVER = $(BUILD_DIR)/chat-server
CLIENT = $(BUILD_DIR)/chat-client
TESTS = $(BUILD_DIR)/run-tests

.PHONY: all server client tests clean compdb

all: $(SERVER) $(CLIENT)

server: $(SERVER)
client: $(CLIENT)
tests: $(TESTS)
	./$(TESTS)

$(SERVER): $(SERVER_OBJ) # $(COMMON_OBJ)
	$(CC) $^ -o $@

$(CLIENT): $(CLIENT_OBJ) $(COMMON_OBJ)
	$(CC) $^ -o $@

$(TESTS): $(TEST_OBJ) $(COMMON_OBJ) $(MUNIT_OBJ)
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

$(BUILD_DIR)/tests/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(INCLUDES) -Ivendor/munit -c $< -o $@

$(MUNIT_OBJ): $(MUNIT_SRC)
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -Ivendor/munit -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)


compdb:
	$(MAKE) clean
	bear -- $(MAKE) all $(TESTS)
