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
    char host[1024] = "skunksmail.ee.unsw.edu.au";
    //string buffer;
    int buffer[1];
    buffer[0] = 1;
    int buffertosend[1];
    char response[9000];

    printf("[*] Creating a socket...\n");
    if ((client = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        printf("    => Error in creating UDP socket\n");
        exit(1);
    }

    struct sockaddr_in serv_addr;
    memset((char *)&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(portNo);

    //struct sockaddr_in client_addr;
    //memset((char *)&client_addr, 0, sizeof(client_addr));
    //client_addr.sin_family = AF_INET;
    //client_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    //client_addr.sin_port = htons(8000);

//    printf("[*] Binding client socket...");
//    if (bind(client, (struct sockaddr *)&client_addr, sizeof(client_addr)) < 0) {
//        printf("    => Error in binding socket");
//        exit(1);
//    }

    printf("[*] Validating IP Address...\n");
    if (inet_pton(AF_INET, "149.171.36.173", &serv_addr.sin_addr) <= 0) {
        printf("    => Invalid Address\n");
        exit(1);
    }
    int i = 0;
    int S = 1;
    cout << *buffer << endl;
    buffertosend[0] = htonl(buffer[0]);
    //buffertosend[0] = buffer[0];
    cout << *buffertosend << endl;
    while (i < 30) {
        printf("[*] Attempting to send message via UDP\n");
        //if (sendto(client, buffer.c_str(), buffer.length(), 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        if (sendto(client, buffertosend, sizeof(buffertosend), 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
            printf("    => Error in sending the message\n");
            exit(1);
        }
        printf("[*] Awaiting for response from the server...\n");
        socklen_t addrlen = sizeof(serv_addr);
        if (recvfrom(client, response, sizeof(response), 0, (struct sockaddr *)&serv_addr, &addrlen) < 0) {
            printf("    => Error in receiving the message\n");
            exit(1);
        }
        printf("[*] Snooped message\n");
        cout << response << endl;
        i++;
    }

    //printf("[*] Awaiting for response from the server...\n");
    //socklen_t addrlen = sizeof(serv_addr);
    //recvfrom(client, response, sizeof(response), 0, (struct sockaddr *)&serv_addr, &addrlen);
    close(client);
    return 0;
}
