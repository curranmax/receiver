
#ifndef _SENDER_H_
#define _SENDER_H_

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <simplefso.h>

#include "utils.h"

class Sender {
public:
	Sender(int lp, int fp, int tp, bool v,SimpleFSO* simplefso);

	void sendBoth(std::string msg);
	void sendBroadcast(std::string msg);

private:
	void sendMsgs(std::string msg, sockaddr_in a1, sockaddr_in a2);	
	void sendMsg(std::string msg, sockaddr_in to_addr);

	int sock;
	int foreign_port, test_port;
	SimpleFSO* fso;

	bool verbose;

	sockaddr_in addr1,addr2;
};

#endif
