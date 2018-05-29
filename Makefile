CC=g++
CFLAGS=-Wall -Werror -g -std=c++11

all: slave1 slave2 slave3 test master

slave1: slave1.cpp
	$(CC) $(CFLAGS) -pthread -o slave1 slave1.cpp
slave2: slave2.cpp
	$(CC) $(CFLAGS) -pthread -o slave2 slave2.cpp
slave3: slave3.cpp
	$(CC) $(CFLAGS) -pthread -o slave3 slave3.cpp
test: test-udp.cpp
	$(CC) $(CFLAGS) -pthread  -o test test-udp.cpp
master: master.cpp
	$(CC) $(CFLAGS) -pthread -o master master.cpp

clean:
	rm -f core slave1 slave2 slave3 test master
