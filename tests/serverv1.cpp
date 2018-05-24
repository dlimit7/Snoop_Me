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

    /*  Creating server socket.
     *  Socket is running over IP (IPv4 default) -> AF_INET
     *  Socket performs secure and reliable tranmission streams -> SOCK_STREAM
     *  Socket runs the TCP protocol, default protocol for SOCK_STREAM -> 0
     */
    cout << "=> Attempting on creating Server Socket" << endl;
    server = socket(AF_INET, SOCK_STREAM, 0);
    if (server < 0) {
        cout << "   => Error creating socket..." << endl;
        exit(1);
    }
    cout << "=> Server socket created! Now ready for connection" << endl;

    /*  Setting up server_addr struct to be complient with IPv4 family (AF_INET),
     *  IP address = INADDR_ANY;
     *  Port = port
     *  Note that htons() treats passed variables as Little Endian
     *  and coverts them into uint16 Big Endian
     *  bind() function binds the struct to the server socket
     */
    struct sockaddr_in server_addr;
    socklen_t size;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("10.248.102.153");//INADDR_ANY;
    server_addr.sin_port = htons(port);
    if ((bind(server, (struct sockaddr*) &server_addr, sizeof(server_addr))) < 0) {
        cout << "   => Error binding connection..." << endl;
        exit(1);
    }

    /*  Set server socket to being listening for any client socket communication.
     *  listen() marks server socket as a passive socket, a socket that can be used
     *  to accept incoming connection requests ussing the accept().
     *  Queue limit for incoming connections is set to 1.
     *  Once the connection has been accpeted by the accept(), communication will
     *  be started using the client details.
     */
    cout << "=> Listening for clients..." << endl;
    listen(server, 1);
    size = sizeof(server_addr);
    client = accept(server, (struct sockaddr*) &server_addr, &size);
    if (client < 0) {
        cout << "=> Error accepting incoming client" << endl;
        exit(1);
    }

    /*  Create initial content and copy into buffer.
     *  Buffer used as a container to hold data to be sent/received.
     *  Server sends data right after accepting client connection.
     */
    strcpy(buffer, "=> Server acknowledges connection. Connection confirmed");
    send(client, buffer, buffersize, 0);
    cout << "=> Connected with client" << endl;
    cout << "Enter '#' to end the connection\n" << endl;

    do {
        /*  Listens for client messages. This is performed from the
         *  recv() function. It will listen for each word and cout
         *  will buffer them into a stream. The string of words is
         *  then flushed out and outputted onto cli via '<< endl'.
         *  The '*' indicates the end of the string.
         */
        cout << "Client(msg): " << flush;
        do {
            recv(client, buffer, buffersize, 0);
            cout << buffer << " ";
            if (*buffer == '#') {
                *buffer = '*';
                isExit = true;
            }
        } while (*buffer != '*');
        cout << endl;

        /*  If server read a '#' as one of the received words,
         *  then this would trigger a change of the isExit state.
         *  If so, it indicates that the client wants to break
         *  communication.
         */
        if (isExit) {
            break;
        }

        /*  Sever now loops through user input via cin, storing each
         *  word seperated by whitespaces into the buffer. The buffer
         *  then one by one gets sent off to the client, where it will
         *  append the word onto its stream, and output to user all at
         *  once through flushing. End of string terminated by the '*'.
         */
        cout << "Server: " << flush;
        do {
            cin >> buffer;
            send(client, buffer, buffersize, 0);
            if (*buffer == '#') {
                send(client, buffer, buffersize, 0);
                *buffer = '*';
                isExit = true;
            }
        } while (*buffer != '*');
    } while (!isExit);

    /*  Connection has been closed. Good practice to close the server socket.
     */
    cout << "\n=> Connection terminated with IP " << inet_ntoa(server_addr.sin_addr);
    close(server);
    cout << "\nGoodbye!" << endl;

    return 0;
}
