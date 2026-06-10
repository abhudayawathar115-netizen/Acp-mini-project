CC = gcc
CFLAGS = -Wall -Wextra -std=c99
TARGET = editor

all: $(TARGET)

$(TARGET): editor.c
	$(CC) $(CFLAGS) editor.c -o $(TARGET)

clean:
	@del /f /q $(TARGET).exe 2>nul || rm -f $(TARGET) $(TARGET).exe

run: $(TARGET)
	./$(TARGET)
