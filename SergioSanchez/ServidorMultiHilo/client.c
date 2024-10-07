#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <signal.h>

#define MAX_PORT 65535
#define MIN_PORT 1024

#define MAX_BUFF_S 1024
#define MAX_BUFF_R 1024

int tcp_socket;

void sigint(int signum) {
    close(tcp_socket);
    exit(0);
}

int main(int argc, char *argv[]) {
    argc--;
    argv++;

    if(argc != 3) {
        fprintf(stderr, "Usage: ./client [cliente_id] [cliente_ip] [cliente_port]\n");
        return -1;
    }

    int client_id = atoi(argv[0]), port = atoi(argv[2]);

    if(client_id == 0){
        fprintf(stderr, "[cliente_id] canot be %s\n", argv[0]);
        return -1;
    }

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

    if(port < MIN_PORT || port > MAX_PORT){
        fprintf(stderr, "[cliente_port] canot be %s. It must be within this range 1024-65535\n", argv[0]);
        return -1;
    }

    // convierto a red
    addr.sin_port = htons((uint16_t)port);

    // Establezco una direccion de ip usando ifconfig
    if (inet_pton(AF_INET, argv[1], &addr.sin_addr) <= 0) {
        fprintf(stderr, "Invalid address: %s\n", argv[1]);
        return -1;
    }

    if (connect(tcp_socket, (struct sockaddr *)&addr, sizeof(addr)) == -1){
        perror("Socket unsuccessfully connected…");
        close(tcp_socket);
        return -1;
    }

    printf("connected to the server...\n");

    char buffer_s[MAX_BUFF_S], buffer_r[MAX_BUFF_R];

    ssize_t n_char;
    snprintf(buffer_s, sizeof(buffer_s), "Hello server! From client: %i", client_id);

    if (send(tcp_socket, buffer_s, sizeof(buffer_s) - 1, 0) < 0) {
        perror("Error sending the input");
    }

    if((n_char = recv(tcp_socket, buffer_r, sizeof(buffer_r) - 1, 0)) < 0) {
        perror("Error receiving the input");
    } else if(n_char == 0) {
        close(tcp_socket);
        return 0;
    } else {
        buffer_r[n_char] = '\0';
        printf("+++ %s\n",buffer_r);
    }

    close(tcp_socket);
    return 0;
}