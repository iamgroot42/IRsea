#include <bits/stdc++.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h> 

#define REGISTER_PORT 5004
#define IRC_PORT 5005

// 3 queues for : p2p messages, groups, file sharing
// 2 threads per queue (one for clearing queue, one for populating it)
// Mapping between usernames and their file-descriptors
// Note: queue for group would be of a vector, not int

using namespace std;

map<string,int> name_id;
map<string, string> username_password; //Mapping of username-password (chached in memory); present in file
queue< pair<int, string> > chat;


void register_user()
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
            buffer[ohho] = 0; //End string
            // Split buffer into username and password
            pch = strtok(buffer," ");
            string username(pch);
            pch = strtok (NULL, " ");
            string password(pch);
            // Store in cache
            username_password.push(make_pair(username,password));
        }
    } 
}

void proc_chat_send()
{
    int file_dee;
    char sendBuff[256];
    // Limit on length of a message in one go : 256
    while(true)
    {
        if(!chat.empty())
        {
            char reply[256];
            memset(sendBuff, '0', sizeof(sendBuff)); 
            file_dee = chat.front().first;
            reply = chat.front().second;
            chat.pop();
            fgets(reply, sizeof(reply), stdin);
            // getline(cin, reply); // Replaces above
            snprintf(sendBuff, sizeof(sendBuff), "%s\n", reply);
            write(file_dee, sendBuff, strlen(sendBuff));
        }
    }
}


void proc_chat_receive(int connfd)
{
    // Limit on length of a message in one go : 256
    char buffer[256];
    int ohho;
    memset(buffer, '0', sizeof(buffer));
    ohho = read(connfd, buffer, sizeof(buffer));
    buffer[ohho] = 0;
    chat.push(make_pair(connfd,buffer));
}


int main()
{
    // Create threads for : user registration, receive messages, send messages
    // pthread_t pot;
    // pthread_create(&pot, NULL, checker, NULL);
    return 0;
}
