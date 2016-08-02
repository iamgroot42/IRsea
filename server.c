#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h> 
#include <pthread.h>

#define PORT 5555

typedef struct node_
{
    int id;
    struct node_ *next;
} node;

node *front,*back;

int isEmpty()
{
    if(front==NULL && back==NULL) return 1;
    return 0;
}

void insert(int s)
{
    if(front==NULL && back==NULL)
    {
        front = (node*)malloc(sizeof(node));
        front->id = s;
        front->next = NULL;
        back = front;
    }
    else
    {
        node* temp;
        temp = (node*)malloc(sizeof(node));
        temp->id = s;
        back->next = temp;
        back = temp;
    }
}

int remove_()
{
    if(front==NULL) return 0;
    int retval;
    if(front == back)
    {
        retval = front->id;
        free(front);
        front = NULL;
        back = NULL;
    }
    else
    {
        node* temp = front->next;
        retval = front->id;
        free(front);
        front = temp;
    }
    return retval;
}

void *checker(void* argv)
{
    int lolk;
    char sendBuff[256];
    while(1)
    {
        if(!isEmpty())
        {
            char reply[256];
            memset(sendBuff, '0', sizeof(sendBuff)); 
            lolk = remove_();
            fgets(reply, sizeof(reply), stdin);
            // scanf("%s",reply);
            snprintf(sendBuff, sizeof(sendBuff), "%s\n", reply);
            write(lolk, sendBuff, strlen(sendBuff)); 
            // close(lolk);
        }
    }
}

int main(int argc, char *argv[])
{
    int listenfd = 0, connfd = 0,ohho;
    struct sockaddr_in serv_addr; 
    char buffer[256];

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&serv_addr, '0', sizeof(serv_addr)); 

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(PORT); 

    bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)); 

    listen(listenfd, 5); 
    printf("Chat application online!\n");
    printf("Type in the terminal and press enter to reply to messages pending in the queue\n");

    pthread_t pot;
    pthread_create(&pot, NULL, checker, NULL);

    while(1)
    {
        connfd = accept(listenfd, (struct sockaddr*)NULL, NULL); 
        if(connfd >= 0)
        {
            memset(buffer,'0',sizeof(buffer));
            ohho = read(connfd,buffer,sizeof(buffer));
            buffer[ohho] = 0;
            // printf("Length : %d\n",strlen(buffer));
            // printf("Size : %d\n",sizeof(buffer));
            printf("Query received : %s\n",buffer);
            insert(connfd);
        }
    }
}
