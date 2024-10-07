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

int tcp_socket, fd;

void sigint(int signum) {
    close(tcp_socket);
    close(fd);
    exit(0); 
}

int main() {
    struct sockaddr_in addr;

    socklen_t addr_len = sizeof(addr);

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

    const int enable = 1;
    if (setsockopt(tcp_socket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
        perror("setsockopt(SO_REUSEADDR) failed");

    if (bind(tcp_socket, (struct sockaddr *)&addr, sizeof(addr)) == -1){
        perror("Socket unsuccessfully connected…");
        close(tcp_socket);
        return -1;
    }

    printf("Socket successfully binded...\n");

    if(listen(tcp_socket, 1) == -1){
        perror("Socket unsuccessfully listening…");
        close(tcp_socket);
        return -1;
    }

    printf("Server listening…\n");

    if((fd = accept(tcp_socket, (struct sockaddr *)&addr, &addr_len)) == -1){
        perror("Socket was not accept");
        close(tcp_socket);
        return -1;
    }

    char buffer_s[MAX_BUFF_S];
    char buffer_r[MAX_BUFF_R];
    ssize_t n_char;

    while(1) {
        int activity, maxfd = fd + 1;
        fd_set readfds;
        struct timeval timeout;

        FD_ZERO(&readfds);
        FD_SET(fd, &readfds);

        timeout.tv_sec = 0;
        timeout.tv_usec = TIME_OUT_NANO;
        activity = select(maxfd, &readfds, NULL, NULL, &timeout);

        if(activity < 0) {
            perror("Error in select");
            close(tcp_socket);
            close(fd);
            exit(-1);
        } else if(FD_ISSET(fd, &readfds)) {
            if((n_char = recv(fd, buffer_r, sizeof(buffer_r) - 1, MSG_DONTWAIT)) < 0) {
                perror("Error receiving the input");
            } else if(n_char == 0) {
                close(tcp_socket);
                close(fd);
                return 0;
            } else {
                buffer_r[n_char] = '\0';
                printf("+++ %s",buffer_r);
            }
        }

        printf("> ");
        if(fgets(buffer_s, sizeof(buffer_s), stdin) == NULL){
            perror("Error reading the input");
        } else{
            if (send(fd, buffer_s, sizeof(buffer_s) - 1, 0) < 0) {
                perror("Error sending the input");
            }
        }
    }
}