#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <vector>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>
using namespace std;

struct pseudo_header {
    u_int32_t source_address;
    u_int32_t dest_address;
    u_int8_t reserved;
    u_int8_t protocol;
    u_int16_t udp_length;
};

unsigned short csum(unsigned short *ptr, int nBytes);
void createRawSock();

int main (int argc, char *argv[]) {
    int serverPort = 8119;
    int masterPort = 8000;
    int cliOnePort = 8000;
    int cliTwoPort = 8000;
    unsigned int S[1] = {htonl(50)};
    int master;
    if ((master = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) < 0) {
        printf("    => Error in creating UDP socket file descriptor\n");
        exit(1);
    }
    struct sockaddr_in serv_addr;
    memset((char *)&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(serverPort);

    createRawSock();

    return 0;
}

unsigned short csum(unsigned short *ptr, int nBytes) {
    long sum;
    unsigned short oddbyte;
    short answer;

    sum = 0;
    while (nBytes > 1) {
        sum+=*ptr++;
        nBytes-=2;
    }
    if (nBytes == 1) {
        oddbyte = 0;
        *((u_char*)&oddbyte)=*(u_char*)ptr;
        sum+=oddbyte;
    }
    sum = (sum>>16)+(sum & 0xffff);
    sum = sum+ (sum>>16);
    answer = (short)~sum;

    return answer;
}

void createRawSock() {

    int s = socket(PF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (s < 0) {
        printf("    => Error in creating RAW socket file descriptor\n");
        exit(1);
    }
    //unsigned int S[1] = {htonl(50)};
    char datagram[4096];
    char *psdogram;
    char *data;
    unsigned int S[1];
    memset(datagram, 0, 4096);

    char source_ip[32];

    struct iphdr *iph = (struct iphdr*) datagram;
    struct udphdr *udph = (struct udphdr *) (datagram + sizeof(struct ip));
    struct sockaddr_in serv_addr;

    data = datagram + sizeof(struct iphdr) + sizeof(struct udphdr);
    strcpy(data, "10");

    strcpy(source_ip, "192.168.1.100");
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(8117);
    serv_addr.sin_addr.s_addr = inet_addr("149.171.36.173");

    /*  IP Header Population
     */
    iph->ihl = 5;       //Internet Header Length. Min value is 5 -> 5*32 bits = 20 bytes
    iph->version = 4;       //IPv4
    iph->tos= 0;        //Type of Service = 0
    iph->tot_len = sizeof(struct iphdr) + sizeof(struct udphdr) + strlen(data);
    iph->id = htonl(12345);     //ID of packet for fragmentation
    iph->frag_off = 0;      //fragmentation offset
    iph->ttl = 255;     //TTL
    iph->protocol = IPPROTO_ICMP;        //UDP Protocol
    iph->check = 0;     //Checksum. Initial vlaue is 0.
    iph->saddr = inet_addr(source_ip);      //src IP address
    iph->daddr = serv_addr.sin_addr.s_addr;     //dst IP address

    iph->check = csum((unsigned short *)datagram, iph->tot_len);

    /*  UDP Header Population
     */
    udph->source = htons(8000);     //Source port number
    udph->dest = htons(8117);       //Destination port number
    udph->len = htons(8 + strlen(data));        //UDP header size is 8 bytes
    udph->check = 0;

    struct pseudo_header psdoh;
    psdoh.source_address = iph->saddr;
    psdoh.dest_address = iph->daddr;
    psdoh.reserved = 0;
    psdoh.protocol = IPPROTO_UDP;
    psdoh.udp_length = htons(sizeof(struct udphdr) + strlen(data));

    int psdosize = sizeof(struct pseudo_header) + sizeof(struct udphdr) + strlen(data);
    psdogram = (char *)malloc((int)psdosize);
    memcpy(psdogram, (char*)&psdoh, sizeof(struct pseudo_header));
    memcpy(psdogram + sizeof(struct pseudo_header), udph, sizeof(struct udphdr) + strlen(data));

    udph->check = csum((unsigned short *)psdogram, psdosize);

    if (sendto(s, datagram, iph->tot_len, 0, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        printf("sendto failed\n");
        cout << errno << endl;
    }
}
