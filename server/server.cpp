/** Usage:
  * Used to simply test the tcp connection to the android app. When connected messages can be sent.
  * Currently only one way connection is used.
  * Valid messages:
  *     0: driver is not distracted
  *     1: driver is distracted
  *     any other char closes the connection.
  */

#include <stdlib.h>

#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <iostream>
#include <string>
#include <stdio.h>
using namespace std;

const string ip = "AND.ROID.IP.ADDR";
const unsigned short port = 8080;
void connection(int sock) {
    char *distracted = (char *) malloc(4);
    
    for (fgets(distracted, 3, stdin); isdigit(distracted[0]); fgets(distracted, 3, stdin)) {
        if (distracted[0] == '0') {
            fprintf(stderr, "sent: 0\n");
            send(sock, (char *)"0\n", 2, 0);
        }
        else if (distracted[0] == '1') {
            fprintf(stderr, "sent: 1\n");
            send(sock, (char *)"1\n", 2, 0);
        }
        else
            break;        
    }
    fprintf(stderr, "closing\n");
    send(sock, (char *)"9\n", 2, 0);
    close(sock);
}

int main(int argc, char *argv[]) {
    int sock, csock;
    struct sockaddr_in serv_addr, cli_addr;
    char *host = (char *) malloc(16); strcat(host, ip.c_str());   // todo: change later

    // First create a socket
    fprintf(stderr, "create socket\n");
    sock  = socket(AF_INET, SOCK_STREAM, 0);

    if (sock < 0) {
        perror("ERROR opening socket");
        exit(1);
    }
    fprintf(stderr, "serv_addr mumbo jumbo\n");
    serv_addr.sin_family = PF_INET;
    serv_addr.sin_port = htons(port);
    if (inet_pton(AF_INET, host, &serv_addr.sin_addr) != 1) {
        perror("inet_pton");
        exit(EXIT_FAILURE);
    }
    fprintf(stderr, "bind host addr\n");
    // bind the host address using bind() call
    while(true) {
        if (bind(sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
            perror("Error on binding");
            sleep(15);

        }
        else break;
    }
    fprintf(stderr, "start listening for rigamarole\n");
    // listen for client
    if (listen(sock, 2) != 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    // accept connection from client
    socklen_t clilen = (socklen_t) sizeof(cli_addr);
    fprintf(stderr, "before accepting their connection request\n");
    csock = accept(sock, (struct sockaddr *) &cli_addr, &clilen);
    fprintf(stderr, "accepted!!\n");
    if (csock < 0) {
        perror("accept");
        exit(EXIT_FAILURE);
    }

    // if connection is established, start communicating
    // n = send(newsock, 1, 1);
    fprintf(stderr, "connect to the client if you feel like it\n");
    connection(csock);
    close(sock);
    /*
    if (n < 0) {
        perror("ERROR writing to socket");
        exit(1);
    }
    */

    return 0;
}