#include<iostream>
using namespace std;

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


#include <string.h>
#include <fstream>
#include <cmath>
#include <cstdlib>

#include <filesystem>
#include <vector>
#include <dirent.h>

#define LO "127.0.0.1"
#define BACKLOG 20

bool sort_names(string* S,int N){
	bool flag = 1;
	string tmp;

	while( flag ){
		flag = 0;
		for (int i = 0; i < N-1; ++i)
		{
			if(S[i] > S[i+1] ){
				tmp = S[i];
				S[i] = S[i+1];
				S[i+1] = tmp;
				flag = 1;
			}
		}
	}
	return true;

}

// -- function to tell if atleast one "FALSE" in array
bool not_all_true(bool* A, int N){
	for (int i = 0; i < N; ++i)
	{
		if(A[i] == false){
			return true;
		}
	}
	return false;
}

// -- validating the message recieved .. check if correct format. and CONNECT to neighbour
// -- message format "LEN_ID_PRIVATE-ID_PORT"
bool analyse_msg(char* buf,int* PRIVATE_IDS,bool* CONNECTED,int* NPORTS,int* NIDS,int NUM_NEIGHBOURS){
	
	int DATA[4]; //--LEN of useful msg ..without header, ID,PRIV_ID,PORT..

	string num = "";
	int counter = 0;
	int max_index = 50;
	for (int i = 0; i < max_index; ++i)
	{
		if (! (buf[i] == '_' ||  (buf[i] - 48 >= 0 && buf[i] - 48 <= 9) ) ){ //--if any character other than [0-9],'_' appears, then exit.
			return false;
		}
		if(buf[i] != '_'){
			num += buf[i];
		}
		else{
			DATA[counter] = stoi(num);
			num = "";
			counter++;
			if(counter == 1){
				max_index = i + DATA[0] + 1;
			}
		}
	}

	for (int i = 0; i < NUM_NEIGHBOURS; ++i)
	{
		if(NIDS[i] == DATA[1] && NPORTS[i] == DATA[3]){
			CONNECTED[i] = true;
			PRIVATE_IDS[i] = DATA[2];
		}
	}

}


//sorting array A, and giving the indexes sorted in array Ind.
bool id_sort(int* A, int* Ind, int N){

	bool found[N] = {false};
	int k = 0;

	while(not_all_true(found,N)){

		int min_id = pow(2,30);
		int index = -1;


		for (int i = 0; i < N ; ++i)
		{
			if(A[i] < min_id && !found[i] ){
				min_id = A[i];
				index = i;
			}
		}

		found[index] = true;
		Ind[k++] = index;

	}

	return true;
}


