all: clear
	mkdir bin
	g++ --std=c++17 -Wall cloudfs.cpp `pkg-config fuse3 --cflags --libs` -lssl -lcrypto -o bin/cloudfs
	mkdir bin/mnt
install:
	cp bin/cloudfs /usr/bin/cloudfs
clear:
	rm -rf bin
