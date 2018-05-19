#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <errno.h>
#include <fcntl.h>

void rm_spaces(void *src);

void rm_slash_n(void *src);

int send_safe(int socket, void *buffer, size_t length);

int get_safe(int socket_fd, char *message);

int setnonblocking(int sock) {
    int opts;

    opts = fcntl(sock, F_GETFL);
    if (opts < 0) {
        perror("fcntl(F_GETFL)");
        return -1;
    }
    opts = (opts | O_NONBLOCK);
    if (fcntl(sock, F_SETFL, opts) < 0) {
        perror("fcntl(F_SETFL)");
        return -1;
    }

    return 0;
}

int main(int argc, char **argv) {
    if (argc > 1 && strcmp(argv[1], "-help") == 0) {
        printf("Usage: ./server [port](optional)\n");
        return 0;
    }

    int listen_file_dirictory;
    struct sockaddr_in server_address;
    int port = 3030;

    printf("...Server initialization\n");

    if (argc > 1) {
        port = atoi(argv[1]);
        if (port < 1 || port > 9999) {
            printf("Wrong argument, usage: ./server [port]{0001-9999}\n");
            exit(0);
        }
        printf("...The specified %d port will be used\n", port);
    } else {
        printf("...The default %d port will be used\n", port);
    }

    if ((listen_file_dirictory = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Socket inst.");
        exit(1);
    }

    bzero(&server_address, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htons(INADDR_ANY);
    server_address.sin_port = htons(port);


    int yeah = 1;
    if (setsockopt(listen_file_dirictory, SOL_SOCKET, SO_REUSEADDR, &yeah, sizeof(int)) == -1) {
        perror("Setsockopt");
        exit(1);
    }

    if (bind(listen_file_dirictory, (struct sockaddr *) &server_address, sizeof(server_address)) == -1) {
        perror("Bind");
        close(listen_file_dirictory);
        exit(1);
    }

    if (listen(listen_file_dirictory, 1) == -1) {
        perror("Listen");
        close(listen_file_dirictory);
        exit(1);
    }

    printf("...Server is up and pending\n");

    int efd = epoll_create(100);
    //printf("C1\n");
    // Добавляем дескриптор в массив ожидания
    struct epoll_event listenev;
    listenev.events = EPOLLIN | EPOLLPRI | EPOLLET;
    listenev.data.fd = listen_file_dirictory;
    if (epoll_ctl(efd, EPOLL_CTL_ADD, listen_file_dirictory, &listenev) < 0) {
        perror("Epoll fd add");
        return -1;
    }
    //printf("C2\n");
    socklen_t client;
    struct epoll_event events[100];
    struct epoll_event connev;
    struct sockaddr_in cliaddr;

    //printf("C3\n");
    while (1) {
        //printf("EE1\n");
        int nfds = epoll_wait(efd, events, 100, -1);

        for (int n = 0; n < nfds; ++n) {
            //printf("EE2\n");
            if (events[n].data.fd == listen_file_dirictory) {
                //printf("GOGOGOGO\n");
                client = sizeof(cliaddr);
                int connfd = accept(listen_file_dirictory, (struct sockaddr *) &cliaddr, &client);
                if (connfd < 0) {
                    perror("accept");
                    continue;
                }

                setnonblocking(connfd);
                connev.data.fd = connfd;
                connev.events = EPOLLIN | EPOLLOUT ;
                if ((!epoll_ctl(efd, EPOLL_CTL_ADD, connfd, &connev)) < 0) {
                    perror("Epoll fd add");
                    close(connfd);
                    continue;
                }

            } else {
                int fd = events[n].data.fd;

                if (events[n].events & EPOLLIN) {
                    //printf("GO1\n");

                    char reqline[1000];

                    bzero(reqline, 1000);

                    if (get_safe(fd, reqline)) {
                        perror("Read");
                        break;
                    } else {
                        printf("New message: %s", reqline);
                    }

                    char response[1000];
                    bzero(response, 1000);
                    sprintf(response, "Hello, %s", reqline);

                    if (send_safe(fd, response, strlen(response))) {
                        perror("Write");
                    } else {
                        printf("Response sent: %s", response);
                    }

                    epoll_ctl(efd, EPOLL_CTL_DEL, fd, &connev);
                    close(fd);
                }

                if (events[n].events & EPOLLOUT) {
                    //printf("GO2\n");
                }
            }
        }
    }
}


/*
 * UTILS
 *
 * */

//remove spaces and \n
void rm_spaces(void *src) {
    char *from = src;
    char *to = src;

    while (*from) {
        if (*from != ' ' && *from != '\n') {
            *to = *from;
            to++;
        }
        from++;
    }
    *to = '\0';
}

void rm_slash_n(void *src) {
    char *from = src;
    char *to = src;

    while (*from) {
        if (*from != '\n') {
            *to = *from;
            to++;
        }
        from++;
    }
    *to = '\0';
}


int send_safe(int socket, void *buffer, size_t length) {
    char *ptr = (char *) buffer;
    while (length > 0) {
        int i = send(socket, ptr, length, 0);
        if (i < 1) {
            return 1;
        }
        ptr += i;
        length -= i;
    }
    return 0;
}

int get_safe(int socket_fd, char *message) {
    char data[100];
    ssize_t data_read;

    while ((data_read = recv(socket_fd, data, 100, 0)) > 0) {
        for (int i = 0; i < data_read; i++) {
            *message++ = data[i];
        }
        if (data[data_read - 1] == '\n') {
            break;
        }
        bzero(data, 100);
    }

    if (data_read == -1 && errno != 0) {
        return 1;
    }

    *message = '\0';

    return 0;
}