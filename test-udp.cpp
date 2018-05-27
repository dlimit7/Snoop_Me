#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <queue>
#include <map>
#include <string.h>
#include <string>
#include <vector>
#include <mutex>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <poll.h> // int poll(struct pollfd *fds, nfds_t nfds, int timeout);

using namespace std;

void warp(char*);

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
        memset((char *)&tmp, 0, sizeof(tmp));
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

void *snooper(void* serv_info) {
    unsigned int S[1] = {10}; // S value
    int i = 0;
    ServerInfo *server_info = (ServerInfo*)serv_info;
    int client_fd = server_info->get_socket();
    struct sockaddr_in serv_addr = server_info->get_sockstruct();
    while (1) {
        i%2 ? (S[0]--):(S[0]++); i++;
        unsigned int buffer[1] = {htonl(S[0])}; 
        if (sendto(client_fd, buffer,sizeof(buffer), 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
            printf("    => Error in sending the message\n");
            exit(1);
        }
    }
}

void receive(ServerInfo *server_info) {
    mutex mtx;
    fd_set readfds;
    int client_fd = server_info->get_socket();
    struct sockaddr_in serv_addr = server_info->get_sockstruct();
    struct timeval timeout;
    char response[30];
    char *answer = NULL;
    int * checklist = NULL;
    int ret;
    char * msg;
    map<string, unsigned long long int>msgtoid;
    map<string, unsigned int>msg_map;
    unsigned int num_packets = 0;
    unsigned int diff_array[10];
    unsigned int diff;
    int i = 0;
    int num_packets_found = 0;
    char ** message= NULL;
    unsigned int offset = 0;
    unsigned int index;
    int count = 0;
    int offsetFound = -1;
    int k;
    while (1) {
        //printf("[*] Awaiting for response from the server...\n");
        FD_ZERO(&readfds);
        FD_SET(client_fd, &readfds);
        timeout.tv_sec = 5;
        timeout.tv_usec = 0;
        ret = select(client_fd+1, &readfds, NULL, NULL, &timeout);
        if (ret == -1) {
            cout << "[-] select failed: " << errno << endl;
            exit(1);
        }
        mtx.lock();
        socklen_t addrlen = sizeof(serv_addr);
        if (recvfrom(client_fd, response, sizeof(response), 0, (struct sockaddr *)&serv_addr, &addrlen) < 0) {
            printf("    => Error in receiving the message\n");
            exit(1);
        }
        ///cout << "response " << response << endl;
        warp(&response[0]);
        
        /**** 
         * 
         *    Packet Processing
         *
         * ***/
        msg = response+8;
        char first_char = msg[0];
        //cout << "message is \"" << msg << "\"" << endl; 
        unsigned long long int id = *(unsigned long long int *)response;
        if (!num_packets_found) { 
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
            if (i == 10) {
                printf("num_packs = %d\n", num_packets);
                i = 0;
                // find lowest common divisor in diff_array
                unsigned int min = 0xfff;
                cout << min << endl;
                unsigned int j = 0;
                for (j = 0; j < 10; j++) {
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
                        for (k = 0; k < 10; k++) {
                            if (diff_array[k] % j) break;
                        }
                    }
                    if (k == 10) break;
                    j++;    
                }
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
                while(1);
            }

        }
       
        mtx.unlock();
        memset(response, 0, 30);
    }
}


int main (int argc, char*argv[]) {

    //int portNo = 8119;
    int client_fd;
    int ret=0;

    if (argc != 3) {
        cout << "[-] Usage: " << argv[0] << " <server_ip> <server_port>" << endl;
        return 1;
    }
    printf("[*] Creating a socket...\n");
    if ((client_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        printf("    => Error in creating UDP socket\n");
        exit(1);
    }
    printf("[*] Socket successfully created\n");
    // set file descriptor to nonblocking io mode
    int opt = 1;
    ret = ioctl(client_fd, FIONBIO, &opt); 
    if (ret == -1) {
        cout << "[-] ioctl failed: " << errno << endl;
        return -1;
    }
    cout << "Starting snooper and receiver" << endl;
    ServerInfo server_info = ServerInfo(argv[1], argv[2], client_fd);
    pthread_t send;
    pthread_create(&send, NULL, snooper, (void*)&server_info);
    receive(&server_info);

/*
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(portNo);
    printf("[*] Validating IP Address...\n");
    if (inet_pton(AF_INET, "149.171.36.173", &serv_addr.sin_addr) <= 0) {
        printf("    => Invalid Address\n");
        exit(1);
    }
    struct sockaddr_in client_addr;
    memset((char *)&client_addr, 0, sizeof(client_addr));
    client_addr.sin_family = AF_INET;
    client_addr.sin_addr.s_addr = htonl(INADDR_ANY); // takes local host address... need to set -multi_snooper flag
    client_addr.sin_port = htons(8001);
*/
    

    close(client_fd);

    return 0;
}



void warp (char * response) {
        char *msg = response+8;
        //cout << " MESSAGE\"" <<msg << "\"" << endl;
        // Response has both the 8 byte packet identifier and the message.
        // Convert 8 byte big endian to little endian
        unsigned int lo = *(unsigned int *)response;
        unsigned int  hi =  *(unsigned int*)(response+8);
        unsigned long long int identifier = *(unsigned long long int*)response;
        //printf("# 0x%llx:   \n", identifier);
        lo = (unsigned int) (identifier);
        lo = ntohl(lo);
        identifier = identifier >> 32;
        hi = (unsigned int) (identifier);
        hi = ntohl(hi);
        identifier = 0;
        identifier |= lo;
        identifier = identifier << 32;
        identifier |= hi;
        printf("Packet Id #: 0x%llx:   ", identifier);
        unsigned long long int * temp = (unsigned long long int *) response;
        *temp = identifier;
        cout << "\"" << msg << "\"" << endl;
}


