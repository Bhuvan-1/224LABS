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


struct neighbr_files{
	int id;
	int numS_files;
	int numH_files;
	string* sfiles;
	string* hfiles;

	public:
		neighbr_files(){
			id= 0;
			numS_files = 0;
			numH_files = 0;
			sfiles = new string[2];
			hfiles = new string[2];
		}
};

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

string file_request(int NUM_FILES, string* SFILES, vector <string> files,int ID,int UID, int PORT,int SEQ_NUM){
	string msg = "";
	msg += to_string(ID) + "#" + to_string(UID) + "#" + to_string(PORT) + "#";
	msg += to_string(NUM_FILES) + "#" + to_string(files.size()-2) + "#" ;

	for (int i = 0; i < NUM_FILES ; ++i){
		msg += SFILES[i] + "#" ;
	}
	
	for (int i = 2; i < files.size(); ++i){
		msg += string(files[i]) + "#";
	}

	// seq # len # id # uid # port # Num_Search # Num_Have # searchfiles... # havefiles....#
	return to_string(SEQ_NUM) + "#" +  to_string(msg.length()) + "#" +  msg;
}


bool analyse_msg(char* buf,int* PRIVATE_IDS,bool* CONNECTED,int *NPORTS,int *NIDS,int NUM_NEIGHBOURS,vector <string> files, neighbr_files* NFILEDATA, int* sockfd_recv, int new_fd,int exp_seq, int exp_id){
	int SEQ_NUM;

	int DATA[5];

	string msg = string(buf);
	int counter = -1;
	string tmp = "";

	string* REQ_FILES;
	string* HAVE_FILES;

	int max_iter = msg.length();

	for (int i = 0; i < max_iter ; ++i)
	{
		if(msg[i] != '#'){
			tmp += msg[i];
		}
		else{
			if( counter == -1){
				SEQ_NUM = stoi(tmp);
				if( SEQ_NUM != exp_seq) return false;
				counter++;
			}
			else if(counter == 0){
				max_iter = i + stoi(tmp) + 1;
				counter++;	
			}
			else if(counter < 5 ){
				DATA[counter-1] = stoi(tmp);
				counter++;
			}
			else if(counter == 5){
				DATA[counter-1] = stoi(tmp);
				counter++;

				REQ_FILES = new string[DATA[3]];
				HAVE_FILES = new string[DATA[4]];
			}
			else if( counter < 6 + DATA[3] ){
				REQ_FILES[counter-6] = tmp;
				counter++;
			}
			else{
				HAVE_FILES[counter -6 - DATA[3]] = tmp;
				counter++;
			}
			tmp = "";
		}
	}

	if(DATA[0] != exp_id && exp_id != -1) return false;

	for (int i = 0; i < NUM_NEIGHBOURS ; ++i)
	{
		if( NIDS[i] == DATA[0] && NPORTS[i] == DATA[2]){
			CONNECTED[i] = true;
			PRIVATE_IDS[i] = DATA[1];

			NFILEDATA[i].id = i;
			NFILEDATA[i].numS_files = DATA[3];
			NFILEDATA[i].numH_files = DATA[4];
			NFILEDATA[i].sfiles = REQ_FILES;
			NFILEDATA[i].hfiles = HAVE_FILES;

			sockfd_recv[i] = new_fd;		//-1 is set for id's I < MY_ID... but we wont use that to recv anyway.
			break;

		}
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
		int sockfd;							//listening sokett.
		int sockfd_send[NUM_NEIGHBOURS];	//array of sockets to send for each neighbour.
		int sockfd_recv[NUM_NEIGHBOURS];	//array of sockets to recieve for each neighbour.
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
			//cout << "Binding Error :" << bind_err << " _ " <<  errno << endl;
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


		neighbr_files DATA[NUM_NEIGHBOURS];		//-- array of structs storing all file info of all neighbours.


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

					// --- sending YOUR ID.+ YOURFILE NAMES + FILE_REQUEST ... SeqUENCE NUM 1.
						string msg1 = file_request(NUM_FILES, SFILES, files,MY_ID,MY_UNIQUE_ID, MYPORT,1);
						char* msg = &msg1[0];
						int len = strlen(msg);
						int bytes_sent = send(new_fd, msg, len, 0);

					// --receiving his ID + HIS FILES + HIS FILE REQUEST. + YOUR REPLY.... SEQUENCE NUM 2.
						bool flag = false;
						char* buf = new char[400];
						int len_buf = 400;
						int bytes_rcved = recv(new_fd, buf, len_buf , 0);
						analyse_msg(buf,PRIVATE_IDS,CONNECTED,NPORTS,NIDS,NUM_NEIGHBOURS,files,DATA,sockfd_recv,new_fd,2,-1);
						
					delete[] buf;
					shutdown(new_fd,2);
						
				}
				//else You send the requrst to your neighbour for conection,
				else{
						int connect_err = connect(sockfd_send[i] , (struct sockaddr *)&dest_addr[i], sizeof(struct sockaddr));
						if (connect_err != -1 ){


							// --receiving his ID + FILE REQUEST.
							char* buf = new char[400];
							int len_buf = 400;
							bool flag = false;
							int bytes_rcved = recv(sockfd_send[i], buf, len_buf , 0);
							analyse_msg(buf,PRIVATE_IDS,CONNECTED,NPORTS,NIDS,NUM_NEIGHBOURS,files,DATA,sockfd_recv,-1,1,NIDS[i]);

							// // --- sending your ID + FILE REQUEST + HIS REPLY
							string msg2 = file_request(NUM_FILES, SFILES, files,MY_ID,MY_UNIQUE_ID, MYPORT,2) ;
							char* msg = &msg2[0];
							int len = strlen(msg);
							int bytes_sent = send(sockfd_send[i], msg, len, 0);

							delete[] buf;
							shutdown(sockfd_send[i],2);

						}
						else{
						}

				}


			}
		}

		shutdown(sockfd,2);
		for (int i = 0; i < NUM_NEIGHBOURS; ++i)
		{
			shutdown(sockfd_send[i],2);
			shutdown(sockfd_recv[i],2);
		}

	//---------------------------------------------------------PRINTING ALL THE FILE NEIGHBOUR INFOS.

		int order[NUM_NEIGHBOURS]; 		//--Incresing order of neighbour ids.
		id_sort(NIDS,order,NUM_NEIGHBOURS);


		for (int i = 0; i < NUM_NEIGHBOURS; ++i)
		{
			int j = order[i];
			cout << "Connected to " << NIDS[j] << " with unique-ID " << PRIVATE_IDS[j] << " on port " << NPORTS[j] << endl;
		}


	// ----------------------------------------------- part5 : ANALYSING ALL THE FILES INFO AND GENERATING RESULTS.

		for (int n = 0; n < NUM_FILES; ++n)
		{
			string file = SFILES[n];
			int depth = 0;
			int uid = 0;

			for (int i = 0; i < NUM_NEIGHBOURS; ++i)
			{

				for (int j = 0; j < DATA[i].numH_files; ++j)
				{
					if(file == DATA[i].hfiles[j]){
						if(depth == 0){
							depth = 1;
							uid = PRIVATE_IDS[i];
						}
						else{
							if(PRIVATE_IDS[i] < uid){
								depth = 1;
								uid = PRIVATE_IDS[i];
							}
						}
					}

				}
			}

			cout << "Found " << SFILES[n] <<  " at " << uid << " with MD5 0 at depth " << depth << endl;

		}

}
