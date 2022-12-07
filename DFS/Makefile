all: dfs.o dfs.o

dfs.o: dfserver/dfs.c
	gcc -pthread dfserver/dfs.c -o dfserver/dfs

dfc.o: dfclient/dfc.c
	gcc dfclient/dfc.c -o dfclient/dfc -lcrypto -lssl

spawn1: dfserver/dfs
	dfserver/dfs /DFS1 10001

spawn2: dfserver/dfs
	dfserver/dfs /DFS2 10002

spawn3: dfserver/dfs
	dfserver/dfs /DFS3 10003

spawn4: dfserver/dfs
	dfserver/dfs /DFS4 10004

connect: dfclient/dfc
	dfclient/dfc dfclient/dfc.conf
