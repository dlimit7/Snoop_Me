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

int main (int argc, char *argv[]) {
    int httpC;
    int portNo = 8118;
    char host[1024] = "skunksmail.ee.unsw.edu.au";
    string msg_fmt = "POST /apikey=%scommand=%s HTTP/1.0\r\n\r\n";

    struct hostent *httpS;
    struct sockaddr_in serv_addr;

    string buffer;
    char response[9000];

    /*  Creates http client socket.
     *  Checks whether socket creation was successful, otherwise it throws an error.
     */
    printf("[*] Creating a socket...\n");
    if ((httpC = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("    => Couldnt create socket for HTTP client\n");
        exit(1);
    }

    /*  Checks the given URL via the gethostbyname() command.
     *  returns a NULL if hostname is invalid
     */
//    httpS = gethostbyname(host);
//    if (httpS == NULL) {
//        printf("    => No such host biatch\n");
//        exit(1);
//    }
    /*  memset fills the block of memory pointed @ &serv_addr
     *  size of memory block is sizeof(serv_addr);
     *  fills block of memory to 0 value
     *  Populate server address struct
     */
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(portNo);
//    memcpy(&serv_addr.sin_addr.s_addr, httpS->h_addr, httpS->h_length);

    /*  Validates whether the given IP address is existing or not. If the IP address
     *  can not be located, the funcion spits out an error and decalres the address
     *  to be invalid.
     */
    //149.171.36.173
    printf("[*] Validating IP Address...\n");
    if (inet_pton(AF_INET, "149.171.36.173", &serv_addr.sin_addr) <= 0) {
        printf("Invalid Address\n");
        exit(1);
    }
    //serv_addr.sin_addr.s_addr = INADDR_ANY;

    /*  Attempt to connect the http client socket to the http server socket.
     *  This is performed through the use of the connect() feature which returns
     *  a negative value if the connection faced an error.
     */
    printf("[*] Attempting connection with server...\n");
    if (connect(httpC, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("    => Error in connection attempt\n");
        exit(1);
    }

    /*  Create a container that will be used to store the local payload.
     *  This is where the snooped and reconstructed/deciphered message
     *  should be stored.
     */
    printf("[*] Storing payload\n");
    string container(argv[1]);
    //container = "davis";
    //container = "\n";
    cout << "   -> " << container << endl;
    /*  Inserts all the relevent HTTP header information + payload, to
     *  construct a HTTP POST request. This request will be created and returned
     *  into 'buffer', where it will be sent to the server via the established
     *  connection previously created.
     */
    printf("[*] Creating HTTP POST request\n");
    buffer = postadickpic(host, container);
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
