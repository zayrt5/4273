all: httpgetserver.o

httpgetserver.o: www/httpgetserver.c
	gcc -pthread www/httpgetserver.c -o www/httpgetserver


run: www/httpgetserver
	www/httpgetserver 8888

test: curltest.sh
	./curltest.sh

clean:
	rm -rf www/httpgetserver