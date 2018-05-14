#include <iostream>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>

using namespace std;

int main () {
    int client;
    int port = 1500;
    bool isExit = false;
    int buffersize = 1024;
    char buffer[buffersize];
    char *ip = "192.168.0.100";

    struct sockaddr_in server_addr;
    client = socket(AF_INET, SOCK_STREAM, 0);

    if (client < 0) {
        cout << "   =>Error creating socket..." << endl;
        exit(1);
    }
    cout << "=> Client socket has been created" << endl;

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (connect(client, (struct sockaddr *) &server_addr, sizeof(server_addr)) == -1) {
        cout << "=> Connection Error to port number: " << port << endl;
        exit(1);
    }

    cout << "=> Awaiting for server to accept connection request..." << endl;
    recv(client, &buffer, buffersize, 0);
    cout << "=> Connection confirmed" << endl;
    cout << "Enter '#' to end the connection\n" << endl;

    do {
        cout << "Client: ";
        do {
            cin >> buffer;
            send(client, &buffer, buffersize, 0);
            if (*buffer == '#') {
                send(client, buffer, buffersize, 0);
                *buffer = '*';
                isExit = true;
            }
        } while (*buffer != '*');

        cout << "Server: ";
        do {
            recv(client, &buffer, buffersize, 0);
            cout << buffer << " ";
            if (*buffer == '#') {
                *buffer = '*';
                isExit = true;
            }
        } while (*buffer != '*');

        cout << endl;

    } while (!isExit);

    cout << "\n=> Connection terminated" << endl;
    cout << "\n=> Goodbye!" <<endl;

    return 0;
}
