
# Makefile based on Terasic Makefile for my_first_hps-fpga template project

TARGET = lcd_test
CFLAGS = -g -Wall
CC=gcc

build: $(TARGET)

$(TARGET): $(TARGET).o
	$(CC) $(LDFLAGS) $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -f $(TARGET) *.a *.o *~
