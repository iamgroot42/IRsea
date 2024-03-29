// Author : iamgroot42

#include <bits/stdc++.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/sendfile.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>

#define REGISTER_PORT 5004 //Port for registrations
#define IRC_PORT 5005 //Port for normal communication
#define BUFFER_SIZE 512 //Maximum size per message

using namespace std;
// Indicator variables for status of server connection, login status
bool server_down = false, logged_in = false;

// Send data via the given socket-fd
int send_data(string data, int sock){
    const char* commy = data.c_str();
    if( (write(sock, commy, strlen(commy)) < 0) ){
        return 0;
    }
    return 1;
}

// Thread to read incoming data (from server)
void* server_feedback(void* void_listenfd){
	long listenfd = (long)void_listenfd;
	char buffer[BUFFER_SIZE];
	int ohho = 0;
	while(1){
		memset(buffer,'0',sizeof(buffer));
		ohho = read(listenfd,buffer,sizeof(buffer));
		// If server shuts down/terminates connection
		if(!ohho){
			cout<<">> Connection with server terminated!"<<endl;
			server_down = true;
			close(listenfd);
			return 0;
		}
		buffer[ohho] = 0;
		// Confirmation for sign-in; update status
		if(!strcmp("Signed in!",buffer)){
			logged_in = true;
		}
		// File being transferred..prepare Uranus
		if(!strcmp("/recv",buffer)){
			FILE* fp = fopen("file_from_server","w");
			memset(buffer,'0',sizeof(buffer));
			while((ohho = read(listenfd,buffer,sizeof(buffer))) > 0){
				buffer[ohho] = 0;
				fwrite(buffer , 1 , ohho ,fp);
				fflush(fp);
				// Buffer ended; stop receiving
				if(ohho < BUFFER_SIZE){
					break;
				}
				memset(buffer,'0',sizeof(buffer));
			}
			cout<<">> File received!"<<endl;	
			fclose(fp);
		}
		// Normal conversation; display on console
		else{
			cout<<">> "<<buffer<<endl;	
		}
	}
}

// Create a socket connection for the given IP and port
int create_socket_and_connect(char* address, int port){
	int sock = 0;
	struct sockaddr_in serv_addr;
	if((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        cerr<<">> Socket creation error"<<endl;
        return 0;
    } 
	memset(&serv_addr, '0', sizeof(serv_addr)); 
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port); 
    if(inet_pton(AF_INET, address, &serv_addr.sin_addr)<=0){
        cerr<<">> Invalid address"<<endl;
        return 0;
    } 
    if( connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){
        cerr<<">> Connection Failed"<<endl;
        return 0;
    }
    return sock;
}


int main(int argc, char *argv[]){
	// Argument: IP address of server
	if(argc<2){
		cout<<"Usage: "<<argv[0]<<" <server ip>"<<endl;
		return 0;
	}
	// Establish connection
	long irc_sock,register_sock;
	irc_sock = create_socket_and_connect(argv[1], IRC_PORT);
	register_sock = create_socket_and_connect(argv[1], REGISTER_PORT);
    // Create thread for receiving messages on irc socket
	pthread_t pot;
    pthread_create(&pot, NULL, server_feedback, (void*)irc_sock);
    // Create thread for receiving messages on register socket
	pthread_t pot2;
    pthread_create(&pot2, NULL, server_feedback, (void*)register_sock);
	string send, username, password, command, current_group;
	// Current group joined by client, if any
	current_group = "";
	cout<<">> Welcome to IRsea!"<<endl;
	while(1){
		// Kill main thread if server is down.
		if(server_down){
			return 0;
		}
		cin>>command;
		if(!command.compare("/exit")){
			// Communicate logout action to server
			bool kill = true;
			if(logged_in){
				if(!send_data(command ,irc_sock)){
					cout<<">> Error logging out. Please try again."<<endl;
					kill = false;
				}
			}
			if(kill){
				// Kill thread listening for feedback
				pthread_kill(pot,0);
				close(irc_sock);
				close(register_sock);
				cout<<">> Exiting!\nThanks for using IRsea!"<<endl;
				return 0;
			}
		}
		else if(!command.compare("/register")){
			cin>>username;
			cin>>password;
			send = username + " " + password;
			if(!send_data(send, register_sock)){
				cout<<">> Error in registration. Please try again."<<endl;
			}
		}
		else if(!command.compare("/login")){	
			cin>>username;
			cin>>password;
			if(logged_in){
				cout<<">> Already logged in!"<<endl;
			}
			else{
				send = "/login " + username + " " + password;
				if(!send_data(send, irc_sock)){
					cout<<">> Error logging-in. Please try again."<<endl;
				}
			}
		}
		else if(!command.compare("/who") && logged_in){
			send = command + " " +  username + " " + password;
			if(!send_data(send, irc_sock)){
				cout<<">> Error communicating with server. Please try again."<<endl;
			}
		}
		else if(!command.compare("/msg") && logged_in){
			cin>>username;
			getline(cin, password);
			send = command + " " + username + " " + password;
			if(!send_data(send, irc_sock)){
				cout<<">> Error communicating with server. Please try again."<<endl;
			}
		}
		else if(!command.compare("/create_grp") && logged_in){
			cin>>username;
			send = command + " " + username;
			if(!send_data(send, irc_sock)){
				cout<<">> Error communicating with server. Please try again."<<endl;
			}
		}
		else if(!command.compare("/join_grp") && logged_in){
			cin>>username;
			send = command + " " + username;
			if(!send_data(send, irc_sock)){
				cout<<">> Error communicating with server. Please try again."<<endl;
			}
			else{
				current_group = username;
			}
		}
		else if(!command.compare("/msg_group") && logged_in){
			getline(cin, password);
			if(current_group == ""){
				cout<<">> Join a group before you can send messages."<<endl;
			}
			else{
				send = command + " " + current_group + " " + password;
				if(!send_data(send, irc_sock)){
					cout<<">> Error communicating with server. Please try again."<<endl;
				}
			}
		}
		else if(!command.compare("/send") && logged_in){	
			cin>>username>>password;
			try{
				send = command + " " + username;
				if(!send_data(send, irc_sock)){
					cout<<">> Error communicating with server. Please try again."<<endl;
				}
				else{
					// Open file 
					FILE* fp = fopen(password.c_str(),"r");
					if(fp == NULL){
						cout<<">> Error opening file. Please specify correct path!"<<endl;
					}
					else{
						// Send data to server
						int wth;
						while((wth = sendfile(irc_sock,fileno(fp),NULL,BUFFER_SIZE)) == BUFFER_SIZE);
					}
				}
			}
			catch(...){
				cout<<">> Error opening file. Please specify correct path!"<<endl;
				continue;
			}
		}
		else if(!command.compare("/recv") && logged_in){
			// Request server to download any pending file
			if(!send_data(command, irc_sock)){
				cout<<">> Error communicating with server. Please try again."<<endl;
			}
		}
		else{
			// Invalid command(s)
			if(logged_in){
				cout<<">> Invalid command! Please read the README for the list of supported commands"<<endl;
			}
			// Not logged in
			else{
				cout<<">> Not signed in!"<<endl;	
			}
		}
	}
	return 0;
}
