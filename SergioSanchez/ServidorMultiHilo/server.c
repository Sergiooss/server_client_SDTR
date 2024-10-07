#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <signal.h>
#include <pthread.h>
#include <stdint.h>

#define MAX_NUM_THREADS 100
#define MAX_BUFF_S 1024
#define MAX_BUFF_R 1024

#define IP "192.168.158.39"

#define MAX_PORT 65535
#define MIN_PORT 1024

/*
void sigint(int signum) {
    close(tcp_socket);
    // close(fd);
    exit(0); 
}
*/

void* iterations(void* args) {
    char buffer_s[MAX_BUFF_S] = "Hello client!";
    char buffer_r[MAX_BUFF_R];
    ssize_t n_char;

    int* fd = (int*)args;

    // fprintf(stderr,"CREO HILO con fd %i\n", *fd);

    if((n_char = recv(*fd, buffer_r, sizeof(buffer_r) - 1, 0)) < 0) {
        perror("Error receiving the input");
        close(*fd);
        pthread_exit(NULL);
    } else if(n_char == 0) {
        close(*fd);
        pthread_exit(NULL);
    } else {
        buffer_r[n_char] = '\0';
        printf("> %s\n",buffer_r);
    }
    
    if (send(*fd, buffer_s, sizeof(buffer_s) - 1, 0) < 0) {
        perror("Error sending the input");
        close(*fd);
        pthread_exit(NULL);
    }
    close(*fd);
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    argc--;
    argv++;

    if(argc != 1) {
        fprintf(stderr, "Usage: ./server [server_port]");
        return -1;
    }

    struct sockaddr_in addr;

    socklen_t addr_len = sizeof(addr);

    // signal(SIGINT, sigint);
    setbuf(stdout, NULL);

    int tcp_socket = socket(AF_INET, SOCK_STREAM, 0);
    
    if(tcp_socket == -1){
        perror("Socket unsuccessfully created…");
        return -1;
    }

    printf("Socket successfully created…\n");
    addr.sin_family = AF_INET;

    int port = atoi(argv[0]);

    if(port < MIN_PORT || port >= MAX_PORT){
        fprintf(stderr, "[cliente_port] canot be %s. It must be within this range 1024-65535\n", argv[0]);
        return -1;
    }

    // convierto a red
    addr.sin_port = htons((uint16_t)port);

    // Establezco una direccion de ip usando ifconfig
    if (inet_pton(AF_INET, IP, &addr.sin_addr) <= 0) {
        fprintf(stderr, "Invalid address: %s\n", argv[1]);
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

    if(listen(tcp_socket, MAX_NUM_THREADS) == -1){
        perror("Socket unsuccessfully listening…");
        close(tcp_socket);
        return -1;
    }

    printf("Server listening…\n");

    pthread_t threads[MAX_NUM_THREADS];
    int i = 0, fd[MAX_NUM_THREADS], executing = 0;

    while(1) {
        fd[i] = accept(tcp_socket, (struct sockaddr *)&addr, &addr_len);

        if(fd[i] < 0){
            perror("Socket was not accept");
            // close(tcp_socket);
            // return -1;
        } else {
            if((i + 1) < MAX_NUM_THREADS) {
                // fprintf(stderr,"CREO HILO con fd %i\n",fd[i]);
                pthread_create(&threads[i], NULL, iterations, (void*)&(fd[i]));
                i++;
            } else {
                for (int j = 0; j < MAX_NUM_THREADS; j++) {
                    pthread_join(threads[j], NULL);
                }
                i = 0;
            }
        }
    }
}