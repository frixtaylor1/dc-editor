BUILD_DIR      = build
SRC_DIR        = src
CXX            = g++
CXX_FLAGS      = -std=c++11 -Werror -Wall -Wpedantic

EXEC_NAME      = $(BUILD_DIR)/game
SRC_FILES      = $(shell find $(SRC_DIR) -name "*.cpp")
RAYLIB_DIR     = vendor/raylib
LDFLAGS        = -L$(RAYLIB_DIR)/lib -lraylib -lm -lpthread -ldl -lrt -L/usr/lib/x86_64-linux-gnu -lX11

INCLUDE_RAYLIB = -I$(RAYLIB_DIR)/include
EXEC_COMMAND   = $(CXX) $(CXX_FALGS) $(INCLUDE_RAYLIB)

$(EXEC_NAME): $(SRC_FILES)
	@mkdir -p $(BUILD_DIR)
	$(EXEC_COMMAND) -o $(EXEC_NAME) $(SRC_FILES) $(LDFLAGS)

run: $(EXEC_NAME)
	LD_LIBRARY_PATH=$(RAYLIB_DIR)/lib ./$(EXEC_NAME)

clean:
	rm -rf $(BUILD_DIR)

clean-build: clean $(EXEC_NAME)

.PHONY: clean clean-build run
