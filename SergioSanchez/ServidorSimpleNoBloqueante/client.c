#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <signal.h>
#include <errno.h>
#include <sys/select.h>

#define MAX_BUFF_S 1024
#define MAX_BUFF_R 1024

#define IP "192.168.158.39"
#define PORT 8080

#define TIME_OUT_NANO 500000

int tcp_socket;

void sigint(int signum) {
    close(tcp_socket);
    exit(0);
}

int main() {
    struct sockaddr_in addr;

    signal(SIGINT, sigint);
    setbuf(stdout, NULL);
    tcp_socket = socket(AF_INET, SOCK_STREAM, 0);

    if(tcp_socket == -1){
        perror("Socket unsuccessfully created…");
        return -1;
    }

    printf("Socket successfully created…\n");

    addr.sin_family = AF_INET;

    // convierto a red
    addr.sin_port = htons(PORT);

    // Establezco una direccion de ip usando ifconfig
    if (inet_pton(AF_INET, IP, &addr.sin_addr) <= 0) {
        perror("Invalid address");
        return -1;
    }

    if (connect(tcp_socket, (struct sockaddr *)&addr, sizeof(addr)) == -1){
        perror("Socket unsuccessfully connected…");
        close(tcp_socket);
        return -1;
    }

    printf("connected to the server...\n");

    char buffer_s[MAX_BUFF_S];
    char buffer_r[MAX_BUFF_R];
    ssize_t n_char;

    while(1) {
        printf("> ");
        if(fgets(buffer_s, sizeof(buffer_s), stdin) == NULL){
            perror("Error reading the input");
        } else{
            if (send(tcp_socket, buffer_s, sizeof(buffer_s) - 1, 0) < 0) {
                perror("Error sending the input");
            }
        }
        
        int activity, maxfd = tcp_socket + 1;
        fd_set readfds;
        struct timeval timeout;

        FD_ZERO(&readfds);
        FD_SET(tcp_socket, &readfds);

        timeout.tv_sec = 0;
        timeout.tv_usec = TIME_OUT_NANO;

        activity = select(maxfd, &readfds, NULL, NULL, &timeout);

        if(activity < 0) {
            perror("Error in select");
            close(tcp_socket);
            close(tcp_socket);
            exit(-1);
        } else if(FD_ISSET(tcp_socket, &readfds)) {
            if((n_char = recv(tcp_socket, buffer_r, sizeof(buffer_r) - 1, MSG_DONTWAIT)) < 0) {
                perror("Error receiving the input");
            } else if(n_char == 0) {
                close(tcp_socket);
                return 0;
            } else {
                buffer_r[n_char] = '\0';
                printf("+++ %s",buffer_r);
            }
        }
    }
}