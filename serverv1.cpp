#include <iostream>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>

using namespace std;

int main() {
    int client, server;
    int port = 1500;
    bool isExit = false;
    int buffersize = 1024;
    char buffer[buffersize];

    struct sockaddr_in server_addr;
    socklen_t size;

    /*
     *
     *
     */

    client = socket(AF_INET, SOCK_STREAM, 0);

    if (client < 0) {
        cout << "   => Error creating socket..." << endl;
        exit(1);
    }
    cout << "\nServer socket created! Now ready for connection" << endl;

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htons(INADDR_ANY);
    server_addr.sin_port = htons(port);

    if ((bind(client, (struct sockaddr*) &server_addr, sizeof(server_addr))) < 0) {
        cout << "   => Error binding connection..." << endl;
        return -1;
    }
    size = sizeof(server_addr);
    cout << "=> Listening for clients..." << endl;

    listen(client, 1);
    server = accept(client, (struct sockaddr*) &server_addr, &size);
    cout << server;
    if (server < 0) {
        cout << "=> Error accepting incoming client" << endl;
    }

    while (server > 0) {
        strcpy(buffer, "=> Server connected...");
        send(server, &buffer, buffersize, 0);
        cout << "=> Connected with client" << endl;

        cout << "Client: ";
        do {
            recv(server, &buffer, buffersize, 0);
            cout << buffer << " ";
            if (*buffer == '#') {
                *buffer = '*';
                isExit = true;
            }
        } while (*buffer != '*');

        cout << endl;

        do {
            cout << "Server: ";
            do {
                cin >> buffer;
                send(server, &buffer, buffersize, 0);
                if (*buffer == '#') {
                    send(server, &buffer, buffersize, 0);
                    *buffer = '*';
                    isExit = true;
                }
            } while (*buffer != '*');
            cout << "Client: ";
            do {
                recv(server, &buffer, buffersize, 0);
                cout << buffer << " ";
                if (*buffer == '#') {
                    *buffer = '*';
                    isExit = true;
                }
            } while (*buffer != '*');

            cout << endl;

        } while (!isExit);

        cout << "\n=> Connection terminated with IP " << inet_ntoa(server_addr.sin_addr);
        close(server);
        cout << "\nGoodbye!" << endl;
        isExit = false;
        exit(1);
    }
    close(client);

    return 0;
}
