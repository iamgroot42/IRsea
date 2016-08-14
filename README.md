# IRsea

## About

A chatting application, designed along the lines of IRC. Made as a course assignment for Network Security (CSE550).
The following commands are supported :

* `/register <USERNAME> <PASSWORD>` : register yourself with the server
* `/login <USERNAME> <PASSWORD>` : login to access the commands below
* `/exit` : log out and exit from the service
* `/who` : see all the users who're logged in right now
* `/msg <USERNAME> <MESSAGE>` : send a message to a specific user
* `/create_grp <GROUP_NAME>` : create a group
* `/join_grp <GROUP_NAME>` : join an existing group
* `/msg_group <GROUP_NAME> <MESSAGE>` : broadcast a message to an entire group which you've currently joined
* `/send <USERNAME> <FILENAME>` : send a file to another user
* `/recv` : receive file being sent by someone

## Working

### Server

* Four threads work in parallel.
* One thread listens on the port assigned for registration of users.
* One thread clears the outgoing message queue by sending data to respective group users.
* One thread clears the outgoing message queue by sending data to respective users.
* One thread creates a new thread (for communication) for every incoming request by the client. This thread is deleted once the client is done with their interaction with the server.


### Client
* Two threads work in parallel.
* One thread listens to the server for incoming data (which may be sent by the server itself, or data redirected by the server).
* One thread is for HCI; sending data to the server as the user requests.


## Running it
* To run the server, compile it as:  ` g++ -pthread -std=c++11 server.cpp -o server`. Then, run it as `./server`
* To run a client, compile it as:  ` g++ -pthread client.cpp -o client`. Then, run it as `./client <SERVER ADDRESS>`

## Specifics
* Mutex locks are inserted around every shared variable to ensure concurrency.
* Maximum length per message: 512 characters.
* Creating group (on the client's end) is not enough to send messages to that group; joining is required as well.
* `/recv` will create a file by the name of 'file_from_server'. To receive more files (and avoid them overwriting this file), move the file to some other location after receiving it.
* Files pushed to ther server are stored in the folder 'temp_data'.