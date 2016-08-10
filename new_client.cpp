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


int register_user(string username, string password)
{
	int sock = 0;
	struct sockaddr_in serv_addr; 
	string combined;
	if((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        cerr<<"Socket creation error \n";
        return 0;
    } 
	memset(&serv_addr, '0', sizeof(serv_addr)); 
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(REGISTER_PORT); 
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
    string command = username + " " + password;
    const char* commy = command.c_str();
    if( (write(sock, commy, strlen(commy)) < 0) )
    {
        return 0;
    }
    return 1;
}

int main(int argc, char *argv[])
{
	string message, receipent, username, password, command;
	int logged_in = 0;
	cout<<"Welcome to IR-sea!\n";
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
			logged_in = 0;
			// Communicate logout action to server
			cout<<"Logged out!";
		}
		else if(!command.compare("/register"))
		{
			cin>>username;
			cin>>password;
			if(register_user(username, password))
			{
				cout<<"Registered successfully! You may now log in\n";
			}
			else
			{
				cout<<"Error in registration. Please try again.";
			}
		}
		else if(!command.compare("/login"))
		{	
			cin>>username;
			cin>>password;
			// Authenticate user. If not registered, ask to. If invalid, throw error.
		}
		else if(!command.compare("/who") && logged_in)
		{
			// Request server for list of people online
		}
		else if(!command.compare("/msg") && logged_in)
		{
			// scanf("%s %s",receipent,message);
			// Look for user in list of logged-in users.
			// If not, throw error. Else, forward message
		}
		else if(!command.compare("/create_grp") && logged_in)
		{
			cin>>receipent;
			// Check if group by same name exists.
			// If does, throw error. Else, create group
		}
		else if(!command.compare("/join_grp") && logged_in)
		{
			cin>>receipent;
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
