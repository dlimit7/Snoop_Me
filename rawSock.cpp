#include <iostream>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>

using namespace std;

int main() {

    int s;
    if ((s = socket(AF_INET, SOCK_RAW, IPPROTO_UDP)) < 0) {
//        printf("socket() failed\n");
        cout << errno << endl;
        return -1;
    }
    return 0;
}





