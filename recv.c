#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

#define MAX_SIZE 65535

char buf[MAX_SIZE+1];

#define swap16(x) ((((x)&0xFF) << 8) | (((x) >> 8) & 0xFF))


void SendRecv(int s_fd, char* buf) {
    // strcat(string, "\r\n");
    printf("%s", buf);
    send(s_fd, buf, strlen(buf), 0);
    int r_size = 0;
    if((r_size = recv(s_fd, buf, MAX_SIZE, 0)) == -1) {
        perror("error");
        exit(EXIT_FAILURE);
    }
    buf[r_size] = '\0'; // Do not forget the null terminator
    printf("%s", buf);
}

void recv_mail()
{
    const char* host_name = "pop.qq.com"; // TODO: Specify the mail server domain name
    const unsigned short port = 110; // POP3 server port
    const char* user = "3040976895@qq.com"; // TODO: Specify the user
    const char* pass = "voxzdnveohdcddhf"; // TODO: Specify the password
    char dest_ip[16];
    int s_fd; // socket file descriptor
    struct hostent *host;
    struct in_addr **addr_list;
    int i = 0;
    int r_size;

    // Get IP from domain name
    if ((host = gethostbyname(host_name)) == NULL)
    {
        herror("gethostbyname");
        exit(EXIT_FAILURE);
    }

    addr_list = (struct in_addr **) host->h_addr_list;
    while (addr_list[i] != NULL)
        ++i;
    strcpy(dest_ip, inet_ntoa(*addr_list[i-1]));

    // TODO: Create a socket,return the file descriptor to s_fd, and establish a TCP connection to the POP3 server
    if((s_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        herror("createsoket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in servaddr;
    servaddr.sin_family         = AF_INET;
    servaddr.sin_port           = swap16(port);
    servaddr.sin_addr.s_addr    = inet_addr(dest_ip);
    memset(servaddr.sin_zero, 0, sizeof(servaddr.sin_zero));

    if(connect(s_fd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1) {
        herror("connectsocket");
        exit(EXIT_FAILURE);
    }

    // Print welcome message
    if ((r_size = recv(s_fd, buf, MAX_SIZE, 0)) == -1)
    {
        perror("recv");
        exit(EXIT_FAILURE);
    }
    buf[r_size] = '\0'; // Do not forget the null terminator
    printf("%s", buf);

    // TODO: Send user and password and print server response
    strcpy(buf, "user ");
    strcat(buf, user);
    strcat(buf, "\r\n");
    SendRecv(s_fd, buf);

    strcpy(buf, "pass ");
    strcat(buf, pass);
    strcat(buf, "\r\n");
    SendRecv(s_fd, buf);

    // TODO: Send STAT command and print server response
    strcpy(buf, "stat\r\n");
    SendRecv(s_fd, buf);
    // TODO: Send LIST command and print server response
    strcpy(buf, "list\r\n");
    SendRecv(s_fd, buf);
    // TODO: Retrieve the first mail and print its content
    strcpy(buf, "retr 1\r\n");
    SendRecv(s_fd, buf);
    // TODO: Send QUIT command and print server response
    strcpy(buf, "quit\r\n");
    SendRecv(s_fd, buf);

    close(s_fd);
}

int main(int argc, char* argv[])
{
    recv_mail();
    exit(0);
}
