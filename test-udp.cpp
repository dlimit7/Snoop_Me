#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <string>
#include <vector>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

using namespace std;

int main (void) {
    int client;
    int portNo = 8117;
    //char host[1024] = "skunksmail.ee.unsw.edu.au";
    unsigned int S[1] = {htonl(10)}; // S value
    char response[90];

   // printf("[*] Creating a socket...\n");
    if ((client = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        printf("    => Error in creating UDP socket\n");
        exit(1);
    }

    struct sockaddr_in serv_addr;
    memset((char *)&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(portNo);

    struct sockaddr_in client_addr;
    memset((char *)&client_addr, 0, sizeof(client_addr));
    client_addr.sin_family = AF_INET;
    client_addr.sin_addr.s_addr = htonl(INADDR_ANY); // takes local host address... need to set -multi_snooper flag
    client_addr.sin_port = htons(8001);

    printf("[*] Validating IP Address...\n");
    if (inet_pton(AF_INET, "149.171.36.173", &serv_addr.sin_addr) <= 0) {
        printf("    => Invalid Address\n");
        exit(1);
    }
    int i = 0;
    while (i < 30) {
        if (sendto(client, S,sizeof(S), 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
            printf("    => Error in sending the message\n");
            exit(1);
        }
        //printf("[*] Awaiting for response from the server...\n");
        socklen_t addrlen = sizeof(serv_addr);
        if (recvfrom(client, response, sizeof(response), 0, (struct sockaddr *)&serv_addr, &addrlen) < 0) {
            printf("    => Error in receiving the message\n");
            exit(1);
        }
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

        // The message
        char *msg = (char*)(response+8);
        cout << "\"" <<msg << "\"" << endl;
        memset(response, 0, 20);
        memset(msg, 0, 20);
        i++;
    }
    close(client);
    return 0;
}
