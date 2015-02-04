
#include <iostream>
#include <fstream>
#include <vector>
#include <stdlib.h>

#include <gm.h>
#include <simplefso.h>

void getValues(int &i,int argc,char const *argv[],std::vector<float> &vec) {
	for(; i < argc; ++i){
		if(argv[i][0] != '-'){
			// For now only support values not test files
			vec.push_back(atof(argv[i]));
		} else {
			break;
		}		
	}
}

void processCommandLine(int argc, char const *argv[],
						int &lp,int &fp,int &tp,float &to,float &wt,
						int &nt,float &tl,std::string &of,std::string &cf,
						std::vector<float> &mls,std::vector<float> &tfs,std::vector<float> &nwts,
						bool &one,bool &v){
	for(int i = 1; i < argc; ++i) {
		std::string val = argv[i];
		if((val.compare("-lp") == 0 || val.compare("--local_port") == 0) && i < argc - 1){
			i++;
			lp = atoi(argv[i]);
		}
		if((val.compare("-fp") == 0 || val.compare("--foreign_port") == 0) && i < argc - 1){
			i++;
			fp = atoi(argv[i]);
		}
		if((val.compare("-tp") == 0 || val.compare("--test_port") == 0) && i < argc - 1){
			i++;
			tp = atoi(argv[i]);
		}
		if((val.compare("-nt") == 0 || val.compare("--num_test") == 0) && i < argc - 1){
			i++;
			nt = atoi(argv[i]);
		}
		if((val.compare("-tout") == 0 || val.compare("--time_out") == 0) && i < argc - 1){
			i++;
			to = atof(argv[i]);
		}
		if((val.compare("-w") == 0 || val.compare("--wait_time") == 0) && i < argc - 1){
			i++;
			wt = atof(argv[i]);
		}
		if((val.compare("-len") == 0 || val.compare("--test_length") == 0) && i < argc - 1){
			i++;
			tl = atof(argv[i]);
		}
		if(val.compare("-one") == 0 || val.compare("--one_transmitter") == 0){
			one = true;
		}
		if(val.compare("-v") == 0 || val.compare("--verbose") == 0){
			v = true;
		}
		if((val.compare("-lens") == 0 || val.compare("--msg_lens") == 0) && i < argc - 1){
			i++;
			getValues(i,argc,argv,mls);
			i--;
		}
		if((val.compare("-fs") == 0 || val.compare("--frews") == 0) && i < argc - 1){
			i++;
			getValues(i,argc,argv,tfs);
			i--;
		}
		if((val.compare("-norm") == 0 || val.compare("--norm_wait_time") == 0) && i < argc - 1){
			i++;
			getValues(i,argc,argv,nwts);
			i--;
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
	// Port controller sends from
	int local_port = 8887;
	// Port controller sends to
	int foreign_port = 8888;
	// Port receiver uses for tests
	int test_port = 9898;
	// Timeout controller uses for socket
	float time_out = 1; // in seconds
	// Time controller waits between diong an action and switching gm
	float wait_time = .05; // in seconds

	// Length of messages to test
	std::vector<float> msg_lens;
	// Frequencys to test
	std::vector<float> test_freqs;
	// Wait times to be used by transmitters
	std::vector<float> norm_wait_times;

	int num_test = 1;
	float test_length = 10; // in seconds
	std::string out_file = "";
	std::string config_file = "";

	// If true then only connected to one transmitter and no switching is needed
	bool one_transmitter = false;
	// If true then all messages sent and recieved by controller outputted
	bool verbose = false;
	
	processCommandLine(argc,argv,local_port,foreign_port,test_port,time_out,wait_time,num_test,test_length,out_file,config_file,msg_lens,test_freqs,norm_wait_times,one_transmitter,verbose);

	// Default list
	if(msg_lens.size() == 0){
		msg_lens.push_back(1.0);
	}
	if(test_freqs.size() == 0){
		test_freqs.push_back(1.0);
	}
	if(norm_wait_times.size() == 0){
		norm_wait_times.push_back(-1.0);
	}

	if(one_transmitter){
		test_freqs.clear();
		test_freqs.push_back(1.0);
	} else{
		// Invert frequencies
		for(int i = 0; i < test_freqs.size(); ++i){
			test_freqs[i] = 1.0 / test_freqs[i]; 
		}
	}
	

	// TODO
	//	Create a socket
	//		Set timeout
	//		Create simple ack procedure
	//	Send messages to endpoints
	//	Loop over values and run tests
	//		Create subprocesses

	SimpleFSO* fso = NULL;
	if(!one_transmitter){
		fso = new SimpleFSO(config_file,wait_time);
	}

	Sender sender(local_port,foreign_port,test_port,verbose,fso);

	for(int i = 0; i < msg_lens.size(); ++i){
		for(int j = 0 ; j < test_freqs.size(); ++j){
			for(int k = 0; k < norm_wait_times; ++k){
				for(int x = 0; x < num_test; ++ x){
					// run a test
				}
			}
		}
	}

	return 0;
}
