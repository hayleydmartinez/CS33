CC = gcc
CFLAGS = -w -pthread -std=gnu99 -O3

thread: thread.c thread.h
	$(CC) $(CFLAGS) -o thread thread.c

clean:
	rm -f *~ thread
