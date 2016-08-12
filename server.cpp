#include <bits/stdc++.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h> 

#define REGISTER_PORT 5004
#define IRC_PORT 5005

using namespace std;

map<string,int> name_id;
map<int,bool> active_users;
map<string, string> username_password;
queue< pair<int, string> > chat;


void* register_user(void* argv)
{
    char buffer[256];
    int listenfd = 0, connfd = 0, ohho = 0;
    sockaddr_in serv_addr; 
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&serv_addr, '0', sizeof(serv_addr)); 
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(REGISTER_PORT); 
    bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)); 
    listen(listenfd, 5);
    while(1)
    {
        connfd = accept(listenfd, (struct sockaddr*)NULL, NULL); 
        if(connfd >= 0)
        {
            char *pch;
            memset(buffer,'0',sizeof(buffer));
            ohho = read(connfd,buffer,sizeof(buffer));
            buffer[ohho] = 0;
            pch = strtok(buffer," ");
            string username(pch);
            pch = strtok (NULL, " ");
            string password(pch);
            username_password.insert(make_pair(username,password));
            cout<<"Registered!\n";
        }
    } 
}


int valid_login(string username, string password)
{
    if(username_password.find(username) != username_password.end())
    {
        if(password.compare(username_password[username]) == 0)
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }
    return 0;
}

bool is_logged_in(int x)
{
	if(active_users.find(x) != active_users.end())
    {
    	return active_users[x];
    }
    return false;
}

void* per_user(void* void_connfd)
{
	long connfd = (long)void_connfd;
	char *pch;
	int ohho = 0, logged_in = 0;
	char buffer[256];
    memset(buffer,'0',sizeof(buffer));
    while(1)
    {
		ohho = read(connfd,buffer,sizeof(buffer));
		buffer[ohho] = 0;
		// Extract command type from incoming data
		pch = strtok(buffer," ");
		string command(pch);
		logged_in = is_logged_in(connfd);
		cout<<command<<endl;
		if(!command.compare("/login"))
		{
			pch = strtok (NULL, " ");
			string username(pch);
			pch = strtok (NULL, " ");
			string password(pch);
			logged_in = valid_login(username, password);
			const char* commy;
			if(logged_in)
			{
				commy = "success";  
				name_id[username] = connfd;
				active_users[connfd] = true;
			}
			else
			{
	        	commy = "failure";
			}
			if( (write(connfd, commy, strlen(commy)) < 0) )
			{
		        continue;
			}
		}
		else if(!command.compare("/who") && logged_in)
		{
        	const char* commy = "5 users\n";
        	if( (write(connfd, commy, strlen(commy)) < 0) )
        	{
	            continue;
        	}
     	}
     	else if(!command.compare("/msg") && logged_in)
     	{

     	}
    	else if(!command.compare("/create_grp") && logged_in)
    	{

    	}
    	else if(!command.compare("/join_grp") && logged_in)
    	{

	    }
	    else if(!command.compare("/send") && logged_in)
	    {

    	}   
    	else if(!command.compare("/msg_group") && logged_in)
    	{   

    	}
    	else if(!command.compare("/recv") && logged_in)
    	{

	    }
	}
}

int main()
{
    pthread_t pot;
    pthread_create(&pot, NULL, register_user, NULL);

    int logged_in = 0, listenfd = 0, connfd = 0;
    sockaddr_in serv_addr; 
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&serv_addr, '0', sizeof(serv_addr)); 
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(IRC_PORT); 
    bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)); 
    listen(listenfd, 15);
    while(1)
    {
        connfd = accept(listenfd, (struct sockaddr*)NULL, NULL);
        // New thread per user (for communication)
        pthread_create(&pot, NULL, per_user, (void*)connfd);
    } 
    return 0;
}
