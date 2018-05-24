/*  Compile: g++ -std=c++11 test.cpp -o test.exe
 *  Need g++ -std=c++11 flag to compile to_string() command.
 *
 *  Description: Client application that attains some message payload and uses server
 *  header information (addresses, port number, etc) to create a complete HTTP POST request.
 *  The client then creates a client socket, and establishes a secure TCP connection with
 *  the remote server, and sends the created HTTP request. The server is designed to accept
 *  the POST request, and compare the message that it currently holds, with the message from
 *  the POST payload. If they are identical, the server responds back to the client with a
 *  HTTP OK, otherwise a HTTP error with some error code. This code will be used to collect
 *  the message that was been snooped and deciphered (from the snooped server, port 8117),
 *  and sent for authentication to the server (same server but on port 8118, which runs the
 *  http application).
 */

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <string>
#include <vector>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

using namespace std;

string postadickpic(char *url, string payload);
class ServerInfo
{
    private:
    int socket_fd;
    string server_ip;
    short server_port;

    public:
    ServerInfo(char *serv_ip, char *serv_port, int socket) 
    {
        server_ip = (string)serv_ip;
        string tmp = (string)serv_port;
        server_port = stoi(tmp);
        socket_fd = socket;
    }

    struct sockaddr_in get_sockstruct()
    {
        struct sockaddr_in tmp;
        memset(&tmp, 0, sizeof(tmp));
        tmp.sin_family = AF_INET;
        tmp.sin_port = htons(server_port);
        tmp.sin_addr.s_addr = inet_addr(server_ip.c_str());
        
        return tmp;
    }

    int get_socket()
    {
        return socket_fd;
    }

    string get_ip()
    {
        return server_ip;
    }

    short get_port()
    {
        return server_port;
    }
};

void post_answer(char * answer, ServerInfo*);


int main (int argc, char *argv[]) {
    int httpC;
    char* portNo = strdup("8118");
    char* ip = strdup("149.171.36.173");
    if (argc != 2) {
        cout << "Erorrrr not enough args" << endl;
    }
    printf("[*] Creating a socket for server comm...\n");
    if ((httpC = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("    => Couldnt create socket for HTTP client\n");
        exit(1);
    }
    /*
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(portNo);
    printf("[*] Validating IP Address...\n");
    if (inet_pton(AF_INET, "149.171.36.173", &serv_addr.sin_addr) <= 0) {
        printf("Invalid Address\n");
        exit(1);
    }*/
    ServerInfo http_server = ServerInfo(ip, portNo, httpC);
    struct sockaddr_in http_addr = http_server.get_sockstruct();    
    printf("[*] Attempting connection with the http server...\n");
    if (connect(httpC, (struct sockaddr *)&http_addr, sizeof(http_addr)) < 0) {
        printf("    => Error in connection attempt\n");
        exit(1);
    }

    int readSockets_fd[3];
    int slave_fd[3];
    struct sockaddr_in master_addr[3];
    char *port_i[3];
    char *my_ip = strdup("192.168.43.49");
    port_i[0] = strdup("1500");
    port_i[1]= strdup("1501");
    port_i[2] = strdup("1502");
    printf("[*] Creating read sockets\n");
    for (int i = 0; i < 1; i++) {
        if ((readSockets_fd[i] = socket(AF_INET, SOCK_STREAM, 0))<0) {
            printf("    => Couldnt create socket for slaves\n");
            exit(1);
        } 
        if (i == 0) {
            ServerInfo master_info0 = ServerInfo(my_ip, port_i[0], readSockets_fd[0]);
            master_addr[i] = master_info0.get_sockstruct();
        } else if (i == 1) {
            ServerInfo master_info1 = ServerInfo(my_ip, port_i[1], readSockets_fd[1]);
            master_addr[i] = master_info1.get_sockstruct();
        } else if (i == 2) {
            ServerInfo master_info2 = ServerInfo(my_ip, port_i[2], readSockets_fd[2]);
            master_addr[i] = master_info2.get_sockstruct();
        } 

        socklen_t size = sizeof(master_addr[i]);
        if ((bind(readSockets_fd[i], (struct sockaddr*)&master_addr[i], sizeof(master_addr[i])))<0) {
            cout << "   => Error binding connection... " << i << endl;
            exit(1);
        }   
        listen(readSockets_fd[i], 1);
        slave_fd[i] = accept(readSockets_fd[i], (struct sockaddr*)&master_addr[i], &size);
        if (slave_fd[i] < 0) {
            cout << "   => Error accepting incoming client" << endl;
            exit(1);
        }
    }
    printf("[*] Read sockets created!\n");

    char buffer[30];
    while (1) {
        recv(slave_fd[0], buffer, sizeof(buffer),0);
        cout << buffer << endl;
    }
    post_answer(argv[1], &http_server);

    close(httpC);

    return 0;
}

string postadickpic(char *url, string payload) {
    string request;
    request = "POST /path/script HTTP/1.1\r\n";
    request = request + "Host: " + url + "\r\n";
    //request = request + "Content-Type: <Some browser type>\r\n";
    request = request + "Content-length: " + to_string(payload.length()) + "\r\n";
    request = request + "\r\n";
    request = request + payload;
    return request;
}



    /*  Create a container that will be used to store the local payload.
     *  This is where the snooped and reconstructed/deciphered message
     *  should be stored.
     *  STRING PAYLOAD. CHANGE THIS TO CONSTRUCTED MESSAGE. 
     */ 
void post_answer(char *answer, ServerInfo* server) {

    int httpC = server->get_socket();
    char response[900];

    char* host = strdup("149.171.36.173");

    printf("[*] Storing payload\n");
    string container(answer); 
    cout << "   -> " << container << endl;
    /*  Inserts all the relevent HTTP header information + payload, to
     *  construct a HTTP POST request. This request will be created and returned
     *  into 'buffer', where it will be sent to the server via the established
     *  connection previously created.
     */
    printf("[*] Creating HTTP POST request\n");
    string buffer = postadickpic(host, container);
    printf("[*] HTTP POST request raw format:\n");
    cout << "==================================" << endl;
    cout << buffer << endl;
    cout << "==================================" << endl;
    cout << endl;
    /*  Once the POST request has been established, send this through the TCP
     *  connection that was made to the server.
     *  NOTE2SELF -> Should we be sending a POST request for every message?
     *  or for every sentence that ends with an EOF? Food4thought.
     */
    printf("[*] Sending HTTP POST request now...\n");
    send(httpC, buffer.c_str(), buffer.length(), 0);
    printf("[*] Awaiting reply...\n");

    /*  Once the POST request has been sent off to the server, the server will
     *  perform some computation to ensure whether the message sent to the server
     *  matches with what the server has. If the server is unhappy with the message,
     *  i.e. does not match, it will send a HTTP error message otherwise it will
     *  send an OK message.
     *  HTTP 404 -> Error
     *  HTTP 200 -> OK
     */
    recv(httpC, response, sizeof(response), 0);
    printf("[*] HTTP POST response:\n");
    cout << "==================================" << endl;
    cout << response << endl;
    cout << "==================================" << endl;
    cout << endl;

    printf("[*] Sending new line to complete Authorization...\n");
    buffer = postadickpic(host, "\n");
    send(httpC, buffer.c_str(), buffer.length(), 0);
    recv(httpC, response, sizeof(response), 0);
    printf("[*] HTTP POST response to new line:\n");
    cout << "==================================" << endl;
    cout << response << endl;
    cout << "==================================" << endl;
    cout << endl;

    printf("[*] Authorization complete. Lets go home\n");

} 
