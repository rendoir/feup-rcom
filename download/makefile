CC = gcc

CFLAGS  = -Wall

SRCS = ftp.c parser.c download.c

TARGET = download

default:
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET)

clean:
	rm -rf *.o
	rm -rf $(TARGET)
