#include <iostream>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>

using namespace std;

int main () {
    int client;
    int port = 1500;
    bool isExit = false;
    int buffersize = 1024;
    char buffer[buffersize];
    //char *ip = "192.168.0.100";

    /*  Creating client socket.
     *  Socket is running over IP (IPv4 default) -> AF_INET
     *  Socket performs secure and reliable transmission streams -> SOCK_STREAM
     *  Socket runs the TCP protocol, deafult protocol for SOCK_STREAM -> 0
     */
    cout << "=> Attempting to create Client Socket" << endl;
    client = socket(AF_INET, SOCK_STREAM, 0);
    if (client < 0) {
        cout << "   =>Error creating socket..." << endl;
        exit(1);
    }
    cout << "=> Client socket created! Now ready for connection" << endl;

    /*  Assigning server details into sockaddr struct. These details are used
     *  when the client is figuring where to connect to. In this example,
     *  the destination server is of the IPv4 family (AF_INET),
     *  IP address = INADDR_ANY;
     *  Port = port
     *  Note that htons() treats passed variables as Little Endian
     *  and converts them into uint16 Big Endian
     */
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    /*  Attempts to connect Client to Server using the details defined in the struct.
     *  Return -1 for any connection failures. This inidicates that either the
     *  server details are wrong, the server with these details does exist or the
     *  server (or something in between the connection) is blocking the communication.
     */
    cout << "=> Awaiting for server to accept connection request..." << endl;
    if (connect(client, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        cout << "   => Connection Error to port number: " << port <<endl;
        exit(1);
    }

    /*  In this setup, the server sents a message first to "confirm" the connection.
     *  Think of it as an acknowledgement that the connection is ready to go.
     */
    recv(client, buffer, buffersize, 0);
    cout << buffer << endl;
    cout << "Enter '#' to end the connection\n" << endl;

    do {
        /*  Client loops through user input from cin strea, storing each
         *  word seperated by whitespaced into the buffer. The buffer
         *  then one by one gets sent off to the server, where it will
         *  append these words into its cout stream. Once the stream is
         *  flushed, the full string will be outputted to user terminal.
         */
        cout << "Client: " << flush;
        do {
            cin >> buffer;
            send(client, buffer, buffersize, 0);
            if (*buffer == '#') {
                send(client, buffer, buffersize, 0);
                *buffer = '*';
                isExit = true;
            }
        } while (*buffer != '*');

        /*  If client sent a '#' as one of the buffered words,
         *  then this would trigger a change of the isExit state.
         *  If so, it inidicates that the client wants to break
         *  communication.
         */
        if (isExit) {
            break;
        }

        /*  Client now listens for a response from the server. This
         *  is performed by the recv() function. It will listen for
         *  each word within each received buffer and store it into
         *  cout stream. Once the end of the string sent by the server
         *  is seen, indicated by the '*' then client will flush out
         *  cout to user terminal and stop listening.
         */
        cout << "Server(msg): " << flush;
        do {
            recv(client, buffer, buffersize, 0);
            cout << buffer << " ";
            if (*buffer == '#') {
                *buffer = '*';
                isExit = true;
            }
        } while (*buffer != '*');
        cout << endl;
    } while (!isExit);

    /*  Connection has been closed. Good practice to close the client socket.
     */
    cout << "\n=> Connection terminated" << endl;
    close(client);
    cout << "Goodbye!" <<endl;

    return 0;
}
