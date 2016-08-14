// Author : iamgroot42

#include <bits/stdc++.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/sendfile.h>
#include <arpa/inet.h> 

#define REGISTER_PORT 5004
#define IRC_PORT 5005
#define BUFFER_SIZE 512
#define USER_FILENAME "users"

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
// A one-many mapping of users and files waiting for them
multimap<string, string> waiting_files;
// Thread-safe counter for files
int file_counter = 1;
// Mutex locks for all shared variables
mutex name_id_l, id_name_l, active_users_l, username_password_l, chat_l, chat_grp_l, groups_l, waiting_files_l, file_counter_l;

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
	fstream file(USER_FILENAME, ios::in);
	string line;
	if (file.is_open())
  	{
  		username_password_l.lock();
	    while(getline(file,line))
	    {
      		char* pch = strtok(strdup(line.c_str()), " ");
      		string username(pch);
      		pch = strtok(NULL, " ");
      		string password(pch);
      		username_password[username] = password;
    	}
    	username_password_l.unlock();
    	file.close();
    	file.open(USER_FILENAME, ios::app);
  	}
  	else
  	{
  		cout<<"LOG : users' file not created yet.\n";
  	}
    // Socket creation snippet
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
            if(!ohho)
            {
                continue;
            }
            buffer[ohho] = 0;
            cout<<"LOG : /register "<<buffer<<endl;
            pch = strtok(buffer," ");
            string username(pch);
            pch = strtok (NULL, " ");
            string password(pch);
            // Mutex lock
            username_password_l.lock();
            username_password.insert(make_pair(username,password));
            username_password_l.unlock();
            send_data(confirm, connfd);
            // Write to file (not working rn)
            file<<username<<" "<<password<<endl;
        }
    }
    file.close(); 
}

// Validates the given username and password
int valid_login(string username, string password)
{
	// Mutex lock
	username_password_l.lock();
    if(username_password.find(username) != username_password.end())
    {
        if(password.compare(username_password[username]) == 0)
        {
            username_password_l.unlock();
            return 1;
        }
        else
        {
        	username_password_l.unlock();
            return 0;
        }
    }
    username_password_l.unlock();
    return 0;
}

// Checks if the given combination is valid, and this user is currently logged in
bool is_logged_in(int x)
{
	// Mutex lock
	active_users_l.lock();
	if(active_users.find(x) != active_users.end())
    {
    	active_users_l.unlock();
    	return true;
    }
    active_users_l.unlock();
    return false;
}

// String representation of all users currently online
string online_users()
{
	string ret_val = "";
	// Mutex lock
	active_users_l.lock();
	for (set<int>::iterator it=active_users.begin(); it!=active_users.end(); ++it)
	{
		ret_val += id_name[*it] + "\n";
	}
	active_users_l.unlock();
	return ret_val.substr(0, ret_val.size()-1);
}

