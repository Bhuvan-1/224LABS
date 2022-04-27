#include<iostream>
using namespace std;

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <openssl/md5.h>

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <fstream>
#include <cmath>
#include <cstdlib>

#include <filesystem>
#include <sys/stat.h>
#include <sys/mman.h>
#include <vector>
#include <dirent.h>

#define LO "127.0.0.1"
#define BACKLOG 20



unsigned char result[MD5_DIGEST_LENGTH];

// Print the MD5 sum as hex-digits.
void print_md5_sum(unsigned char* md) {
    int i;
    for(i=0; i <MD5_DIGEST_LENGTH; i++) {
            printf("%02x",md[i]);
    }
}

// Get the size of the file by its file descriptor
unsigned long get_size_by_fd(int fd) {
    struct stat statbuf;
    if(fstat(fd, &statbuf) < 0) exit(-1);
    return statbuf.st_size;
}



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

string* split_4(string s){
	string* res = new string[4];
	int counter = 0;
	res[0] = "";
	res[1] = "";
	res[2] = "";
	res[3] = "";
	for (int i = 0; i < s.length(); ++i)
	{
		if(s[i] != '#') res[counter] += s[i];
		else{ counter++; }
	}

	return res;
}

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

bool analyse_reply(char* buf,string ** RESPONSES,int* NIDS,int* NPORTS,string* SFILES,int NUM_FILES,int NUM_NEIGHBOURS,bool* CONNECTED,int exp_seq, int& WHO){

	string tmp = "";
	int i = 0;
	string msg = string(buf);
	int counter = -1;

	int who_send = 0;
	int SEQ;
	int index = -1;

	while(counter < 4*NUM_FILES + 1){
		if(msg[i] != '#'){
			tmp += msg[i];
		}
		else{
			if(counter == -1){
				SEQ = stoi(tmp);
				if(SEQ != exp_seq){  return false; }
				tmp = "";
			}
			else if( counter == 0){
				who_send = stoi(tmp);
				for (int j = 0; j < NUM_NEIGHBOURS; ++j)
				{
					if(NIDS[j] == who_send){
						index = j;
						break;
					}
				}
				WHO = index;
				if(index == -1){  return false; }
				tmp = "";
			}
			else{
				if( counter%4 == 0 ){
					RESPONSES[index][(counter/4) - 1] = tmp;
					tmp = "";
				}
				else{
					tmp += "#";
				}
			}
			counter++;
		}
		i++;
	}

	CONNECTED[index] = true;

	return true;


}

string analyse_file_req(char* buf,int* NIDS,int* NPORTS,string* SFILES,int NUM_FILES,int NUM_NEIGHBOURS,int exp_seq,string* DEPTH2RESPONSES,int neigh_index){

	string tmp = "";
	int i = 0;
	string msg = string(buf);
	int counter = 0;

	int SEQ;

	string file_req;
	string depth2info;
	int max_count = 3;

	while(counter < max_count){
		if(msg[i] != '#'){
			tmp += msg[i];
		}
		else{
			if(counter == 0){
				SEQ = stoi(tmp);
				//if(SEQ != exp_seq){  return false; }
				tmp = "";
			}
			else if( counter == 1){
				file_req = tmp;
				tmp = "";
				max_count = 1 + file_req.length() + 1;
			}
			else if(counter >= 2){
				tmp += "#";
				depth2info = tmp;
			}
			counter++;
		}
		i++;
	}


	DEPTH2RESPONSES[neigh_index] = depth2info ;

	return file_req;

}

bool analyse_depth2_info(char* buf,int &NUM_2_NEIGHBOURS,int* N2IDS,int * N2PORTS,int* N2PRIV_IDS,int index){
	string tmp = "";
	int i = 0;
	string msg = string(buf);
	int counter = -1;
	int max_count = 1;

	int num_connectors = 0;

	while(counter < max_count){
		if(msg[i] != '#'){
			tmp += msg[i];
		}
		else{
			if(counter == -1){
				num_connectors = stoi(tmp);
				max_count = 3*stoi(tmp);
				tmp = "";
			}
			else if( counter%3 == 0){
				N2IDS[NUM_2_NEIGHBOURS + (counter/3) ] = stoi(tmp);
				tmp = "";
			}
			else if(counter%3 == 1){
				N2PORTS[NUM_2_NEIGHBOURS + (counter/3) ] = stoi(tmp);
				tmp = "";
			}
			else if(counter%3 == 2){
				N2PRIV_IDS[NUM_2_NEIGHBOURS + (counter/3) ] = stoi(tmp);
				tmp = "";
			}
			counter++;
		}
		i++;
	}

	NUM_2_NEIGHBOURS += num_connectors;

	return true;
}


