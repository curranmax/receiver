CC = g++
GM_FLAGS = -I/usr/local/include/gm -L/usr/local/lib -lgm
FLAGS = -static -Wall  $(GM_FLAGS)

all: alternate

alternate: clean
	$(CC) -o alternate main.cpp $(FLAGS)

clean:
	rm -rf *o align