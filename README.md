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
* `/msg_group <MESSAGE>` : broadcast a message to an entire group which you've currently joined
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
* To run the server, run:  ` make`. Then, run it as `./server` (from server folder).
* To run a client, run:  ` make`. Then, run it as `./client <SERVER ADDRESS>` (from client folder)
* If you don't  mind concurrency bugs, run:  ` make`. Then, run it as `./server` (from server(no_locks) folder).
* Run `make clean` to remove compiled programs.


## Specifics
* Default user (hello,world) for testing.
* Mutex locks are inserted around every shared variable to ensure concurrency.
* Maximum length per message: 512 characters.
* Creating group (on the client's end) is not enough to send messages to that group; joining is required as well.
* `/recv` will create a file by the name of 'file_from_server'. To receive more files (and avoid them overwriting this file), move the file to some other location after receiving it.
* Files pushed to ther server are stored in the folder 'temp_data'.
* strtok() is not thread safe (as standard implementation doesn't use TLS). Thus, strtok_r() has been used.
* 'User is offline/doesn't exist' is flashed to the client if they try messaging a user who is offline or doesn't exist (further specification isn't provided to avoid any secutiry attacks).


## Cases tested (& handled)
* Register via client.
* Register and then logs in.
* Already registered user logs in.
* Unregistered user tries to log in.
* Logged in user tries to log in again.
* Joining a non-existent group.
* Messaging a group they haven't joined/not part of.
* Sending a message to group after joining it.
* Sending message to user who doesn't exist/isn't online.
* Sending file to user who's online, sending file to yourself.
* Sending file to user who doesn't exist/isn't online.
* Sending file, going offline, and then the reciepent downloads it.
* Server shuts down in between.
* Client shuts down their service unexpectedly.
* Checking who's online.
* Messages sent to a group do not come back to the sender (as it doesn't make sense).
* Any message received includes, in brackets, name of the user/group from which it is.
* User tries sending a file that doesn'r exist on their system.
* Messages from people have username in brackets, messages from groups have group's name in square brackets.
* Multiple files waiting for a person to receive (multiset is used in backend, so a person can have multiple pending files).


### Example (for client)
```
>> Welcome to IRsea!
/register test user
>> Registered!
/login hello world
>> Signed in!
/who
>> hello
random_user
/msg random_user what's up?
>> (random_user)  nothing much
/create_grp random
>> Group created!
/join_grp random
>> Joined group!
>> [random_user] hi, random user :)
/send random_user Makefile                    
>> File sent to server!
/recv
>> File received!
random_command
>> Invalid command! Please read the README for the list of supported commands
/exit
>> Exiting!
Thanks for using IRsea!

```