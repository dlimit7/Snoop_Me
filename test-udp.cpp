#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <queue>
#include <string.h>
#include <string>
#include <vector>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <poll.h> // int poll(struct pollfd *fds, nfds_t nfds, int timeout);

using namespace std;


//char host[1024] = "skunksmail.ee.unsw.edu.au";
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
    pthread_mutex_t* lock;
    fd_set readfds;
    int client_fd = server_info->get_socket();
    struct sockaddr_in serv_addr = server_info->get_sockstruct();
    struct timeval timeout;
    char response[30] = {0};
    int ret;
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
        pthread_mutex_lock(lock);
        socklen_t addrlen = sizeof(serv_addr);
        if (recvfrom(client_fd, response, sizeof(response), 0, (struct sockaddr *)&serv_addr, &addrlen) < 0) {
            printf("    => Error in receiving the message\n");
            exit(1);
        }
        //cout << response << endl;
        char *msg = (char*)(response+8);
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
        printf("Packet Identifier # 0x%llx:   ", identifier);
        cout << "\"" <<msg << "\"" << endl;
        memset(response, 0, 20);
        memset(msg, 0, 20);
        pthread_mutex_unlock(lock);
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
    // set file descriptor to nonblocking io mode
    int opt = 1;
    ret = ioctl(client_fd, FIONBIO, &opt); 
    if (ret == -1) {
        cout << "[-] ioctl failed: " << errno << endl;
        return -1;
    }

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
