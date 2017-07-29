CFLAGS = -Wall
LDFLAGS = -lcrypto

all: md5sum.c client.c
	$(CC) $(CFLAGS) -o client md5sum.c client.c $(LDFLAGS)