int main(int argc, char* argv[] ){

	//------------------------------------------- PART1 : READING FROM THE CONFIG FILE.
		ifstream CONFIG_FILE(argv[1]);

		//--host ports,ids
		int MYPORT = 0;
		int MY_ID = 0;
		int MY_UNIQUE_ID = 0;

		//---next-neighbour information
		int NUM_NEIGHBOURS = 0;
		int* NIDS;
		int* NPORTS;

		//--files to search for,
		int NUM_FILES = 0;
		string* SFILES;


		//---reading host details
		CONFIG_FILE >> MY_ID;
		CONFIG_FILE >> MYPORT;
		CONFIG_FILE >> MY_UNIQUE_ID;

		//---neighbour details
		CONFIG_FILE >> NUM_NEIGHBOURS;

		NIDS = new int[NUM_NEIGHBOURS];
		NPORTS = new int[NUM_NEIGHBOURS];

		for(int i = 0; i < NUM_NEIGHBOURS; i++){
			CONFIG_FILE >> NIDS[i];
			CONFIG_FILE >> NPORTS[i];
		}



		// -- file details
		CONFIG_FILE >> NUM_FILES;

		SFILES = new string[NUM_FILES];

		for(int i = 0; i < NUM_FILES; i++){
			CONFIG_FILE >> SFILES[i];
		}

		sort_names(SFILES,NUM_FILES);


		// Close the file
		CONFIG_FILE.close();


	//----------------------------------------------- PART2 : OPENING SOCKETS and BINDING the LISTEN() Socket.

	int sockfd;
	int sockfd_send[NUM_NEIGHBOURS];	//array of sockets to send for each neighbour.
	struct sockaddr_in* dest_addr = new sockaddr_in[NUM_NEIGHBOURS];		//dest addresses of all neighbours

	// -- 2 kinds of  sockets one for listen() and for connect()
	sockfd = socket(AF_INET, SOCK_STREAM, 0); // do some error checking!

	for (int i = 0; i < NUM_NEIGHBOURS; ++i)
	{
		sockfd_send[i] = socket(AF_INET, SOCK_STREAM, 0);


		//destination info
		int DEST_PORT = NPORTS[i];
		char* DEST_IP = LO;


		dest_addr[i].sin_family = AF_INET; // host byte order
		dest_addr[i].sin_port = htons(DEST_PORT); // short, network byte order
		dest_addr[i].sin_addr.s_addr = inet_addr(DEST_IP);
		memset(&(dest_addr[i].sin_zero), '\0', 8); // zero the rest of the struct
	}


	struct sockaddr_in my_addr;	

	my_addr.sin_family = AF_INET; // host byte order
	my_addr.sin_port = htons(MYPORT); // short, network byte order
	my_addr.sin_addr.s_addr = INADDR_ANY; // use my IP address;
	memset(&(my_addr.sin_zero), '\0', 8); // zero the rest of the struct



	// don’t forget your error checking for bind():
	int bind_err = -1;

	while(bind_err == -1){
		bind_err = bind(sockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr));
	}


	// ----------------------------------------------------------- PART3 : READING FILES IN DIRECTORY
		char* DIR_PATH = argv[2];

	    DIR *dir; struct dirent *diread;
	    vector<string> files;

	    if ((dir = opendir(DIR_PATH)) != nullptr) {
	        while ((diread = readdir(dir)) != nullptr) {
	        	if(diread->d_type == DT_REG || string(diread->d_name) == "." || string(diread->d_name) == ".."){
	            	files.push_back( string(diread->d_name) );
	            }
	        }
	        closedir (dir);
	    } else {
	        perror ("opendir");
	        return EXIT_FAILURE;
	    }

	    for (auto file : files){
	    	if( string(file) != "." && string(file) != ".."){
	    		cout << file << endl;
	    	}
	    }


	// ----------------------------------------------------------- PART4 : LOOPING THORUGH ALL NEIGHBORS ..AGAIN n AGAIN TILL U GET all info needed, 


	bool CONNECTED[NUM_NEIGHBOURS];
	for (int i = 0; i < NUM_NEIGHBOURS; i++)
	{
		CONNECTED[i] = false;
	}
	int PRIVATE_IDS[NUM_NEIGHBOURS] = {0};



	int new_fd; //new socket returned
	struct sockaddr_in their_addr; // connector’s address information
	unsigned int sin_size;


	sin_size = sizeof(struct sockaddr_in);
	listen(sockfd,BACKLOG);

	while( not_all_true(CONNECTED,NUM_NEIGHBOURS) ){
		for( int i = 0 ; i < NUM_NEIGHBOURS; i++){

			if(CONNECTED[i]){ 
				continue;
			}


			// if neighbour ID > Your ID, then listen() for it to send request to You.
			if(NIDS[i] > MY_ID){


				new_fd = accept(sockfd,(struct sockaddr *) &their_addr, &sin_size);


				// --- sending message
					char msg[40];
					sprintf(msg,"%d_%d_%d_",MY_ID,MY_UNIQUE_ID,MYPORT);
					int len = strlen(msg);
					sprintf(msg,"%d_%d_%d_%d_",len,MY_ID,MY_UNIQUE_ID,MYPORT);
					len = strlen(msg);	
					int bytes_sent = send(new_fd, msg, len, 0);


				// --receiving message
					char buf[40];
					int len_buf = 30;
					int bytes_recved = recv(new_fd, buf, len_buf , 0);

					analyse_msg(buf,PRIVATE_IDS,CONNECTED,NPORTS,NIDS,NUM_NEIGHBOURS);

				shutdown(new_fd,2);

			}
			//else You send the requrst to your neighbour for conection,
			else{

					int connect_err = connect(sockfd_send[i] , (struct sockaddr *)&dest_addr[i], sizeof(struct sockaddr));
					if (connect_err != -1 ){


						// --receiving message
						char buf[40];
						int len_buf = 30;
						int bytes_rcved = recv(sockfd_send[i], buf, len_buf , 0);
						analyse_msg(buf,PRIVATE_IDS,CONNECTED,NPORTS,NIDS,NUM_NEIGHBOURS);

						// --- sending message
						char msg[40];
						sprintf(msg,"%d_%d_%d_",MY_ID,MY_UNIQUE_ID, MYPORT);
						int len = strlen(msg);
						sprintf(msg,"%d_%d_%d_%d_",len,MY_ID,MY_UNIQUE_ID,MYPORT);
						len = strlen(msg);
						int bytes_sent = send(sockfd_send[i], msg, len, 0);		

						shutdown(sockfd_send[i],2);

					}
					else{}

			}


		}
	}

	shutdown(sockfd,2);
	for (int i = 0; i < NUM_NEIGHBOURS; ++i)
	{
		shutdown(sockfd_send[i],2);
	}


	//---------------------------------------------------------PRINTING ALL THE CONNECTION INFOS.

		int order[NUM_NEIGHBOURS]; 		//--order in which we should print the neighbours
		id_sort(NIDS,order,NUM_NEIGHBOURS);

		for (int i = 0; i < NUM_NEIGHBOURS; ++i)
		{
			int j = order[i];
			cout << "Connected to " << NIDS[j] << " with unique-ID " << PRIVATE_IDS[j] << " on port " << NPORTS[j] << endl;
		}




}


