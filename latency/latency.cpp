
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <simplefso.h>

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

void processCommandLine(int argc, char const *argv[],int &tp,int &nt,int &ps,float &wt,std::string &of,std::string &cf) {
	for(int i = 1; i < argc; ++i) {
		std::string val = argv[i];
		if((val.compare("-tp") == 0 || val.compare("--test_port") == 0) && i < argc - 1){
			i++;
			tp = atoi(argv[i]);
		}
		if((val.compare("-w") == 0 || val.compare("--wait_time") == 0) && i < argc - 1){
			i++;
			wt = atof(argv[i]);
		}
		if((val.compare("-nt") == 0 || val.compare("--num_test") == 0) && i < argc - 1){
			i++;
			nt = atoi(argv[i]);
		}
		if((val.compare("-ps") == 0 || val.compare("--packet_size") == 0) && i < argc - 1){
			i++;
			ps = atoi(argv[i]);
		}
		if((val.compare("-out") == 0 || val.compare("--out_file") == 0) && i < argc - 1){
			i++;
			of = argv[i];
		}
		if((val.compare("-config") == 0 || val.compare("--config_file") == 0) && i < argc - 1){
			i++;
			cf = argv[i];
		}
	}
}

int main(int argc, char const *argv[]) {
	int test_port = 9999;
	int num_test = 1;
	int packet_size = 1;
	float wait_time = 1.0; // in seconds
	std::string out_file = "";
	std::string config_file = "";

	processCommandLine(argc,argv,test_port,num_test,packet_size,wait_time,out_file,config_file);

	int micro_wait_time = int(wait_time * 1000000);

	// Make a socket
	int sock = makeUDPSocket(test_port);

	int buffer_size = packet_size + 10;
	char buffer[buffer_size];
	sockaddr_in from_addr;
	socklen_t addrlen = sizeof(from_addr);

	// read in config file
	SimpleFSO *fso = new SimpleFSO(config_file);

	fso->connectGM();

	std::vector<double> latency_vals;
	for(int i = 0; i < num_test; ++i){
		// Wait some time, this is to ensure that the gm doesn't switch too fast and break
		usleep(micro_wait_time);

		// Wait till recieve a packet
		recvfrom(sock,buffer,buffer_size,0,(sockaddr *)&from_addr,&addrlen);

		// record time
		const clock_t begin_time = clock();

		// Switch
		fso->switchLink();

		// recieve new packet
		recvfrom(sock,buffer,buffer_size,0,(sockaddr *)&from_addr,&addrlen);
		// record time
		const clock_t end_time = clock();  // Not most accurate,  at most accurate to .01
		latency_vals.push_back( double(end_time - begin_time) / CLOCKS_PER_SEC );
	}

	// Write data to file
	std::ofstream out;
	out.open(out_file.c_str(),std::ios::app);

	// Print params
	out << "packet_size=" << packet_size << ",wait_time=" << wait_time << ",language=C++";
	// Print data
	for(unsigned int i = 0; i < latency_vals.size(); ++i){
		out << " " << latency_vals[i];
	}
	out << std::endl;
	
	return 0;
}
