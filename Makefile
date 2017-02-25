
# Makefile based on Terasic Makefile for my_first_hps-fpga template project

TARGET = lcd_test
CFLAGS = -static -g -Wall -I$/home/debian/hwlib/include
LDFLAGS = -g -Wall
CC=gcc

build: $(TARGET)

$(TARGET): $(TARGET).o
	$(CC) $(LDLAGS) $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -f $(TARGET) *.a *.o *~
