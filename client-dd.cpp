#include <iostream>
//#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
using namespace std;

int main() {

    int client;
    client = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(9002);
    server_addr.sin_addr.s_addr = inet_addr("192.168.0.100");//INADDR_ANY;

    int connection_stat = connect(client, (struct sockaddr *) &server_addr, sizeof(server_addr));
    if (connection_stat == -1) {
        cout << "Connection Error!" << endl;
        exit(1);
    }


    char server_msg[1024];
    recv(client, &server_msg, sizeof(server_msg), 0);

    //printf("The server sent data: %s\n", server_msg);
    cout << "Connection! Server message: " << endl;
    cout << server_msg << endl;
    close(client);
    return 0;
}
