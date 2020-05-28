all: clear
	mkdir bin
	gcc -Wall cloudfs.c `pkg-config fuse3 --cflags --libs` -o bin/cloudfs
install:
	cp bin/cloudfs /usr/bin/cloudfs
clear:
	rm -rf bin
