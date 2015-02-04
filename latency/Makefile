CC = g++
FLAGS = -Wall
GM_FLAGS = -static -Wall -I/usr/local/include/gm -L/usr/local/lib -lgm

all: latency

latency: latency.o
	$(CC) latency.o -o latency $(GM_FLAGS)

latency.o:
	$(CC) -c latency.cpp $(GM_FLAGS)