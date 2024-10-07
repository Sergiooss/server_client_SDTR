#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <signal.h>

#define MAX_BUFF_S 1024
#define MAX_BUFF_R 1024

#define IP "192.168.158.39"
#define PORT 8080

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
        
        if((n_char = recv(tcp_socket, buffer_r, sizeof(buffer_r) - 1, 0)) < 0) {
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