string d2file_request(string* SFILES,int NUM_FILES,int NUM_2_NEIGHBOURS,int index,int* N2IDS,int MY_ID,int* DEPTHS,int* FIDS){
	int num = 0;
	string req = "";

	for (int i = 0; i < NUM_FILES; ++i)
	{
		if(DEPTHS[i] == 2 && FIDS[i] == N2IDS[index]){
			num++;
			req += SFILES[i] + "#";
		}
	}
	return (to_string(MY_ID) + "#" + to_string(num) + "#" + req);
}

string* analyse_d2fr(char* buf,int &N,int &index,int* N2IDS,int NUM_2_NEIGHBOURS){
	string tmp = "";
	int i = 0;
	string msg = string(buf);
	int counter = -1;
	int max_count = 1;

	int id = 0;
	string* res;

	while(counter < max_count){
		if(msg[i] != '#'){
			tmp += msg[i];
		}
		else{
			if(counter == -1){
				id = stoi(tmp);
				tmp = "";
			}
			else if( counter == 0){
				N = stoi(tmp);
				max_count = 1 + N;
				tmp = "";
				res = new string[N];
			}
			else{
				res[counter-1] = tmp;
				tmp = "";
			}

			counter++;
		}
		i++;
	}

	for (int i = 0; i < NUM_2_NEIGHBOURS; ++i)
	{
		if(N2IDS[i] == id){
			index = i;
		}
	}

	return res;


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


		sockfd = socket(AF_INET, SOCK_STREAM, 0); // do some error checking!

		for (int i = 0; i < NUM_NEIGHBOURS; ++i)
		{
			sockfd_send[i] = socket(AF_INET, SOCK_STREAM, 0);


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
	// This time, upgrading from phase-1, we also give the list of files we wanted to search for. 


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

					//***See prev vrsion
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

						}
						else{
						}

				}


			}
		}


	//---------------------------------------------------------PRINTING ALL THE FILE NEIGHBOUR INFOS.

		int order[NUM_NEIGHBOURS]; 		//--Incresing order of neighbour ids.
		id_sort(NIDS,order,NUM_NEIGHBOURS);


		for (int i = 0; i < NUM_NEIGHBOURS; ++i)
		{
			int j = order[i];
			cout << "Connected to " << NIDS[j] << " with unique-ID " << PRIVATE_IDS[j] << " on port " << NPORTS[j] << endl;
		}

	// ----------------------------------------------- part5 : ANALYSING ALL THE FILES INFO AND GENERATING RESPONSES.

		string replies[NUM_NEIGHBOURS];

		for (int i = 0; i < NUM_NEIGHBOURS; ++i)
		{
			replies[i] = "";
			for (int j = 0; j < DATA[i].numS_files; ++j)
			{
				int depth = 0;
				int id = 0;
				int uniq_id = 0;
				int port = 0;

				string file = DATA[i].sfiles[j];

				for (int k = 0; k < files.size(); ++k)
				{
					if(file == string(files[k])){
						depth = 1;
						id = MY_ID;
						uniq_id = MY_UNIQUE_ID;
						port = MYPORT;
						break;
					}
				}

				if( depth == 0 ){
					for (int k = 0; k < NUM_NEIGHBOURS ; ++k)
					{
						if( k == i) continue;
						for (int l = 0; l < DATA[k].numH_files; ++l)
						{
							if( file == DATA[k].hfiles[l]){
								if( depth == 0){
									depth = 2;
									id = NIDS[k];
									uniq_id = PRIVATE_IDS[k];
									port = NPORTS[k];
								}
								else{
									if( uniq_id > PRIVATE_IDS[k] ){
										id = NIDS[k];
										uniq_id = PRIVATE_IDS[k];
										port = NPORTS[k];										
									}
								}
							}
						}
					}
				}


			replies[i] += to_string(depth) + "#" + to_string(id) + "#" + to_string(uniq_id) + "#" +  to_string(port) + "#" ;

			}
		}

	//---------------------------------------------------------------------------PART 6 : SENDING THE REPLIES,......
		string** RESPONSES = new string*[NUM_NEIGHBOURS];

		for (int i = 0; i < NUM_NEIGHBOURS; ++i)
		{
			RESPONSES[i] = new string[NUM_FILES];
			CONNECTED[i] = false;			
		}


		for( int i = 0 ; i < NUM_NEIGHBOURS; i++){

			bool flag;
			int index = 0;

			if(NIDS[i] > MY_ID){

				// --- RECEIVING HIS REPLY.
					char* buf = new char[400];
					int len_buf = 400;
					bool flag = false;
					int bytes_rcved = recv(sockfd_recv[i], buf, len_buf , 0);
					flag = analyse_reply(buf,RESPONSES,NIDS,NPORTS,SFILES,NUM_FILES,NUM_NEIGHBOURS,CONNECTED,3,index);

				// --- SENDING HIS REPLY ..
					string msg2 = "4#" + to_string(MY_ID) + "#" +  replies[index];
					char* msg = &msg2[0];
					int len = strlen(msg);
					int bytes_sent = send(sockfd_recv[i], msg, len, 0);


			}
			//else You send the requrst to your neighbour for conection,
			else{

					// // --- sending HIS REPLY....
					string msg1 = "3#" + to_string(MY_ID) + "#" +  replies[i];
					char* msg = &msg1[0];
					int len = strlen(msg);
					int bytes_sent = send(sockfd_send[i], msg, len, 0);

					// --receiving his ID + FILE REQUEST.
					char* buf = new char[400];
					int len_buf = 400;
					int bytes_rcved = recv(sockfd_send[i], buf, len_buf , 0);
					flag = analyse_reply(buf,RESPONSES,NIDS,NPORTS,SFILES,NUM_FILES,NUM_NEIGHBOURS,CONNECTED,4,index);
				
			}

		}


	//---------------------------------------------------------------------------PART 7 : ANALYSING THE OBTAINED INFO TO FIND WHOM WE SHOULD CONNECT TO GET THOSE FILES..
	
		int FUIDS[NUM_FILES];
		int FIDS[NUM_FILES];
		int FPORTS[NUM_FILES];
		int DEPTHS[NUM_FILES];
		int NEXTHOPS[NUM_FILES];
		int NEXTHOP_IDS[NUM_FILES];

		// -- bit strings for neighbours.. to ask for which files he want from that neighbour.
		string file_searches[NUM_NEIGHBOURS];
		string Depth2Info[NUM_NEIGHBOURS];
		//string Depth2FileSearches[NUM_NEIGHBOURS];

		for (int i = 0; i < NUM_NEIGHBOURS; ++i)
		{
			file_searches[i] = "";
			Depth2Info[i] = "";
		}

		for (int i = 0; i < NUM_FILES; ++i)
		{

			int depth = 100;
			int id = 0;
			int uid = 0;
			int next_hop_id = 0;
			int next_hop_index = 0;
			int port = 0;

			for (int n = 0; n < NUM_NEIGHBOURS; ++n)
			{
				string* s = split_4(RESPONSES[n][i]);
				if( stoi(s[0]) != 0 && ( depth > stoi(s[0]) || depth == stoi(s[0]) && uid > stoi(s[2]) ) ){
					depth = stoi(s[0]);
					id = stoi(s[1]);
					uid = stoi(s[2]);
					port = stoi(s[3]);
					next_hop_id	 = NIDS[n];
					next_hop_index = n;
				}
			}

			if(depth == 100) depth = 0;

			DEPTHS[i] = depth;
			FUIDS[i] = uid;
			FIDS[i] = id;
			FPORTS[i] = port;
			NEXTHOPS[i] = next_hop_index;
			NEXTHOP_IDS[i] = next_hop_id;


			for (int n = 0; n < NUM_NEIGHBOURS; ++n)
			{
				file_searches[n] += to_string(depth*(NIDS[n] == FIDS[i]));
				if( next_hop_index == n && depth == 2){
					Depth2Info[n] += to_string(id) + "#";
				}
				else{
					Depth2Info[n] += "0#";
				}
			}


		}


	// ----------------------------------------------- creating Downloaded Directory.

		string path = string(argv[2]) + "/Downloaded";
		mkdir(&path[0],0777);

	// ------------------------------------------------------------------------PART7 : DEPTH 1  FILE TRANSFERRRRRRRR.

		string DEPTH2RESPONSES[NUM_NEIGHBOURS];

		for( int i = 0 ; i < NUM_NEIGHBOURS; i++){

			bool flag;
			string req_str;

			if(NIDS[i] > MY_ID){

				// --- RECEIVING HIS REPLY.
					char* buf = new char[200];
					int len_buf = 200;
					bool flag = false;
					int bytes_rcved = recv(sockfd_recv[i], buf, len_buf , 0);
					req_str = analyse_file_req(buf,NIDS,NPORTS,SFILES,NUM_FILES,NUM_NEIGHBOURS,3,DEPTH2RESPONSES,i);

				for (int j = 0; j < DATA[i].numS_files; ++j)
				{
					if( req_str[j] == '1' ){
							string path = string(argv[2]) + '/' + DATA[i].sfiles[j];
							char* path_c = &path[0];
						    ifstream file(path_c, ios::binary);
						    file.seekg(0, ios::end);
						    int size = file.tellg();
						    file.seekg(0, ios::beg);
						    char* buffer = new char[size];
						    file.read(buffer, size);
						    file.close();

						    int* fsize = &size;
						    int err = send(sockfd_recv[i], (char*)fsize, sizeof(int), 0);

						    err = send(sockfd_recv[i], buffer, size, 0);

						    delete[] buffer;
					}
				}	

				// --- SENDING HIS REPLY ..
					string msg2 = "4#" +  file_searches[i] + "#" + Depth2Info[i];
					char* msg = &msg2[0];
					int len = strlen(msg);
					int bytes_sent = send(sockfd_recv[i], msg, len, 0);


				for (int j = 0; j < NUM_FILES; ++j)
				{
					if(file_searches[i][j] == '1'){

						    int filesize = 0;
						    int err = recv(sockfd_recv[i], (char*)&filesize, sizeof(filesize), 0);


						    char* buffer = new char[filesize];
						    memset(buffer, '\0', filesize);

						    err = recv(sockfd_recv[i], buffer, filesize, MSG_WAITALL);

						    string saveas = string(argv[2]) + "/Downloaded" +  "/" +  SFILES[j];
						    ofstream file(&saveas[0], ios::binary);
						    file.write(buffer, filesize);
						    delete[] buffer;
						    file.close(); 

					}
				}

					delete[] buf;
				
			}
			//else You send the requrst to your neighbour for conection,
			else{

					// // --- sending HIS REPLY....
						string msg1 = "3#" +  file_searches[i] + "#" + Depth2Info[i];
						char* msg = &msg1[0];
						int len = strlen(msg);
						int bytes_sent = send(sockfd_send[i], msg, len, 0);


					for (int j = 0; j < NUM_FILES; ++j)
					{
						if(file_searches[i][j] == '1'){
							    int filesize = 0;
							    int err = recv(sockfd_send[i], (char*)&filesize, sizeof(filesize), 0);

							    char* buffer = new char[filesize];
							    memset(buffer, '\0', filesize);

							    err = recv(sockfd_send[i], buffer, filesize, MSG_WAITALL);

							    string saveas = string(argv[2]) + "/Downloaded" + "/" +  SFILES[j];
							    ofstream file(&saveas[0], ios::binary);
							    file.write(buffer, filesize);
							    delete[] buffer;
							    file.close(); 

						}
					}

					// --receiving his FILE REQUEST.
						char* buf = new char[200];
						int len_buf = 200;
						int bytes_rcved = recv(sockfd_send[i], buf, len_buf , 0);
						req_str = analyse_file_req(buf,NIDS,NPORTS,SFILES,NUM_FILES,NUM_NEIGHBOURS,4,DEPTH2RESPONSES,i);

					for (int j = 0; j < DATA[i].numS_files; ++j)
					{
						if( req_str[j] == '1' ){

								string path = string(argv[2]) + '/' + DATA[i].sfiles[j];
								char* path_c = &path[0];
							    ifstream file(path_c, ios::binary);
							    file.seekg(0, ios::end);
							    int size = file.tellg();
							    file.seekg(0, ios::beg);
							    char* buffer = new char[size];
							    file.read(buffer, size);
							    file.close();

							    int* fsize = &size;
							    int err = send(sockfd_send[i], (char*)fsize, sizeof(int), 0);

							    err = send(sockfd_send[i], buffer, size, 0);

							    delete[] buffer;
						}
					}	


					delete[] buf;					
			}


		}


	// ------------------------------------------------------------------------ PART8 ANALYSING THE DEPTH 2 RESPOBNSES RECEIVED TO FIGURE WHAT TO TLL TO NEIGHBOURS.
		string DEPTH2_CONNECTION_REPLIES[NUM_NEIGHBOURS];	
		int NUM_CONNECTIONS[NUM_NEIGHBOURS];


		for (int i = 0; i < NUM_NEIGHBOURS; ++i)
		{
			DEPTH2_CONNECTION_REPLIES[i] = "";
			NUM_CONNECTIONS[i] = 0;
		}


		for (int i = 0; i < NUM_NEIGHBOURS; ++i)
		{
			string tmp = "";
			string msg = DEPTH2RESPONSES[i];
			bool needed[NUM_NEIGHBOURS] = {false};
			for (int j = 0; j < msg.length() ; ++j)
			{
				if(msg[j] != '#'){tmp += msg[j]; }
				else{
					int n = stoi(tmp);
					if(n != 0){
						for (int k = 0; k < NUM_NEIGHBOURS; ++k)
						{
							if(NIDS[k] == n && !needed[k]){
								DEPTH2_CONNECTION_REPLIES[k] += to_string(NIDS[i]) + "#" + to_string(NPORTS[i]) + "#" +  to_string(PRIVATE_IDS[i]) + "#";
								NUM_CONNECTIONS[k] += 1;
								needed[k]  = true;
								break;
							}
						}
					}
					tmp = "";
				}
			}
		}

	// ------------------------------------------------------------------------- SHARING THE DEPTH 2 CONNECTION INFO TO NEIGHBOURS ---- PRE CONNECTION SETUP

		int NUM_2_NEIGHBOURS = 0;
		int N2IDS[50];
		int N2PORTS[50];
		int N2PRIV_IDS[50];


		for( int i = 0 ; i < NUM_NEIGHBOURS; i++){

			bool flag;

			if(NIDS[i] > MY_ID){

				// --- RECEIVING HIS REPLY.
					char* buf = new char[300];
					int len_buf = 300;
					bool flag = false;
					int bytes_rcved = recv(sockfd_recv[i], buf, len_buf , 0);
					flag = analyse_depth2_info(buf,NUM_2_NEIGHBOURS,N2IDS,N2PORTS,N2PRIV_IDS,i);

				// --- SENDING HIS REPLY ..
					string msg2 = to_string(NUM_CONNECTIONS[i]) + "#" +  DEPTH2_CONNECTION_REPLIES[i];
					char* msg = &msg2[0];
					int len = strlen(msg);
					int bytes_sent = send(sockfd_recv[i], msg, len, 0);

					delete[] buf;

			}
			//else You send the requrst to your neighbour for conection,
			else{

					// // --- sending HIS REPLY....
					string msg1 =  to_string(NUM_CONNECTIONS[i]) + "#" +  DEPTH2_CONNECTION_REPLIES[i];
					char* msg = &msg1[0];
					int len = strlen(msg);
					int bytes_sent = send(sockfd_send[i], msg, len, 0);

					// --receiving his ID + FILE REQUEST.
					char* buf = new char[300];
					int len_buf = 300;
					int bytes_rcved = recv(sockfd_send[i], buf, len_buf , 0);
					flag = analyse_depth2_info(buf,NUM_2_NEIGHBOURS,N2IDS,N2PORTS,N2PRIV_IDS,i);

					delete[] buf;
				
			}

		}		



		int N2N = NUM_2_NEIGHBOURS;
		for (int i = 0; i < NUM_FILES; ++i)
		{
			if(DEPTHS[i] == 2){
				int id = FIDS[i];
				bool found = false;

				for (int j = 0; j < N2N ; ++j)
				{
					if(N2IDS[j] == id) found = true;
				}

				if(!found){
					N2IDS[N2N] = FIDS[i];
					N2PORTS[N2N] = FPORTS[i];
					N2PRIV_IDS[N2N] = FUIDS[i];
					N2N++;
				}


			}
		}
		NUM_2_NEIGHBOURS = N2N;
	// -------------------------------------------------------------------------- FINALLY DEPTH 2 FILE TRANSFER .... :}}}}}

		bool CONNECTED2[NUM_2_NEIGHBOURS];
		for (int i = 0; i < NUM_2_NEIGHBOURS; i++)
		{
			CONNECTED2[i] = false;
		}	

		int sockfd_send2[NUM_NEIGHBOURS];	//array of sockets to send for each neighbour.
		struct sockaddr_in* dest_addr2 = new sockaddr_in[NUM_NEIGHBOURS];		//dest addresses of all neighbours

		for (int i = 0; i < NUM_2_NEIGHBOURS; ++i)
		{
			sockfd_send2[i] = socket(AF_INET, SOCK_STREAM, 0);


			//destination info
			int DEST_PORT = N2PORTS[i];
			char* DEST_IP = LO;


			dest_addr2[i].sin_family = AF_INET; // host byte order
			dest_addr2[i].sin_port = htons(DEST_PORT); // short, network byte order
			dest_addr2[i].sin_addr.s_addr = inet_addr(DEST_IP);
			memset(&(dest_addr2[i].sin_zero), '\0', 8); // zero the rest of the struct
		}


		while( not_all_true(CONNECTED2,NUM_2_NEIGHBOURS) ){
			for( int i = 0 ; i < NUM_2_NEIGHBOURS; i++){

				if(CONNECTED2[i]){ 
					continue;
				}

				int index = 0;
				if(N2IDS[i] > MY_ID){

					new_fd = accept(sockfd,(struct sockaddr *) &their_addr, &sin_size);

					// --- RECEIVING HIS REPLY.
						char* buf = new char[200];
						int len_buf = 200;
						bool flag = false;
						int bytes_rcved = recv(new_fd, buf, len_buf , 0);
						int N = 0;
						string* list = analyse_d2fr(buf,N,index,N2IDS,NUM_2_NEIGHBOURS);

						for (int j = 0; j < N; ++j)
						{
							string path = string(argv[2]) + '/' + list[j];
							char* path_c = &path[0];
						    ifstream file(path_c, ios::binary);
						    file.seekg(0, ios::end);
						    int size = file.tellg();
						    file.seekg(0, ios::beg);
						    char* buffer = new char[size];
						    file.read(buffer, size);
						    file.close();

						    int* fsize = &size;
						    int err = send(new_fd, (char*)fsize, sizeof(int), 0);

						    err = send(new_fd, buffer, size, 0);
				
						    delete[] buffer;
						}	


						// // --- sending HIS REPLY....
						string msg2 = d2file_request(SFILES,NUM_FILES,NUM_2_NEIGHBOURS,index,N2IDS,MY_ID,DEPTHS,FIDS);
						char* msg = &msg2[0];
						int len = strlen(msg);
						int bytes_sent = send(new_fd, msg, len, 0);

						for (int j = 0; j < NUM_FILES; ++j)
						{
							if ( !( DEPTHS[j] == 2 && FIDS[j] == N2IDS[index]) ) continue;

						    int filesize = 0;
						    int err = recv(new_fd, (char*)&filesize, sizeof(filesize), 0);


						    char* buffer = new char[filesize];
						    memset(buffer, '\0', filesize);

						    err = recv(new_fd, buffer, filesize, MSG_WAITALL);

						    string saveas = string(argv[2]) + "/Downloaded" + "/" +  SFILES[j];
						    ofstream file(&saveas[0], ios::binary);
						    file.write(buffer, filesize);
						    delete[] buffer;
						    file.close(); 

						}

						CONNECTED2[index] = true;
						delete[] buf;

					shutdown(new_fd,2);

				}
				//else You send the requrst to your neighbour for conection,
				else{

						int connect_err = connect(sockfd_send2[i] , (struct sockaddr *)&dest_addr2[i], sizeof(struct sockaddr));
						if (connect_err != -1 ){

							int N=  0;
							string* list;

							// // --- sending HIS REPLY....
							string msg1 = d2file_request(SFILES,NUM_FILES,NUM_2_NEIGHBOURS,i,N2IDS,MY_ID,DEPTHS,FIDS);
							char* msg = &msg1[0];
							int len = strlen(msg);
							int bytes_sent = send(sockfd_send2[i], msg, len, 0);

							for (int j = 0; j < NUM_FILES; ++j)
							{
								if ( !( DEPTHS[j] == 2 && FIDS[j] == N2IDS[i]) ) continue;

							    int filesize = 0;
							    int err = recv(sockfd_send2[i], (char*)&filesize, sizeof(filesize), 0);

							    char* buffer = new char[filesize];
							    memset(buffer, '\0', filesize);

							    err = recv(sockfd_send2[i], buffer, filesize, MSG_WAITALL);

							    string saveas = string(argv[2]) + "/Downloaded" + "/" +  SFILES[j];
							    ofstream file(&saveas[0], ios::binary);
							    file.write(buffer, filesize);
							    delete[] buffer;
							    file.close(); 

							}


							// --- RECEIVING HIS REPLY.
								char* buf = new char[200];
								int len_buf = 200;
								bool flag = false;
								int bytes_rcved = recv(sockfd_send2[i], buf, len_buf , 0);
								list = analyse_d2fr(buf,N,index,N2IDS,NUM_2_NEIGHBOURS);

							for (int j = 0; j < N; ++j)
							{
								string path = string(argv[2]) + '/' + list[j];
								char* path_c = &path[0];
							    ifstream file(path_c, ios::binary);
							    file.seekg(0, ios::end);
							    int size = file.tellg();
							    file.seekg(0, ios::beg);
							    char* buffer = new char[size];
							    file.read(buffer, size);
							    file.close();

							    int* fsize = &size;
							    int err = send(sockfd_send2[i], (char*)fsize, sizeof(int), 0);

							    err = send(sockfd_send2[i], buffer, size, 0);

							    delete[] buffer;
							}	


							delete[] buf;
							CONNECTED2[i] = true;
						}
						else{

						}
					
				}

			}
		}

		for (int i = 0; i < NUM_NEIGHBOURS; ++i)
		{
			shutdown(sockfd_recv[i],2);
			shutdown(sockfd_send[i],2);
		}

		for (int i = 0; i < NUM_2_NEIGHBOURS; ++i)
		{
			shutdown(sockfd_send2[i],2);
		}

		shutdown(sockfd,2);


	// -------------------------------------------------------------------------- MD5HASH PRINTING.
	int file_descript;
    unsigned long file_size;
    char* file_buffer;
    void* F;


	for (int i = 0; i < NUM_FILES; ++i)
	{
		if( DEPTHS[i] != 0 ){
			string path = string(argv[2]) + "/Downloaded/" + SFILES[i];

		    file_descript = open(&path[0], O_RDONLY);
		    if(file_descript < 0) exit(-1);

		    file_size = get_size_by_fd(file_descript);

		    F = mmap(0, file_size, PROT_READ, MAP_SHARED, file_descript, 0);
		    file_buffer = (char*)F;
		    MD5((unsigned char*) file_buffer, file_size, result);
		    munmap(file_buffer, file_size); 

		  	cout << "Found " << SFILES[i] <<  " at " << FUIDS[i] << " with MD5 "; 
		    print_md5_sum(result);
		    cout << " at depth " << DEPTHS[i] << endl;
		}
		else{
			cout << "Found " << SFILES[i] <<  " at " << 0 << " with MD5 0 at depth " << 0 << endl; 
		}
	
	}	



}