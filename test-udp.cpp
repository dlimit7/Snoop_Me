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

int duplicate(char msgArray[500][500]);

int main (void) {
    int client;
    int portNo = 8119;
    //char host[1024] = "skunksmail.ee.unsw.edu.au";
    unsigned int S[1] = {htonl(50)}; // S value
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

    //printf("[*] Validating IP Address...\n");
    if (inet_pton(AF_INET, "149.171.36.173", &serv_addr.sin_addr) <= 0) {
        printf("    => Invalid Address\n");
        exit(1);
    }
    int i = 0, x = 0, y = 0;
    unsigned long long int identArray[9000];
    string str;
    char msgArray[500][500];
    while (i < 30) {
        if (sendto(client, S,sizeof(100), 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
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
        printf("# 0x%llx:   ", identifier);

        // The message
        char *msg = (char*)(response+8);
        cout << "\"" << msg << "\"" << endl;
        identArray[i] = identifier;
        memcpy(msgArray[i], msg, 500);
        memset(response, 0, 20);
        memset(msg, 0, 20);
        i++;
    }
    while (x < i) {
        //cout << identArray[x] << endl;
        printf("0x%llx ", identArray[x]);
        cout << "\"" << msgArray[x] << "\"" << endl;
        x++;
    }
    y = duplicate(msgArray);
    cout << "index is: " <<y << endl;
    close(client);
    return 0;
}

int duplicate(char msgArray[500][500]) {
    int currindex, nextindex;
    int i = 2;
    char *tmp;
    tmp = msgArray[1];
    cout << tmp << endl;
    currindex = i;

    while (i < 500) {
        if (msgArray[i] == tmp) {
            nextindex = i;
            i = 500;
        }
        i++;
    }
    //nextindex = i;

    return nextindex;
}
