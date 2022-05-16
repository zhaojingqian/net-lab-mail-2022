#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <getopt.h>
#include "base64_utils.h"

#define MAX_SIZE 4095
#define ATT_MAX_SIZE 4096000

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

// receiver: mail address of the recipient
// subject: mail subject
// msg: content of mail body or path to the file containing mail body
// att_path: path to the attachment
void send_mail(const char* receiver, const char* subject, const char* msg, const char* att_path)
{
    const char* end_msg = "\r\n.\r\n";
    const char* host_name = "smtp.qq.com"; // TODO: Specify the mail server domain name
    const unsigned short port = 25; // SMTP server port
    const char* user = "3040976895@qq.com"; // TODO: Specify the user
    const char* pass = "voxzdnveohdcddhf"; // TODO: Specify the password
    const char* from = "3040976895@qq.com"; // TODO: Specify the mail address of the sender

    // const char* host_name = "smtp.qq.com"; // TODO: Specify the mail server domain name
    // const unsigned short port = 25; // SMTP server port
    // const char* user = "xxxxxxxxxx@qq.com"; // TODO: Specify the user
    // const char* pass = "xxxxxxxxxxxxxx"; // TODO: Specify the password
    // const char* from = "xxxxxxxxxx@qq.com"; // TODO: Specify the mail address of the sender


    char dest_ip[16]; // Mail server IP address
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

    // TODO: Create a socket, return the file descriptor to s_fd, and establish a TCP connection to the mail server
    if((s_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        herror("createsocket");
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


    // Send EHLO command and print server response
    strcpy(buf, "ehlo qq.com\r\n");
    SendRecv(s_fd, buf);

    // TODO: Authentication. Server response should be printed out.
    // const char* auth = "AUTH login\r\n";
    strcpy(buf, "AUTH login\r\n");
    SendRecv(s_fd, buf);

    char* mail_name = encode_str(user);
    strcat(mail_name, "\r\n");
    strcpy(buf, mail_name);
    SendRecv(s_fd, buf);
    free(mail_name);

    char* mail_pass = encode_str(pass);
    strcat(mail_pass, "\r\n");
    strcpy(buf, mail_pass);
    SendRecv(s_fd, buf);
    free(mail_pass);

    // TODO: Send MAIL FROM command and print server response
    strcpy(buf, "mail from:<");
    strcat(buf, user);
    strcat(buf, ">\r\n");
    SendRecv(s_fd, buf);

    // TODO: Send RCPT TO command and print server response
    strcpy(buf, "rcpt to:<");
    strcat(buf, receiver);
    strcat(buf, ">\r\n");
    SendRecv(s_fd, buf);
    
    // TODO: Send DATA command and print server response
    strcpy(buf, "data\r\n");
    SendRecv(s_fd, buf);

    // TODO: Send message data
    strcpy(buf, "From:");
    strcat(buf, from);  
    strcat(buf, "\r\nTo:");
    strcat(buf, receiver);
    strcat(buf, "\r\nMIME-Version: 1.0\r\n");  
    strcat(buf, "Subject:");
    strcat(buf, subject);   
    strcat(buf, "\r\n");   
    strcat(buf, "Content-Type: multipart/mixed; boundary=qwertyuiopasdfghjklzxcvbnm\r\n");
  
    send(s_fd, buf, strlen(buf), 0); 

    if(msg) {
        strcpy(buf, "\r\n--qwertyuiopasdfghjklzxcvbnm\r\n");
        strcat(buf, "Content-Type: text/plain\r\n\r\n");
        send(s_fd, buf, strlen(buf), 0);
        char msg_buf[MAX_SIZE+1];
        FILE *raw_file = fopen(msg, "r");
        if(raw_file == NULL) {
            printf("%s\n", msg);
            strcpy(buf, msg);
            strcat(buf, "\r\n");
            send(s_fd, buf, strlen(buf), 0);  
        } else {
            fread(msg_buf, 1, MAX_SIZE, raw_file);
            fclose(raw_file);
            printf("%s\n", msg_buf);
            send(s_fd, msg_buf, strlen(msg_buf), 0);
            strcpy(buf, "\r\n");
            send(s_fd, buf, strlen(buf), 0);    
        }
     
    }

    if(att_path) {
        strcpy(buf, "\r\n--qwertyuiopasdfghjklzxcvbnm\r\n");
        strcat(buf, "Content-Type: application/octet-stream\r\n");
        strcat(buf, "Content-Disposition: attachment; name=");
        strcat(buf, att_path);
        strcat(buf, "\r\n");
        strcat(buf, "Content-Transfer-Encoding: base64\r\n\r\n");
        send(s_fd, buf, strlen(buf), 0);
        printf("%s", buf);

        char att_buf[ATT_MAX_SIZE];
        FILE *raw_file = fopen(att_path, "rb");
        FILE *base64_file = tmpfile();
        encode_file(raw_file, base64_file);
        fclose(raw_file);
        rewind(base64_file);
        fread(att_buf, 1, ATT_MAX_SIZE, base64_file);
        att_buf[ATT_MAX_SIZE-1] = '\0';
        fclose(base64_file);

        send(s_fd, att_buf, strlen(att_buf), 0);
    }

    // TODO: Message ends with a single period
    strcpy(buf, end_msg);
    SendRecv(s_fd, buf);

    // TODO: Send QUIT command and print server response
    strcpy(buf, "quit\r\n");
    SendRecv(s_fd, buf);

    close(s_fd);
}

int main(int argc, char* argv[])
{
    int opt;
    char* s_arg = NULL;
    char* m_arg = NULL;
    char* a_arg = NULL;
    char* recipient = NULL;
    const char* optstring = ":s:m:a:";
    while ((opt = getopt(argc, argv, optstring)) != -1)
    {
        switch (opt)
        {
        case 's':
            s_arg = optarg;
            break;
        case 'm':
            m_arg = optarg;
            break;
        case 'a':
            a_arg = optarg;
            break;
        case ':':
            fprintf(stderr, "Option %c needs an argument.\n", optopt);
            exit(EXIT_FAILURE);
        case '?':
            fprintf(stderr, "Unknown option: %c.\n", optopt);
            exit(EXIT_FAILURE);
        default:
            fprintf(stderr, "Unknown error.\n");
            exit(EXIT_FAILURE);
        }
    }

    if (optind == argc)
    {
        fprintf(stderr, "Recipient not specified.\n");
        exit(EXIT_FAILURE);
    }
    else if (optind < argc - 1)
    {
        fprintf(stderr, "Too many arguments.\n");
        exit(EXIT_FAILURE);
    }
    else
    {
        recipient = argv[optind];
        send_mail(recipient, s_arg, m_arg, a_arg);
        exit(0);
    }
}
