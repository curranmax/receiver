
CC = g++
GM_FLAGS = -I/usr/local/include/gm -L/usr/local/lib -lgm
FLAGS = -static -Wall  $(GM_FLAGS)

all: clean receiver_controller

receiver_controller: receiver_controller.o simplefso.o utils.o
	$(CC)  receiver_controller.o simplefso.o utils.o -o receiver_controller $(FLAGS)

receiver_controller.o:
	$(CC) -c receiver_controller.cpp $(FLAGS)

simplefso.o:
	$(CC) -c simplefso.cpp $(FLAGS)

utils.o:
	$(CC) -c utils.cpp $(FLAGS)

clean:
	rm -rf *.o receiver_controller