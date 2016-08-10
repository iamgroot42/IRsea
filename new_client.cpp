#include <fstream.h>
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


int main(int argc, char *argv[])
{
	char message[1024],receipent[16],username[16],password[16],command[16];
	int logged_in = 0;
	printf("Chat client active.\nEnter 'exit' to quit.\n");
	while(1)
	{
		scanf("%s",command);
		if(!strcmp(command,"/exit"))
		{
			// Communicate logout action to server
			printf("Exiting!\nThanks for using IR-sea!\n");
			exit();
		}
		else if(!strcmp(command,"/logout") && logged_in)
		{
			logged_in = 0;
			// Communicate logout action to server
			printf("Logged out!");
		}
		else if(!strcmp(command,"/register"))
		{
			printf("Enter your desired username: ");
			scanf("%s",username);
			printf("Enter your desired password: ");
			scanf("%s",password);
			// Try registering. If already registered, ask to sign in
			// else, authenticate session and log user in
		}
		else if(!strcmp(command,"/login"))
		{	
			printf("Enter username: ");
			scanf("%s",username);
			printf("Enter password: ");
			scanf("%s",password);
			// Authenticate user. If not registered, ask to. If invalid, throw error.
		}
		else if(!strcmp(command,"/who") && logged_in)
		{
			// Request server for list of people online
		}
		else if(!strcmp(command,"/msg") && logged_in)
		{
			scanf("%s %s",receipent,message);
			// Look for user in list of logged-in users.
			// If not, throw error. Else, forward message
		}
		else if(!strcmp(command,"/create_grp") && logged_in)
		{
			scanf("%s",receipent);
			// Check if group by same name exists.
			// If does, throw error. Else, create group
		}
		else if(!strcmp(command,"/join_grp") && logged_in)
		{
			scanf("%s",receipent);
			// Check if group exists. If yes, add self to group
			// Else, throw error.
		}
		else if(!strcmp(command,"/send") && logged_in)
		{
			scanf("%s %s",receipent,message);
			// Try opening file
			// Check if user exists
			// Upload to server
		}
		else if(!strcmp(command,"/msg_group") && logged_in)
		{	
			scanf("%s %s",receipent,message);
			// Check if group exists
			// Check if joined group
			// Send message to group
		}
		else if(!strcmp(command,"/recv") && logged_in)
		{
			// Wut? :|
		}
		else
		{
			if(logged_in)
			{
				printf("Invalid command! Please read the README for the list of supported commands\n");
			}
			else
			{
				printf("Not signed in!\n");	
			}
		}
	}
	return 0;
}
