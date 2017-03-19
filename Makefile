
# Makefile based on Terasic Makefile for my_first_hps-fpga template project

TARGET = calibration_testing
CFLAGS = -static -g -Wall
LDFLAGS = -g -Wall
CC = gcc

build: $(TARGET)

$(TARGET): $(TARGET).o
	$(CC) $(LDLAGS) -lm $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -f $(TARGET) *.a *.o *~