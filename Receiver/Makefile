CC = g++
FLAGS = -Wall -std=c++0x
# -std=c++0x flag is required to use chrono library for timing

all: clean receiver

receiver: receiver.o utils.o
	$(CC) $(FLAGS) udp_receiver.o utils.o -o receiver

receiver.o:
	$(CC) $(FLAGS) -c udp_receiver.cpp

utils.o:
	$(CC) $(FLAGS) -c utils.cpp

clean:
	rm -rf *.o receiver
