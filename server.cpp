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
map<int,string> id_name;
set<int> active_users;
map<string, string> username_password;
queue< pair<int, string> > chat;
map< string, vector<string> > groups; 

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
	connfd = accept(listenfd, (struct sockaddr*)NULL, NULL); 
	if(connfd >= 0)
    {
		while(1)
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
    	return true;
    }
    return false;
}


int send_data(string data, int sock)
{
    const char* commy = data.c_str();
    if( (write(sock, commy, strlen(commy)) < 0) )
    {
        return 0;
    }
    return 1;
}

string online_users()
{
	string ret_val = "";
	for (set<int>::iterator it=active_users.begin(); it!=active_users.end(); ++it)
	{
		ret_val += id_name[*it] + "\n";
	}
	return ret_val.substr(0, ret_val.size()-1);
}

void* per_user(void* void_connfd)
{
	long connfd = (long)void_connfd;
	int ohho = 0, logged_in = 0;
	char buffer[256];
    while(1)
    {
    	memset(buffer,'0',sizeof(buffer));
		ohho = read(connfd,buffer,sizeof(buffer));
		buffer[ohho] = 0;
		cout<<"LOG : "<<buffer<<endl;
		// Extract command type from incoming data
		char *pch = strtok(buffer," ");
		string command(pch);
		logged_in = is_logged_in(connfd);
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
				id_name[connfd] = username;
				active_users.insert(connfd);
			}
			else
			{
	        	commy = "failure";
			}
			send_data(commy, connfd);
		}
		else if(!command.compare("/who") && logged_in)
		{
        	const char* commy = online_users().c_str();
        	send_data(commy, connfd);
     	}
     	else if(!command.compare("/logout") && logged_in)
		{
			int c = connfd;
			active_users.erase(c);
			name_id.erase(id_name[c]);
			id_name.erase(c);
			// Close this connection; destroy thread
			close(c);
			break;
     	}
     	else if(!command.compare("/msg") && logged_in)
     	{
     		pch = strtok (NULL, " ");
			string to(pch);
			pch = strtok (NULL, " ");
			string data(pch);
			chat.push(make_pair(connfd, data));
     	}
    	else if(!command.compare("/create_grp") && logged_in)
    	{
    		pch = strtok (NULL, " ");
    		string g_name(pch);
    		const char* commy;
    		if(groups.find(g_name) == groups.end())
    		{
    			groups[g_name] = vector<string>();
    			groups[g_name].push_back(id_name[connfd]);
    			commy = "Group already exists!";
    		}
    		else
    		{
    			commy = "Group already exists!";
    		}
    		send_data(commy, connfd);
    	}
    	else if(!command.compare("/join_grp") && logged_in)
    	{
    		pch = strtok (NULL, " ");
    		string g_name(pch);
    		const char* commy;
    		if(groups.find(g_name) == groups.end())
    		{
    			commy = "Group doesn't exist!";
    		}
    		else
    		{
    			commy = "Joined group!";
    			groups[g_name].push_back(id_name[connfd]);
    		}
    		send_data(commy, connfd);
	    }	
    	else if(!command.compare("/msg_group") && logged_in)
	    {
	    	pch = strtok (NULL, " ");
    		string g_name(pch);
    		pch = strtok (NULL, " ");
    		string message(pch);
    		if(groups.find(g_name) != groups.end())
    		{
    			const char* commy = "Group doesn't exist!";
    			send_data(commy, connfd);
    		}
    		else
    		{
    			for(vector<string>::iterator it = groups[g_name].begin(); it != groups[g_name].end(); ++it)
    			{
    				chat.push(make_pair(name_id[*it],message));
    			}
    		}
    	}   
	    else if(!command.compare("/send") && logged_in)
    	{   

    	}
    	else if(!command.compare("/recv") && logged_in)
    	{

	    }
	}
}

void* send_back(void* argv)
{
	pair<int, string> x;
	while(true)
	{
		while(chat.size())
		{
			x = chat.front();
			chat.pop();
			send_data(x.second, x.first);
		}	
	}	
}

int main()
{
    pthread_t pot,pot2;
    pthread_create(&pot, NULL, register_user, NULL);
    // pthread_create(&pot2, NULL, send_back, NULL);
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
