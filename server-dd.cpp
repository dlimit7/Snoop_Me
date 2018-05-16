//#include <iostream>
//#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>      //define socket functions
#include <sys/socket.h>     //define socket functions. Includes API to create socket.
#include <netinet/in.h>     //structures used to store address information
#include <arpa/inet.h>
#include <unistd.h>

using namespace std;

int main() {
    //bool isExit = false;
    //int buffsize = 1024;
    //char buffer[buffsize];

    //socklen_t size;

    //Initializing the Server Socket
    //Uses client descriptor to call socket function. Socket will run over an internet
    //domain, "AF_INET", with 'connection-oriented' type traffic, "SOCK_STREAM",
    //based off its default protocol of TCP, "0".
    int server;
    server = socket(AF_INET, SOCK_STREAM, 0);

    //Define Server Address
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;        //Decalres IP address family to AF_INET
    server_addr.sin_port = htons(9002);
    server_addr.sin_addr.s_addr = inet_addr("192.168.0.100");  //INADDR_ANY;   //Defines socket to any available IP address on local machine.

    //Binding socket to specified IP address/Port number.
    bind(server, (struct sockaddr*) &server_addr, sizeof(server_addr));
    listen(server, 5);      //Listens from server socket over 5 different possible connections.

    //Attain details of connecting client
    int client;             //Client descriptor
    client = accept(server, NULL, NULL);      //Saves client socket details that connect to our server socket.

    //Return message to client after establishing a connection
    int dataSize = 1024;
    char server_msg[] = "Connection established!";
    send(client, server_msg, sizeof(server_msg), 0);
    close(server);


    //if (client < 0) {
    //    cout << "Error in establishing a connection!" << endl;
    //   exit(1);
    //}
    //cout << "Connection established!" << endl;

    //server_addr.sin_family = AF_INET;
    //server.addr.sin_addr.saddr = htons(INADDER_ANY);
    //server.addr.sin_port = htons(portNum);

    return 0;
}
