CFLAGS= -Wall -Werror
LDFLAGS=-pthread 
CC=gcc
OBJECTS=main.o dll.o
TARGET=5013HW3

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -pthread -o $@

clean:
	rm ./$(TARGET) *.o
