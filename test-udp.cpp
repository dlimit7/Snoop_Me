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
    int ret;
    char * msg;
    map<string, unsigned long long int>msgtoid;
    unsigned int num_packets = 0;
    unsigned int diff_array[10];
    unsigned int diff;
    int i = 0;
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
        //string msg;
        //msg.assign(message, 30);
        //cout << "message is \"" << msg << "\"" << endl; 

        unsigned long long int id = *(unsigned long long int *)response;
        if (msgtoid.count(msg) < 1) {
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
        if (i==10) {
            //printf("num_packs = %d\n", num_packets);
            //printf("\n\n\n\n\n\n");
            i = 0;
            // find lowest common divisor in diff_array
            unsigned int min = 0xfff;
            cout << min << endl;
            unsigned int j = 0;
            for (j = 0; j < 10; j++) {
                if (diff_array[j] < min) {
                    min = diff_array[j];
                    //printf("min is %d j is %d diff[j]  is %d\n\n\n\n\n", min, j, diff_array[j]);   
                }
            }
            int k;
            j = 2;
            while (j <= min) {
               if (j >= num_packets) {
                   for (k = 0; k < 10; k++) {
                       if (diff_array[k] % j) break;
                   }
               }
               if (k == 10) break;
               j++;
            }
            printf("number of packets = %d\n", j);
            while(1);

        }
        
        memset(response, 0, 30);
        mtx.unlock();
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


