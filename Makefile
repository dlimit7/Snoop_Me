CC=g++
CFLAGS=-Wall -Werror -g -std=c++11

all: slave test master

slave: slave.cpp
	$(CC) $(CFLAGS) -pthread -o slave slave.cpp
test: test-udp.cpp
	$(CC) $(CFLAGS) -pthread  -o test test-udp.cpp
master: http_master.cpp
	$(CC) $(CFLAGS) -o master http_master.cpp

clean:
	rm -f core slave test master
