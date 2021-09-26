'make all' will compile the c files and place their objects 'udpclient' and 'udpserver' in their respective dir

'make server.o'  compiles and places server object
'make client.o' compiles and places client object

'make clean' removes all created .o files

running programs is as declared before
./udpclient <hostname> <port no>
./udpserver <port no>

Program runs in a loop taking options 1-6
1: message
2: GET
3: PUT
4: DELETE
5: ls
6: EXIT


I was unable to get network communications to work across machines, fortunately  it does work across the cselra clusters and my device, so hopefully it does the same on yours. 