// A thread spawned per connection, to handle all incoming requests from there
void* per_user(void* void_connfd)
{
    string current_username;
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
			// Mutex lock
			active_users_l.lock();
			name_id_l.lock();
			id_name_l.lock();
			active_users.erase(c);
			name_id.erase(id_name[c]);
			id_name.erase(c);
			active_users_l.unlock();
			name_id_l.unlock();
			id_name_l.unlock();
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
			const char* commy;
			try
			{
				pch = strtok (NULL, " ");
				string username(pch);
				pch = strtok (NULL, " ");
				string password(pch);
				logged_in = valid_login(username, password);
				if(logged_in)
				{
					commy = "Signed in!"; 
					// Mutex lock
					name_id_l.lock();
					id_name_l.lock();
					active_users_l.lock(); 
                    // Update 1-1(effective) mapping of connectionID and username
                    current_username = username;
					name_id[username] = connfd;
					id_name[connfd] = username;
                    // Update list of active users
					active_users.insert(connfd);
					name_id_l.unlock();
					id_name_l.unlock();
					active_users_l.unlock(); 
				}
				else
				{
		        	commy = "Error signing in!";
				}
				send_data(commy, connfd);
			}
			catch(...)
			{
				commy = "Malformed message!";
				send_data(commy, connfd);
			}
		}
		else if(!command.compare("/who") && logged_in)
		{
        	const char* commy = online_users().c_str();
        	send_data(commy, connfd);
     	}
     	else if(!command.compare("/exit") && logged_in)
		{
			int c = connfd;
			// Mutex lock
			active_users_l.lock();
			name_id_l.lock();
			id_name_l.lock();
			active_users.erase(c);
			name_id.erase(id_name[c]);
			id_name.erase(c);
			active_users_l.unlock();
			name_id_l.unlock();
			id_name_l.unlock();
			// Close this connection; destroy thread
			close(c);
			return 0;
     	}
     	else if(!command.compare("/msg") && logged_in)
     	{
     		const char* commy;
     		try
     		{
     			pch = strtok (NULL, " ");
				string to(pch);
				pch = strtok (NULL, " ");
				string data(pch);
				// Mutex lock
				chat_l.lock();
                name_id_l.lock();
                // Push outgoing message to queue, to be handled by another process
				chat.push(make_pair(name_id[to], data));
                name_id_l.unlock();
                chat_l.unlock();
			}
			catch(...)
			{
				commy = "Malformed message!";
				send_data(commy, connfd);
			}
     	}
    	else if(!command.compare("/create_grp") && logged_in)
    	{
    		const char* commy;
    		try{
    			pch = strtok (NULL, " ");
    			string g_name(pch);
    			// Mutex lock
    			groups_l.lock();
                // Check for duplication of group name
    			if(groups.find(g_name) == groups.end())
    			{
	    			groups[g_name] = set<string>();
    				groups[g_name].insert(current_username);
    				commy = "Group created!";
    			}
    			else
    			{
	    			commy = "Group already exists!";
    			}
    			groups_l.unlock();
    			send_data(commy, connfd);
    		}
    		catch(...)
    		{
    			commy = "Malformed message!";
				send_data(commy, connfd);
    		}
    	}
    	else if(!command.compare("/join_grp") && logged_in)
    	{
    		const char* commy;
    		try{
    			pch = strtok (NULL, " ");
    			string g_name(pch);
    			// Mutex lock
    			groups_l.lock();
                // Check whether group name exists
    			if(groups.find(g_name) == groups.end())
    			{
	    			commy = "Group doesn't exist!";
    			}
    			else
    			{
	    			commy = "Joined group!";
    				groups[g_name].insert(current_username);
    			}
    			groups_l.unlock();
    			send_data(commy, connfd);
    		}
    		catch(...)
    		{
    			commy = "Malformed message!";
				send_data(commy, connfd);
    		}
	    }	
    	else if(!command.compare("/msg_group") && logged_in)
	    {
	    	const char* commy;
	    	try{
	    		pch = strtok (NULL, " ");
    			string g_name(pch);
    			pch = strtok (NULL, " ");
    			string message(pch);
    			// Mutex lock
    			groups_l.lock();
    			if(groups.find(g_name) == groups.end())
    			{
    				groups_l.unlock();
    				commy = "Group doesn't exist!";
    				send_data(commy, connfd);
    			}
    			// A sanity check; just in case client modifies their code before running it
    			else if(groups[g_name].find(current_username) == groups[g_name].end())
    			{
    				groups_l.unlock();
    				commy = "You're not part of this group!";
    				send_data(commy, connfd);	
    			}
    			else
    			{
    				set<string> tempo = groups[g_name];
    				groups_l.unlock();
                    // Mutex lock
    				chat_grp_l.lock();
                    name_id_l.lock();
    				for(set<string>::iterator it = tempo.begin(); it != tempo.end(); ++it)
    				{
    					// Message sent by user shouldn't coma back to them
    					if(name_id[*it] != connfd)
    					{
    						chat_grp.push(make_pair(name_id[*it],make_pair(g_name, message)));	
    					}
    				}
                    name_id_l.unlock();
    				chat_grp_l.unlock();
    			}
    		}
    		catch(...)
    		{
    			commy = "Malformed message!";
				send_data(commy, connfd);
    		}
    	}   
	    else if(!command.compare("/send") && logged_in)
    	{   
            pch = strtok(NULL, " ");
            string to_name(pch);
            // Mutex lock
            file_counter_l.lock();
            name_id_l.lock();
            int temp = file_counter++;
            // Check whether user with this name exists
            if(name_id.find(to_name) == name_id.end())
            {
                const char* commy = "No user by this name exists!";
                send_data(commy, connfd);
                name_id_l.unlock();
                file_counter_l.unlock();
            }
            else
            {
                // Receive and store file into server storage, ready for retreival by targeted user
                name_id_l.unlock();
                file_counter_l.unlock();
                string file_counter_string("temp_data/" + to_string(temp));
                FILE* fp = fopen(file_counter_string.c_str(),"w");
                memset(buffer,'0',sizeof(buffer));
                while((ohho = read(connfd,buffer,sizeof(buffer))) == BUFFER_SIZE)
                {
                    buffer[ohho] = 0;
                    fwrite(buffer , 1 , sizeof(buffer) ,fp);
                    fflush(fp);
                    memset(buffer,'0',sizeof(buffer));
                }
                // Mutex lock
                waiting_files_l.lock();
                waiting_files.insert(make_pair(to_name, file_counter_string));
                waiting_files_l.unlock();
                fclose(fp);
            }
    	}
    	else if(!command.compare("/recv") && logged_in)
    	{
            // Mutex lock
            waiting_files_l.lock();
            multimap<string,string>::iterator it = waiting_files.find(current_username);
            // Check if user has any pending files ready for them
            if(it != waiting_files.end())
            {
                waiting_files_l.unlock();
                string fname = (*it).second;
                FILE* fp = fopen(fname.c_str(),"r");
                // Comunicate start of file transfer
                send_data(command.c_str(), connfd);
                // Send file
                int wth;
                while((wth = sendfile(connfd,fileno(fp),NULL,BUFFER_SIZE)) == BUFFER_SIZE);
                // Remove file from server storage
                // remove(fname);
            }
            else
            {
                waiting_files_l.unlock();
                const char* commy = "No files for you!";
                send_data(commy, connfd);
            }
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
			// Mutex lock
			chat_l.lock();
			x = chat.front();
			chat.pop();
			string formatted = "(" + id_name[x.first] + ") " + x.second;
			send_data(formatted, x.first);
			chat_l.unlock();
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
			// Mutex lock
			chat_grp_l.lock();
			x = chat_grp.front();
			chat_grp.pop();
			string formatted = "(" + x.second.first + ") " + x.second.second;
			send_data(formatted, x.first);
			chat_grp_l.unlock();
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
    int listenfd = 0;
    long connfd = 0;
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
