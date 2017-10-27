CC = gcc

CFLAGS  = -g '-std=c11' -D_POSIX_C_SOURCE -Wall

CPPFLAGS += -D DEBUG

SRCS = applicationLayer.c linkLayer.c alarm.c serialPort.c utils.c

TARGET = app

default:
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET)

debug:
	$(CC) $(CPPFLAGS) $(CFLAGS) $(SRCS) -o $(TARGET)

clean:
	rm -rf *.o
	rm -rf app
