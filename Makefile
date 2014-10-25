CC = gcc
LD = gcc
CFLAGS = -I .
LFLAGS = -lcrypto

OBJECTS = p2p_server.o p2p_client.o p2p_stun.o p2p_bcast_pkt.o

all: server client

install: all
	chown root ./server
	chgrp root ./server
	chmod 755 ./server
	chmod u+s ./server
	chmod root ./client
	chgrp root ./client
	chmod 755 ./client
	chmod u+s ./client

server: $(OBJECTS) test_server.o
	$(LD) $^ $(LFLAGS) -o $@

client: $(OBJECTS) test_client.o
	$(LD) $^ $(LFLAGS) -o $@

.c.o:
	$(CC) $(CFLAGS) $< -c

clean:
	$(RM) $(OBJECTS) test_server.o server test_client.o client
