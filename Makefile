CC = gcc
LD = gcc
CFLAGS = -I .
LFLAGS = -lcrypto

OBJECTS = p2p_server.o p2p_stun.o p2p_bcast_pkt.o

all: test

install: all
	chown root ./test
	chgrp root ./test
	chmod 755 ./test
	chmod u+s ./test

test: $(OBJECTS) test.o
	$(LD) $^ $(LFLAGS) -o $@

.c.o:
	$(CC) $(CFLAGS) $< -c

clean:
	$(RM) $(OBJECTS) test.o test
