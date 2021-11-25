all: webproxy.o

webproxy.o: webproxy.c
	gcc -pthread webproxy.c -o webproxy


run: 
	./webproxy 8888


clean:
	rm -rf webproxy