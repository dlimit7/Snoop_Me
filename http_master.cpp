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
#include <map>
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
    ServerInfo(char *serv_ip, char *serv_port, int socket) {
        server_ip = (string)serv_ip;
        string tmp = (string)serv_port;
        server_port = stoi(tmp);
        socket_fd = socket;
    }
    struct sockaddr_in get_sockstruct() {
        struct sockaddr_in tmp;
        memset(&tmp, 0, sizeof(tmp));
        tmp.sin_family = AF_INET;
        tmp.sin_port = htons(server_port);
        tmp.sin_addr.s_addr = inet_addr(server_ip.c_str());
        
        return tmp;
    }
    int get_socket() {
        return socket_fd;
    }
    string get_ip() {
        return server_ip;
    }
    short get_port() {
        return server_port;
    }
};

void post_answer(char * answer, ServerInfo*);


int main (int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usages: ./master port# master_ip\n");
        exit(1);
    }
    int httpC;
    char* portNo = strdup(argv[1]);
    char* ip = strdup("149.171.36.173");
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

    int readSockets_fd[1];
    int slave_fd[3];
    struct sockaddr_in master_addr[1];
    char *port_i[1];
    char *my_ip = strdup(argv[2]);
    port_i[0] = strdup("1500");
    for (int i = 0; i < 1; i++) {
        printf("[*] Creating read socket %d\n", i);
        if ((readSockets_fd[i] = socket(AF_INET, SOCK_STREAM, 0))<0) {
            printf("    => Couldnt create socket for slaves\n");
            exit(1);
        } 
        ServerInfo master_info0 = ServerInfo(my_ip, port_i[0], readSockets_fd[0]);
        master_addr[i] = master_info0.get_sockstruct();
        cout << "binding" << endl;
        socklen_t size = sizeof(master_addr[i]);
        if ((bind(readSockets_fd[i], (struct sockaddr*)&master_addr[i], sizeof(master_addr[i])))<0) {
            cout << "   => Error binding connection... " << i << endl;
            exit(1);
        }   
        cout << "listening" << endl;
        listen(readSockets_fd[i], 1);
        slave_fd[i] = accept(readSockets_fd[i], (struct sockaddr*)&master_addr[i], &size);
        // the accepted slave_fd descripter had been set to non blocking. I think this property maintains.
        if (slave_fd[i] < 0) {
            cout << "   => Error accepting incoming client" << endl;
            exit(1);
        }
        slave_fd[1] = accept(readSockets_fd[i], (struct sockaddr*)&master_addr[i], &size);
        // the accepted slave_fd descripter had been set to non blocking. I think this property maintains.
        if (slave_fd[1] < 0) {
            cout << "   => Error accepting incoming client" << endl;
            exit(1);
        }
        slave_fd[2] = accept(readSockets_fd[i], (struct sockaddr*)&master_addr[i], &size);
        // the accepted slave_fd descripter had been set to non blocking. I think this property maintains.
        if (slave_fd[2] < 0) {
            cout << "   => Error accepting incoming client" << endl;
            exit(1);
        }
    }
    printf("[*] Read sockets created!\n");

    char buffer1[30];
    //char buffer2[30];
    char *answer = NULL;
    int * checklist = NULL;
    char msg[22];
    map<string, unsigned long long int>msgtoid;
    map<string, unsigned int>msg_map;
    unsigned int num_packets = 0;
    unsigned int diff_array[10];
    unsigned int diff;
    unsigned int i = 0;
    unsigned int num_packets_found = 0;
    char ** message= NULL;
    unsigned int offset = 0;
    unsigned int index;
    unsigned int count = 0;
    int offsetFound = -1;
    unsigned int k;
 

    while (1) {
        fd_set read_fds;
        FD_ZERO(&read_fds);
        FD_SET(slave_fd[0], &read_fds);
        FD_SET(slave_fd[1], &read_fds);
        FD_SET(slave_fd[2], &read_fds);
        int activity = select(slave_fd[2]+1, &read_fds, NULL, NULL, NULL);

        switch(activity) {
            case -1:
                cout << "Problem with select" << endl;
                exit(1);
            case 0:
                cout << "Problem with select" << endl;
                exit(1);
            default:
                memset(buffer1, 0, sizeof(buffer1));
                if (FD_ISSET(slave_fd[0], &read_fds)) {
                    recv(slave_fd[0], buffer1, sizeof(buffer1),0);
                } else if (FD_ISSET(slave_fd[1], &read_fds)) {
                    recv(slave_fd[1], buffer1, sizeof(buffer1),0);
                } else if (FD_ISSET(slave_fd[2], &read_fds)) {
                    recv(slave_fd[2], buffer1, sizeof(buffer1),0);
                }
                //printf("buffer content %s\n", buffer1);
                strcpy(msg,buffer1+8);
                char first_char = msg[0];
                cout << "message is \"" << msg << "\"" << endl; 
                unsigned long long int id = *(unsigned long long int *)buffer1;
                if (num_packets_found == 0) { 
                    if (msgtoid.count(msg) < 1) {
                        // If i havn't seen this message before, note it, increment num packs
                        msgtoid[msg] = id;
                        
                        num_packets++;
                    }   
                    if (first_char == 0x4) {
                        //printf("id 0x%llx - msgtoid[msg] 0x%llx\n\n\n\n\n", id, msgtoid[msg]);
                        if ((diff = (unsigned int)(id - msgtoid[msg])) > 0) {
                            if (diff == 0) exit(1);
                            //while(1);
                            // update id and record diff
                            msgtoid[msg] = id;
                            diff_array[i++] = diff;
                        }
                    }
                    if (i == 3) {
                        printf("num_packs = %d\n", num_packets);
                        // find lowest common divisor in diff_array
                        unsigned int min = 0xffff;
                        cout << min << endl;
                        unsigned int j = 0;
                        for (j = 0; j < i; j++) {
                            if (diff_array[j] < min) {
                                min = diff_array[j];
                                printf("min is %d j is %d diff[j]  is %d\n\n\n\n\n", min, j, diff_array[j]);   
                            }
                        }
                        j = 2;
                        while (j <= min) {
                            if (num_packets == 1) {
                                j = 1;
                                break;
                            } else if (j >= num_packets) {
                                for (k = 0; k < i; k++) {
                                    if (diff_array[k] % j) break;
                                }
                            }
                            if (k == i) break;
                            j++;    
                        }
                        if ((j > min) && ( (min%num_packets==0) || (num_packets%min==0))) j = (num_packets>min)?min:num_packets;
                        printf("number of packets = %d\n", j);
                        num_packets_found = j;
                        //while(1);
                        // malloc an array of num_packets amount of strings of size 20
                        message = (char**) calloc(num_packets_found , sizeof(char*));
                        answer = (char*)malloc(num_packets_found*20);
                        checklist = (int*)calloc(num_packets_found, sizeof(int));
                        for (k = 0; k < num_packets_found; k++) {
                            message[k] = (char*)malloc(20);
                        }
                        i = 0;
                   }
                } else {
                    /*
                     *
                     *  Packet reconstruction
                     *
                     */
                    if (first_char == 0x4 && offsetFound<0) {
                        printf("first eom found\n");
                        count++;
                        offsetFound = 1;
                        index = id % num_packets_found;
                        offset = num_packets_found - index -1;
                        index = (index+offset)%num_packets_found;
                        strcpy(message[index], msg);
                        msg_map[msg] = id;
                        printf("Offset found! %d\n", offset);
                        printf("Message packet is \"%s\" at index %d\n", message[index], index);
                        checklist[index] = 1;
                    } else if (offsetFound > 0) {
                        index = id % num_packets_found;
                        index = (index+offset)%num_packets_found;
                        if (msg_map.count(msg) == 0) {
                            strcpy(message[index], msg);
                            msg_map[msg] = id;
                            printf("Message packet is \"%s\" at index %d\n", message[index], index);
                            count++;
                            checklist[index] = 1;
                        } else {
                            // Check if duplicates exist
                            if (index != msg_map[msg] && checklist[index] == 1) {
                                int diff = index - msg_map[msg];
                                index = msg_map[msg] + diff;
                                msg_map[msg] = index;
                                strcpy(message[index], msg);
                                checklist[index] = 1;

                            }

                        }
                    }
                    if ( count == num_packets_found ) {
                        strcpy(answer, message[0]);
                        for (k = 1; k < num_packets_found; k++) {
                            strcat(answer, message[k]);
                        }
                        printf("Final answer: %s\n", answer);
                        post_answer(answer, &http_server);
                        // reset
                        recv(slave_fd[0], buffer1, sizeof(buffer1),0);
                        recv(slave_fd[0], buffer1, sizeof(buffer1),0);
                        recv(slave_fd[0], buffer1, sizeof(buffer1),0);
                        recv(slave_fd[1], buffer1, sizeof(buffer1),0);
                        recv(slave_fd[1], buffer1, sizeof(buffer1),0);
                        recv(slave_fd[1], buffer1, sizeof(buffer1),0);
                        i = 0;
                        num_packets_found = 0;
                        num_packets = 0;
                        offset = 0;
                        offsetFound = -1;
                        count = 0;
                        msgtoid.clear();
                        msg_map.clear();
                        //while(1);
                    }
                }
        }
    }
    close(httpC);
    close(readSockets_fd[0]);

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

    /*
    printf("[*] Sending new line to complete Authorization...\n");
    buffer = postadickpic(host, "\n");
    send(httpC, buffer.c_str(), buffer.length(), 0);
    recv(httpC, response, sizeof(response), 0);
    printf("[*] HTTP POST response to new line:\n");
    cout << "==================================" << endl;
    cout << response << endl;
    cout << "==================================" << endl;
    cout << endl;
    */
    printf("[*] Authorization complete. Lets go home\n");

} 
