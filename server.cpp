// Author : iamgroot42

#include <bits/stdc++.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/sendfile.h>
#include <arpa/inet.h> 

#define REGISTER_PORT 5004 //Port for registrations
#define IRC_PORT 5005 //Port for normal communication
#define BUFFER_SIZE 512 //Maximum size per message
#define USER_FILENAME "users" //Filename containing username & passwords

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
// Counter for files
int file_counter = 1;
// Mutex locks for all shared variables
mutex name_id_l, id_name_l, active_users_l, username_password_l, chat_l, chat_grp_l,groups_l, waiting_files_l, file_counter_l;

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

// Read registered accounts from file
void populate_userlist(){
	fstream file(USER_FILENAME, ios::in);
	string line;
	if (file.is_open()){
	    while(getline(file,line)){
      		char* pch = strtok(strdup(line.c_str()), " ");
      		string username(pch);
      		pch = strtok(NULL, " ");
      		string password(pch);
      		username_password[username] = password;
    	}
    	file.close();
  	}
  	else{
  		cout<<"LOG : users' file not created yet.\n";
  	}
}

// Thread to listen to register users
void* register_user(void* argv){
	// Populate username_password
	populate_userlist();
	// Open file for writing username-password pairs
	fstream file(USER_FILENAME, ios::app);
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
	while(1){
		connfd = accept(listenfd, (struct sockaddr*)NULL, NULL); 
		if(connfd >= 0){
            char *pch;
            memset(buffer,'0',sizeof(buffer));
            ohho = read(connfd,buffer,sizeof(buffer));
            if(!ohho){
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
int valid_login(string username, string password){
	// Mutex lock
	username_password_l.lock();
    if(username_password.find(username) != username_password.end()){
        if(password.compare(username_password[username]) == 0){
            username_password_l.unlock();
            return 1;
        }
        else{
        	username_password_l.unlock();
            return 0;
        }
    }
    username_password_l.unlock();
    return 0;
}

// Checks if the given combination is valid, and this user is currently logged in
bool is_logged_in(int x){
	// Mutex lock
	active_users_l.lock();
	if(active_users.find(x) != active_users.end()){
    	active_users_l.unlock();
    	return true;
    }
    active_users_l.unlock();
    return false;
}

// String representation of all users currently online
string online_users(){
	string ret_val = "";
	// Mutex lock
	active_users_l.lock();
	for (set<int>::iterator it=active_users.begin(); it!=active_users.end(); ++it){
		ret_val += id_name[*it] + "\n";
	}
	active_users_l.unlock();
	return ret_val.substr(0, ret_val.size()-1);
}

// Client ended connection; remove everything associated with them
void remove_user(int c){
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
	// Close this connection
	close(c);
}

// A thread spawned per connection, to handle all incoming requests from there
void* per_user(void* void_connfd){
    string current_username;
	long connfd = (long)void_connfd;
	int ohho = 0, logged_in = 0;
	char buffer[BUFFER_SIZE];
    while(1){
    	memset(buffer,'0',sizeof(buffer));
		ohho = read(connfd,buffer,sizeof(buffer));
		if(!ohho){
			remove_user(connfd); // Remove active-user
			return 0; //End thread
		}
		buffer[ohho] = 0;
		cout<<"LOG : "<<buffer<<endl;
		// Extract command type from incoming data
		char *pch = strtok(buffer," ");
		string command(pch);
		logged_in = is_logged_in(connfd);
		if(!command.compare("/login")){
			try{
				pch = strtok (NULL, " ");
				string username(pch);
				pch = strtok (NULL, " ");
				string password(pch);
				logged_in = valid_login(username, password);
				if(logged_in){
					send_data("Signed in!", connfd);
					// Mutex lock
					name_id_l.lock();
					id_name_l.lock();
					active_users_l.lock(); 
                    // Update 1-1(effective) mapping of connectionID and username
                    current_username = username;
					name_id[username] = connfd;
					id_name[connfd] = username;
					active_users.insert(connfd); // Update list of active users
					name_id_l.unlock();
					id_name_l.unlock();
					active_users_l.unlock(); 
				}
				else{
					send_data("Error signing in!", connfd);
				}
			}
			catch(...){
				send_data("Malformed message!", connfd);
			}
		}
		else if(!command.compare("/who") && logged_in){
        	send_data(online_users().c_str(), connfd);
     	}
     	else if(!command.compare("/exit") && logged_in){
			remove_user(connfd); // Remove active-user
			return 0; //End thread;
     	}
     	else if(!command.compare("/msg") && logged_in){
     		try{
     			pch = strtok (NULL, " ");
				string to(pch);
				pch = strtok (NULL, " ");
				string data(pch);
				// Mutex lock
				chat_l.lock();
                name_id_l.lock();
				chat.push(make_pair(name_id[to], data)); // Push outgoing message to queue
                name_id_l.unlock();
                chat_l.unlock();
			}
			catch(...){
				send_data("Malformed message!", connfd);
			}
     	}
    	else if(!command.compare("/create_grp") && logged_in){
    		try{
    			pch = strtok (NULL, " ");
    			string g_name(pch);
    			// Mutex lock
    			groups_l.lock();
                // Check for duplication of group name
    			if(groups.find(g_name) == groups.end()){
	    			groups[g_name] = set<string>();
    				groups[g_name].insert(current_username);
    				send_data("Group created!", connfd);
    			}
    			else{
	    			send_data("Group already exists!", connfd);
    			}
    			groups_l.unlock();
    		}
    		catch(...){
				send_data("Malformed message!", connfd);
    		}
    	}
    	else if(!command.compare("/join_grp") && logged_in){
    		try{
    			pch = strtok (NULL, " ");
    			string g_name(pch);
    			// Mutex lock
    			groups_l.lock();
                // Check whether group name exists
    			if(groups.find(g_name) == groups.end()){
	    			send_data("Group doesn't exist!", connfd);
    			}
    			else{
	    			send_data("Joined group!", connfd);
    				groups[g_name].insert(current_username);
    			}
    			groups_l.unlock();
    		}
    		catch(...){
				send_data("Malformed message!", connfd);
    		}
	    }	
    	else if(!command.compare("/msg_group") && logged_in){
	    	try{
	    		pch = strtok (NULL, " ");
    			string g_name(pch);
    			pch = strtok (NULL, " ");
    			string message(pch);
    			// Mutex lock
    			groups_l.lock();
    			if(groups.find(g_name) == groups.end()){
    				groups_l.unlock();
    				send_data("Group doesn't exist!", connfd);
    			}
    			// A sanity check; just in case client modifies their code before running it
    			else if(groups[g_name].find(current_username) == groups[g_name].end()){
    				groups_l.unlock();
    				send_data("You're not part of this group!", connfd);	
    			}
    			else{
    				set<string> tempo = groups[g_name];
    				groups_l.unlock();
                    // Mutex lock
    				chat_grp_l.lock();
                    name_id_l.lock();
    				for(set<string>::iterator it = tempo.begin(); it != tempo.end(); ++it){
    					// Message sent by user shouldn't coma back to them
    					if(name_id[*it] != connfd){
    						chat_grp.push(make_pair(name_id[*it],make_pair(g_name, message)));	
    					}
    				}
                    name_id_l.unlock();
    				chat_grp_l.unlock();
    			}
    		}
    		catch(...){
				send_data("Malformed message!", connfd);
    		}
    	}   
	    else if(!command.compare("/send") && logged_in){ 
	    	bool keep_file = true;  
            pch = strtok(NULL, " ");
            string to_name(pch);
            // Mutex lock
            file_counter_l.lock();
            username_password_l.lock();
            int temp = file_counter++;
			file_counter_l.unlock();
            // Check whether user with this name exists
            if(username_password.find(to_name) == username_password.end()){
                keep_file = false;
            }
			username_password_l.unlock();
			// Receive and store file into server storage, ready for retrieval by targeted user
			string file_counter_string("temp_data/" + to_string(temp));
			FILE* fp = fopen(file_counter_string.c_str(),"w");
			memset(buffer,'0',sizeof(buffer));
			while((ohho = read(connfd,buffer,sizeof(buffer))) > 0){
				buffer[ohho] = 0;
				fwrite(buffer , 1 , ohho ,fp);
				fflush(fp);
				// Buffer ended; stop receiving
				if(ohho < BUFFER_SIZE){
					break;
				}
				memset(buffer,'0',sizeof(buffer));
			}
			// Mutex lock
			waiting_files_l.lock();
			waiting_files.insert(make_pair(to_name, file_counter_string));
			waiting_files_l.unlock();
			fclose(fp);
			// Delete file if it was meant for a user who's not registered yet
            if(!keep_file){
            	send_data("User with this username doesn't exist!", connfd);
            	remove(file_counter_string.c_str());
            }
            else{
				send_data("File received!", connfd);
            }
    	}
    	else if(!command.compare("/recv") && logged_in){
            // Mutex lock
            waiting_files_l.lock();
            multimap<string,string>::iterator it = waiting_files.find(current_username);
            // Check if user has any pending files ready for them
            if(it != waiting_files.end()){
            	waiting_files.erase(it);
                waiting_files_l.unlock();
                const char* fname = ((*it).second).c_str();
                FILE* fp = fopen(fname,"r");
                send_data(command.c_str(), connfd); // Comunicate start of file transfer
                int wth;
                while((wth = sendfile(connfd,fileno(fp),NULL,BUFFER_SIZE)) == BUFFER_SIZE); //Send file
                remove(fname); // Remove file from server storage
            }
            else{
                waiting_files_l.unlock();
                const char* commy = "No files for you!";
                send_data(commy, connfd);
            }
	    }
	}
}

// Empties the send-queue by sending messages to respective clients
void* send_back(void* argv){
	pair<int, string> x;
	while(true){
		while(chat.size()){
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
void* send_back_grp(void* argv){
	pair<int, pair<string,string> > x;
	while(true){
		while(chat_grp.size()){
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

int main(){
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
    while(1){
        connfd = accept(listenfd, (struct sockaddr*)NULL, NULL);
        // New thread per user (for communication)
    	pthread_t pot_temp;
        pthread_create(&pot_temp, NULL, per_user, (void*)connfd);
    } 
    return 0;
}
