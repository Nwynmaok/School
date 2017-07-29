CFLAGS = -Wall
LDFLAGS = -lcrypto

all: md5sum.c server.c
	$(CC) $(CFLAGS) -o server md5sum.c server.c $(LDFLAGS)
