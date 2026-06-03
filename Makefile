CC = gcc
CFLAGS = -Wall -Wextra

ssi: ssi.c
	$(CC) $(CFLAGS) ssi.c -o ssi

clean:
	rm -f ssi
