# Makefile
CC = gcc
CFLAGS = -Wall -I./include
SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin

# Kaynak dosyalar
SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

# Hedef dosya
TARGET = $(BIN_DIR)/randevu_sistemi

# Klasörleri oluştur
$(shell mkdir -p $(OBJ_DIR) $(BIN_DIR))

# Ana hedef
all: $(TARGET)

# Program derleme
$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET)

# Obje dosyaları derleme
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Temizleme
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

.PHONY: all clean