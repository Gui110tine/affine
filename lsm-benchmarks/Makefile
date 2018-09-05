EXEC = benchmark
SRC_FILES = benchmark.cc

CC = g++
CFLAGS = -g -Wall -std=c++11
TARGET = benchmark
LDFLAGS = -L/usr/local/lib
LDLIBS = -lleveldb -lpthread -lrocksdb -lz

all: $(TARGET)

$(TARGET): $(TARGET).cc
	$(CC) $(CFLAGS) -o $(TARGET) $(TARGET).cc $(LDFLAGS) $(LDLIBS)

clean: 
	$(RM) $(TARGET)
