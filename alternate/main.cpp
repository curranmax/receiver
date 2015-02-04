
#include <gm_utils.h>
#include <FSO.h>

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <iostream>
#include <signal.h>

#define SWAP(x,y) {FSO* t = x; x = y; y = t; }

volatile sig_atomic_t flag = 0;
void signal_handler(int signal){
	flag = 1;
}

int main(int argc, char **argv){
	// Input arguments
	//	-input infile_name
	//	-freq frequency_of_switch
	float freq = 0; // In seconds (10^-6 seconds)
	std::string fname = "";

	for(int i = 0; i < argc; i++){
		if(strcmp(argv[i],"-input") == 0 && i + 1 < argc ){
			fname = argv[i+1];
			i++;
		} else if((strcmp(argv[i],"-f") == 0 ||strcmp(argv[i],"-freq") == 0 || strcmp(argv[i],"-frequency") == 0) && i + 1 < argc ){
			freq = atof(argv[i+1]);
			i++;
		}
	}

	//Convert frequency to microseconds (units of usleep())
	freq = freq * 1000000;

	if(fname == ""){
		std::cerr << "No input file given" << std::endl;
		exit(0);
	}

	fso_map fsos = readData(fname);

	if(fsos.size() != 3){
		std::cerr << "Not correct setup of network for this program" << std::endl;
		exit(0);
	}

	FSO *fso_with_gm = NULL,*fso_target1 = NULL,*fso_target2 = NULL;

	// Check network structure
	for(fso_map_itr itr = fsos.begin(); itr != fsos.end(); itr++){
		if(itr->second->hasGM()){
			if(fso_with_gm == NULL){
				fso_with_gm = itr->second;
				continue;
			}
		} else{
			if(fso_target1 == NULL){
				fso_target1 = itr->second;
				continue;
			}else if(fso_target2 == NULL){
				fso_target2 = itr->second;
				continue;
			}
		}
		std::cerr << "Not correct setup of network for this program" << std::endl;
		exit(0);
	}

	if(fso_with_gm == NULL || fso_target1 == NULL || fso_target2 == NULL || !fso_with_gm->hasLink(fso_target1) || !fso_with_gm->hasLink(fso_target2)){
		std::cerr << "Not correct setup of network for this program" << std::endl;
		exit(0);
	}

	fso_with_gm->connectGM();

	
	if(freq > 0){
		// Alternate forever
		signal(SIGINT,signal_handler);
		int n_swaps = 0;
		while(true){
			fso_with_gm->setActiveLink(fso_target1);
			SWAP(fso_target1,fso_target2);
			usleep(freq);
			if(flag == 1){
				fso_with_gm->disconnectGM();
				break;
			}
			n_swaps += 1;
		}
		std::cout << std::endl << "Number of times link switched: " << n_swaps << std::endl;
	} else {
		std::string user_input = "";
		std::cout << "Enter 'q' to quit and anything else to switch mirror" << std::endl;
		while(user_input != "q"){
			fso_with_gm->setActiveLink(fso_target1);
			SWAP(fso_target1,fso_target2);
			std::cout << "#### ";
			std::cin >> user_input;
		}
	}

}
