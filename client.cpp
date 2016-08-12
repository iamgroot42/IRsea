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

int send_data(string data, int port, int sock)
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
	int connfd = 0;
	long listenfd = (long)void_listenfd;
	char buffer[256];
    int ohho = 0;
    sockaddr_in serv_addr; 
    memset(&serv_addr, '0', sizeof(serv_addr)); 
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(IRC_PORT); 
    bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)); 
    listen(listenfd, 15);
    while(1)
    {
        connfd = accept(listenfd, (struct sockaddr*)NULL, NULL);
        if(connfd >= 0)
        {
            char *pch;
            memset(buffer,'0',sizeof(buffer));
            ohho = read(connfd,buffer,sizeof(buffer));
            buffer[ohho] = 0;
            cout<<">> "<<buffer<<endl;
        }
    }
}

int create_socket_and_connect(int port)
{
	int sock = 0;
	struct sockaddr_in serv_addr;
	if((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        cerr<<"Socket creation error \n";
        return 0;
    } 
	memset(&serv_addr, '0', sizeof(serv_addr)); 
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port); 
    if(inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr)<=0)
    {
        cerr<<"Invalid address\n";
        return 0;
    } 
    if( connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        cerr<<"Connection Failed\n";
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
	cout<<"Welcome to IRsea!\n";
	while(1)
	{
		cin>>command;
		if(!command.compare("/exit"))
		{
			// Communicate logout action to server
			cout<<"Exiting!\nThanks for using IR-sea!\n";
			return 0;
		}
		else if(!command.compare("/logout") && logged_in)
		{
			if(!send_data(command, IRC_PORT ,irc_sock))
			{
				cout<<"Error logging out. Please try again.\n";
			}
			else
			{
				logged_in = 0;
				cout<<"Logged out!";
			}
		}
		else if(!command.compare("/register"))
		{
			cin>>username;
			cin>>password;
			send = username + " " + password;
			if(!send_data(send, REGISTER_PORT ,register_sock))
			{
				cout<<"Error in registration. Please try again.\n";
			}
		}
		else if(!command.compare("/login"))
		{	
			cin>>username;
			cin>>password;
			send = "/login " + username + " " + password;
			if(!send_data(send, IRC_PORT, irc_sock))
			{
				cout<<"Error logging-in. Please try again.\n";
			}
			else
			{
				logged_in = 1;
			}
		}
		else if(!command.compare("/who") && logged_in)
		{
			send = command + " " +  username + " " + password;
			if(!send_data(send, IRC_PORT, irc_sock))
			{
				cout<<"Error communicating with server. Please try again.\n";
			}
		}
		else if(!command.compare("/msg") && logged_in)
		{
			cin>>username;
			cin>>password;
			send = command + " " + username + " " + password;
			if(!send_data(send, IRC_PORT, irc_sock))
			{
				cout<<"Error communicating with server. Please try again.\n";
			}
		}
		else if(!command.compare("/create_grp") && logged_in)
		{
			// Check if group by same name exists.
			// If does, throw error. Else, create group
		}
		else if(!command.compare("/join_grp") && logged_in)
		{
			// cin>>username;
			// Check if group exists. If yes, add self to group
			// Else, throw error.
		}
		else if(!command.compare("/send") && logged_in)
		{
			// scanf("%s %s",receipent,message);
			// Try opening file
			// Check if user exists
			// Upload to server
		}
		else if(!command.compare("/msg_group") && logged_in)
		{	
			// scanf("%s %s",receipent,message);
			// Check if group exists
			// Check if joined group
			// Send message to group
		}
		else if(!command.compare("/recv") && logged_in)
		{
			// Wut? :|
		}
		else
		{
			if(logged_in)
			{
				cout<<"Invalid command! Please read the README for the list of supported commands\n";
			}
			else
			{
				cout<<"Not signed in!\n";	
			}
		}
	}
	return 0;
}
