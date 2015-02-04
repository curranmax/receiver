
#include <iostream>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int makeUDPSocket(int port){
	// Make socket
	int sock = socket(AF_INET,SOCK_DGRAM,0);
	if(sock < 0){
		std::cerr << "Couldn't open socket" << std::endl;
		exit(0);
	}

	// Bind socket
	sockaddr_in addr;
	memset((void *)&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(port);

	int rv = bind(sock, (struct sockaddr *)&addr, sizeof(addr));
	if(rv < 0){
		std::cerr << "Couldn't bind socket" << std::endl;
		exit(0);
	}

	return sock;
}
