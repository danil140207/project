CC = gcc
CFLAGS = -Wall -Wextra -O2 -g
LDFLAGS = -lm

TARGET = spreadsheet
SOURCES = main.c spreadsheet.c expr_eval.c
OBJECTS = $(SOURCES:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) -o $(TARGET) $(OBJECTS) $(LDFLAGS)

%.o: %.c spreadsheet.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET)

.PHONY: all clean