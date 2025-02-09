MODULE := sstv_decoder

CC := gcc
CFLAGS := -Werror -Wextra -Wpedantic

SRC_DIR := src
OBJ_DIR := obj
BIN_DIR := bin

SRC_C_LIST := $(MODULE).c wav_file.c
SRC_O_LIST := $(patsubst %.c, $(OBJ_DIR)/%.o, $(notdir $(SRC_C_LIST)))


.PHONY: bin_dir obj_dir clean


$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | obj_dir
	$(CC) $(CFLAGS) $< -I$(SRC_DIR) -c -o $@


$(MODULE): $(SRC_O_LIST) | bin_dir
	$(CC) $(CFLAGS) $^ -o $(BIN_DIR)/$@

bin_dir:
	mkdir -p $(BIN_DIR)


obj_dir:
	mkdir -p $(OBJ_DIR)


clean:
	@rm -rf $(OBJ_DIR) $(BIN_DIR)
