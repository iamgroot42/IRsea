// Author : iamgroot42

#include <bits/stdc++.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h> 

#define REGISTER_PORT 5004
#define IRC_PORT 5005
#define BUFFER_SIZE 500

using namespace std;

// Basically creates a 1-1 mapping between current socket and username (for efficient access)
map<string,int> name_id;
map<int,string> id_name;
// A set of FDs of currently active users
set<int> active_users;
// Username-password mapping..stored as cache, written to memory when program ends
map<string, string> username_password;
// A queue which contains outgoing p2p data
queue< pair<int, string> > chat;
// A queue which contains outgoing group data
queue< pair<int, pair<string,string> > > chat_grp;
// A mapping of group names and their members
map< string, set<string> > groups;

// Send data back to the client
int send_data(string data, int sock)
{
    const char* commy = data.c_str();
    if( (write(sock, commy, strlen(commy)) < 0) )
    {
        return 0;
    }
    return 1;
}

// Thread to listen to register users
void* register_user(void* argv)
{
	// Read registered accounts from file
	// fstream file((string)argv);
	fstream file("users", ios::app);
	string line;
	if (file.is_open())
  	{
	    while(getline(file,line) )
	    {
	    	char *pch = strdup(line.c_str());
      		string username(pch);
      		pch = strtok(NULL, " ");
      		string password(pch);
      		username_password[username] = password;
    	}
  	}
  	else
  	{
  		cout<<"LOG : users' file not created yet.\n";
  	}
	char buffer[BUFFER_SIZE];
    string confirm = "Registered!";
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
            cout<<"LOG : /register "<<buffer<<endl;
            pch = strtok(buffer," ");
            string username(pch);
            pch = strtok (NULL, " ");
            string password(pch);
            username_password.insert(make_pair(username,password));
            send_data(confirm, connfd);
            // Write to file
            file<<username<<" "<<password<<endl;
        }
    }
    file.close(); 
}

// Validates the given username and password
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

// Checks if the given combination is valid
bool is_logged_in(int x)
{
	if(active_users.find(x) != active_users.end())
    {
    	return true;
    }
    return false;
}

// String representation of all users currently online
string online_users()
{
	string ret_val = "";
	for (set<int>::iterator it=active_users.begin(); it!=active_users.end(); ++it)
	{
		ret_val += id_name[*it] + "\n";
	}
	return ret_val.substr(0, ret_val.size()-1);
}

// A thread spawned per connection, to handle all incoming requests from there
void* per_user(void* void_connfd)
{
	long connfd = (long)void_connfd;
	int ohho = 0, logged_in = 0;
	char buffer[BUFFER_SIZE];
    while(1)
    {
    	memset(buffer,'0',sizeof(buffer));
		ohho = read(connfd,buffer,sizeof(buffer));
		// Client ended connection.. close it
		if(!ohho)
		{
			int c = connfd;
			active_users.erase(c);
			name_id.erase(id_name[c]);
			id_name.erase(c);
			// Close this connection; destroy thread
			close(c);
			return 0;
		}
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
				commy = "Signed in!";  
				name_id[username] = connfd;
				id_name[connfd] = username;
				active_users.insert(connfd);
			}
			else
			{
	        	commy = "Error signing in!";
			}
			send_data(commy, connfd);
		}
		else if(!command.compare("/who") && logged_in)
		{
        	const char* commy = online_users().c_str();
        	send_data(commy, connfd);
     	}
     	else if(!command.compare("/exit") && logged_in)
		{
			int c = connfd;
			active_users.erase(c);
			name_id.erase(id_name[c]);
			id_name.erase(c);
			// Close this connection; destroy thread
			close(c);
			return 0;
     	}
     	else if(!command.compare("/msg") && logged_in)
     	{
     		pch = strtok (NULL, " ");
			string to(pch);
			pch = strtok (NULL, " ");
			string data(pch);
			chat.push(make_pair(name_id[to], data));
     	}
    	else if(!command.compare("/create_grp") && logged_in)
    	{
    		pch = strtok (NULL, " ");
    		string g_name(pch);
    		const char* commy;
    		if(groups.find(g_name) == groups.end())
    		{
    			groups[g_name] = set<string>();
    			groups[g_name].insert(id_name[connfd]);
    			commy = "Group created!";
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
    			groups[g_name].insert(id_name[connfd]);
    		}
    		send_data(commy, connfd);
	    }	
    	else if(!command.compare("/msg_group") && logged_in)
	    {
	    	pch = strtok (NULL, " ");
    		string g_name(pch);
    		pch = strtok (NULL, " ");
    		string message(pch);
    		if(groups.find(g_name) == groups.end())
    		{
    			const char* commy = "Group doesn't exist!";
    			send_data(commy, connfd);
    		}
    		// A sanity check; just in case client modifies their code before running it
    		else if(groups[g_name].find(id_name[connfd]) == groups[g_name].end())
    		{
    			const char* commy = "You're not part of this group!";
    			send_data(commy, connfd);	
    		}
    		else
    		{
    			for(set<string>::iterator it = groups[g_name].begin(); it != groups[g_name].end(); ++it)
    			{
    				// Message sent by user shouldn't coma back to them
    				if(name_id[*it] != connfd)
    				{
    					chat_grp.push(make_pair(name_id[*it],make_pair(g_name, message)));	
    				}
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

// Empties the send-queue by sending messages to respective clients
void* send_back(void* argv)
{
	pair<int, string> x;
	while(true)
	{
		while(chat.size())
		{
			x = chat.front();
			chat.pop();
			string formatted = "(" + id_name[x.first] + ") " + x.second;
			send_data(formatted, x.first);
		}	
	}
}

// Empties the send-queue by sending messages to respective clients
void* send_back_grp(void* argv)
{
	pair<int, pair<string,string> > x;
	while(true)
	{
		while(chat_grp.size())
		{
			x = chat_grp.front();
			chat_grp.pop();
			string formatted = "(" + x.second.first + ") " + x.second.second;
			send_data(formatted, x.first);
		}	
	}	
}


int main()
{
    pthread_t pot,pot2,pot3;
    // Thread to handle registrations
    pthread_create(&pot, NULL, register_user, NULL);
    // Thread to handle out-going p2p messages
    pthread_create(&pot2, NULL, send_back, NULL);
    // Thread to handle out-going group messages
    pthread_create(&pot3, NULL, send_back_grp, NULL);
    // Main thread
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
    	pthread_t pot_temp;
        pthread_create(&pot_temp, NULL, per_user, (void*)connfd);
    } 
    return 0;
}
