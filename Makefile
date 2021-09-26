

all:  server.o client.o



client.o: client/udp_client.c
	cd client;
	gcc client/udp_client.c -o client/udpclient

server.o: server/udp_server.c
	cd server;
	gcc server/udp_server.c -o server/udpserver







clean:
	rm -f server/udpserver 
	rm -f client/udpclient
