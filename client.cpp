#include <bits/stdc++.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>

#define SERVER_IP "127.0.0.1"
#define REGISTER_PORT 5004
#define IRC_PORT 5005

using namespace std;

int send_data(string data, int sock)
{
    const char* commy = data.c_str();
    if( (write(sock, commy, strlen(commy)) < 0) )
    {
        return 0;
    }
    return 1;
}

void* server_feedback(void* void_listenfd)
{
	long listenfd = (long)void_listenfd;
	char buffer[256];
	int ohho = 0;
	while(1)
	{
		memset(buffer,'0',sizeof(buffer));
		ohho = read(listenfd,buffer,sizeof(buffer));
		buffer[ohho] = 0;
		cout<<">> "<<buffer<<endl;
	}
}

int create_socket_and_connect(int port)
{
	int sock = 0;
	struct sockaddr_in serv_addr;
	if((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        cerr<<">> Socket creation error \n";
        return 0;
    } 
	memset(&serv_addr, '0', sizeof(serv_addr)); 
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port); 
    if(inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr)<=0)
    {
        cerr<<">> Invalid address\n";
        return 0;
    } 
    if( connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        cerr<<">> Connection Failed\n";
        return 0;
    }
    return sock;
}

int main(int argc, char *argv[])
{
	// Establish connection
	long irc_sock,register_sock;
	irc_sock = create_socket_and_connect(IRC_PORT);
	register_sock = create_socket_and_connect(REGISTER_PORT);
    // Create thread for receiving messages
	pthread_t pot;
    pthread_create(&pot, NULL, server_feedback, (void*)irc_sock);

	string send, username, password, command;
	int logged_in = 0;
	cout<<">> Welcome to IRsea!\n";
	while(1)
	{
		cin>>command;
		if(!command.compare("/exit"))
		{
			// Communicate logout action to server
			close(irc_sock);
			close(register_sock);
			cout<<">> Exiting!\nThanks for using IR-sea!\n";
			return 0;
		}
		else if(!command.compare("/logout") && logged_in)
		{
			if(!send_data(command ,irc_sock))
			{
				cout<<">> Error logging out. Please try again.\n";
			}
			else
			{
				logged_in = 0;
				cout<<">> Logged out!\n";
			}
		}
		else if(!command.compare("/register"))
		{
			cin>>username;
			cin>>password;
			send = username + " " + password;
			if(!send_data(send, register_sock))
			{
				cout<<">> Error in registration. Please try again.\n";
			}
		}
		else if(!command.compare("/login"))
		{	
			cin>>username;
			cin>>password;
			send = "/login " + username + " " + password;
			if(!send_data(send, irc_sock))
			{
				cout<<">> Error logging-in. Please try again.\n";
			}
			else
			{
				logged_in = 1;
			}
		}
		else if(!command.compare("/who") && logged_in)
		{
			send = command + " " +  username + " " + password;
			if(!send_data(send, irc_sock))
			{
				cout<<">> Error communicating with server. Please try again.\n";
			}
		}
		else if(!command.compare("/msg") && logged_in)
		{
			cin>>username;
			getline(cin, password);
			send = command + " " + username + " " + password;
			if(!send_data(send, irc_sock))
			{
				cout<<">> Error communicating with server. Please try again.\n";
			}
		}
		else if(!command.compare("/create_grp") && logged_in)
		{
			cin>>username;
			send = command + " " + username;
			if(!send_data(send, irc_sock))
			{
				cout<<">> Error communicating with server. Please try again.\n";
			}
		}
		else if(!command.compare("/join_grp") && logged_in)
		{
			cin>>username;
			send = command + " " + username;
			if(!send_data(send, irc_sock))
			{
				cout<<">> Error communicating with server. Please try again.\n";
			}
		}
		else if(!command.compare("/msg_group") && logged_in)
		{
			cin>>username;
			getline(cin, password);
			send = command + " " + username + " " + password;
			if(!send_data(send, irc_sock))
			{
				cout<<">> Error communicating with server. Please try again.\n";
			}
		}
		else if(!command.compare("/send") && logged_in)
		{	

		}
		else if(!command.compare("/recv") && logged_in)
		{

		}
		else
		{
			if(logged_in)
			{
				cout<<">> Invalid command! Please read the README for the list of supported commands\n";
			}
			else
			{
				cout<<">> Not signed in!\n";	
			}
		}
	}
	return 0;
}
