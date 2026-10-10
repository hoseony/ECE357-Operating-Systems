CC = gcc
CFLAGS = -Wall -Wextra

miniShell: miniShell.c
	$(CC) $(CFLAGS) -o ../tests/miniShell miniShell.c

debug: miniShell.c 
	$(CC) $(CFLAGS) -DDEBUG_MODE -o ../tests/miniShell miniShell.c

clean: 
	rm -f ../tests/miniShell
