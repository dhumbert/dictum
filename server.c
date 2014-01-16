#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include "db.h"
#include "util.h"

void error(char *msg)
{
  perror(msg);
  exit(1);
}

void handle_connection(int sock)
{
    int n;
    char buffer[256];

    bzero(buffer,256);
    n = read(sock,buffer,255);
    if (n < 0) error("ERROR reading from socket");

    const char *cmd = strip_spaces(buffer);

    printf("Here is the message: %s\n", cmd);
    n = write(sock,"I got your message\n",18);
    if (n < 0) error("ERROR writing to socket");
}

void run_server(int portno)
{
    signal(SIGCHLD,SIG_IGN);

    int sockfd, newsockfd, clilen, pid;
    
    struct sockaddr_in serv_addr, cli_addr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
      error("ERROR opening socket");
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        error("ERROR on binding");
    }

    listen(sockfd,5);
    clilen = sizeof(cli_addr);

    while (1) {
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        if (newsockfd < 0) error("ERROR on accept");
        pid = fork();

        if (pid < 0) error("ERROR on fork");
        if (pid == 0) {
            close(sockfd);
            handle_connection(newsockfd);
            exit(0);
        } else {
            close(newsockfd);
        }
    }
    close(sockfd);
}

int main(int argc, char *argv[])
{
    int port = 4486;
    run_server(port);
    Db *db = db_init();
    db_set(db, "key1", "value1");
    const char *value = db_get(db, "key1");
    printf("%s\n", value);
}
