
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <signal.h>

#include <chrono>
#include <ratio>
#include <ctime>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "utils.h"
std::chrono::high_resolution_clock::time_point start_time,end_time;
volatile sig_atomic_t stop_flag = 0;
void signal_handler(int sig){
	end_time = std::chrono::high_resolution_clock::now();
	stop_flag = 1;
}

int main(int argc,char **argv) {
	// Command line input
	int port = 0;
	std::string out_file = "";
	std::string out_info = "";
	bool out_b = false,out_kb = false, out_mb = false, out_gb = false;
	bool command_line_output = false;
	bool count_packets = false;
	int packet_size = 0;
	for(int i = 0; i < argc; i++){
		if((strcmp("-p",argv[i]) == 0 || strcmp("-port",argv[i]) == 0) && i < argc - 1){
			port = atoi(argv[++i]);
		}
		if(strcmp("-packet",argv[i]) == 0 && i < argc - 1){
			count_packets = true;
			packet_size = atoi(argv[++i]);
		}
		if((strcmp("-o",argv[i]) == 0 || strcmp("-out",argv[i]) == 0) && i < argc - 1){
			out_file = argv[++i];
		}
		if(strcmp("-info",argv[i]) == 0 && i < argc - 1){
			out_info = argv[++i];
		}
		if(strcmp("-c",argv[i]) == 0){
			command_line_output = true;
		}
		if(strcmp("-b",argv[i]) == 0){
			out_b = true;
		}
		if(strcmp("-kb",argv[i]) == 0){
			out_kb = true;
		}
		if(strcmp("-mb",argv[i]) == 0){
			out_mb = true;
		}
		if(strcmp("-gb",argv[i]) == 0){
			out_gb = true;
		}
	}

	// Check command line input
	if(port == 0){
		std::cerr << "No port specified" << std::endl;
		exit(0);
	}
	// If none set then defaults to only kb
	if(!(out_b || out_kb || out_mb || out_gb)){
		out_kb = true;
	}
	// buffer is big enough for two packets
	int buffer_size =  2 * packet_size;

	// Set up receiver
	int sock = makeUDPSocket(port);
	

	sockaddr_in trans_addr;
	socklen_t addrlen = sizeof(trans_addr);
	char buffer[buffer_size];
	unsigned long total_bytes = 0;
	unsigned int number_of_packets = 0;

	signal(SIGINT,signal_handler);

	start_time = std::chrono::high_resolution_clock::now();

	// Receive loop
	// Problem: Ctrl+C doesn't exit wait for receive
	while(stop_flag == 0){
		int recv_len = recvfrom(sock,buffer,buffer_size,0,(sockaddr *)&trans_addr,&addrlen);
		if(stop_flag == 0){
			total_bytes += recv_len;
			if(count_packets && recv_len == packet_size){
				number_of_packets += 1;
			}
		}
	}
	
	// Calculate results
	// int seconds = difftime(end_time,start_time);
	std::chrono::duration<double> span = std::chrono::duration_cast<std::chrono::duration<double> >(end_time - start_time);
	float seconds = span.count();
	float packets_per_second = (float)number_of_packets/(float)seconds;
	float bytes_per_second = (float)total_bytes / (float)seconds;
	float kilobytes_per_second = bytes_per_second / 1000.0;
	float megabytes_per_second = kilobytes_per_second / 1000.0;
	float gigabytes_per_second = megabytes_per_second / 1000.0;

	// Print results to command line
	if(command_line_output){
		std::cout << std::endl << seconds << " seconds elapsed" << std::endl;
		if(count_packets){
			std::cout << "Packets per second: " << packets_per_second << std::endl;
		}
		if(out_b){
			std::cout << "Bytes per second: " << bytes_per_second << std::endl;
		}
		if(out_kb){
			std::cout << "KB per second: " << kilobytes_per_second << std::endl;
		}
		if(out_mb){
			std::cout << "MB per second: " << megabytes_per_second << std::endl;
		}
		if(out_gb){
			std::cout << "GB per second: " << gigabytes_per_second << std::endl;
		}
	}

	// Open file to append to then add data
	if(out_file != ""){
		std::ofstream out;
		out.open(out_file.c_str(),std::ios::app);
		if(out_info != ""){
			out << out_info << "\t";
		}
		out << seconds << " s\t\t";
		if(count_packets){
			out << packets_per_second << " packets/s\t";
		}
		if(out_b){
			out << bytes_per_second << " bytes/s";
			if(out_kb || out_mb || out_gb){
				out << "\t";
			}
		}
		if(out_kb){
			out << kilobytes_per_second << " KB/s";
			if(out_mb || out_gb){
				out << "\t";
			}
		}
		if(out_mb){
			out << megabytes_per_second << " MB/s";
			if(out_gb){
				out << "\t";
			}
		}
		if(out_gb){
			out << gigabytes_per_second << " GB/s";
		}
		out << std::endl;
		out.close();
	}

	return 0;
}

