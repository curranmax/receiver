
#include "sender.h"
#include "utils.h"

Sender::Sender(int lp, int fp, int tp, bool v,SimpleFSO* simplefso){
	fso = simplefso;

	sock = makeUDPSocket(lp);

	foreign_port = fp;
	test_port = tp;
}
void Sender::sendBoth(std::string msg){
	sendMsgs(msg,addr1,addr2);
}

void Sender::sendBroadcast(std::string msg){
	// Make broadcast address
	sockaddr_in broadcast_addr;

	sendMsgs(msg,broadcast_addr,broadcast_addr);
}

void Sender::sendMsgs(std::string msg){
	if(fso != NULL){
		fso->reset();
	}
	sendMsg(msg,addr1);
	if(fso != NULL){
		fso->switchLink();
		sendMsg(msg,addr2);
		fso->switchLink();
	}
}

void Sender::sendMsg(std::string msg, sockaddr_in to_addr){
	while(true){
		// Send message

		// Wait for ack message

		break;
	}
}