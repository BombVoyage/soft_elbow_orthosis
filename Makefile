CC = g++
CFLAGS = -I./include
TARGET = bin/driver
LIBS = -lEposCmd -lpthread -lrt

all: $(TARGET)

$(TARGET): src/driver.cpp src/listener.cpp
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

clean:
	$(RM) $(TARGET)
