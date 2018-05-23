#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <printf.h>
#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>

int main(int argc, char **argv) {

    if (argc < 2 || (argc > 1 && strcmp(argv[1], "-help")) == 0) {
        printf("Usage: ./client_project message [host [port]](optional)\n");
        return 0;
    }

    char message[1000];

    sprintf(message, "%s", argv[1]);
    char *host = "127.0.0.1";
    int port = 3030;

    printf("...Server initialization\n");

    if (argc > 4) {
        printf("Wrong argument, usage: ./client_project [message] [host] [port]{0001-9999}\n");
        return 0;
    } else {
        if (argc > 2) {
            host = (char *) argv[2];
            printf("...Сonnecting to the specified host \"%s\"\n", host);
            if (argc > 3) {
                port = atoi(argv[3]);
                if (port < 1 || port > 9999) {
                    printf("Wrong argument, usage: ./client_project [host] [port]{0001-9999}\n");
                    exit(0);
                }
                printf("...The specified %d port will be used\n", port);
            } else {
                printf("...The default %d port will be used\n", port);
            }
        } else {
            printf("...Сonnecting to the default host \"%s\"\n", host);
            printf("...The default %d port will be used\n", port);
        }

    }

    int fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, IPPROTO_TCP);
    if (fd == -1) {
        perror("create socket");
        return 1;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_aton(host, &addr.sin_addr);

    bool connected = true;
    int i = connect(fd, (struct sockaddr *) &addr, sizeof(struct sockaddr_in));

    if ((i == -1) && (errno == EINPROGRESS)) {
        connected = false;
    }

    struct epoll_event event;
    int epollfd;
    epollfd = epoll_create(10);

    event.events = EPOLLIN;
    if (!connected) {
        event.events |= EPOLLOUT;
    }
    event.data.fd = fd;
    int r = epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
    if (r == -1) {
        perror("epoll 1");
        close(epollfd);
        close(fd);
        return 1;
    }

    int loop = 0;
    int epollout = 0;
    int epollin = 0;

    while (1) {
        struct epoll_event events[10];
        int n = epoll_wait(epollfd, events, 10, -1);
        if (n == -1) {
            close(epollfd);
            close(fd);
            return 1;
        }

        char buffer[1000];

        if (loop == 1) {
            break;
        }


        for (int i = 0; i < n; i++) {

            if (events[i].data.fd == fd) {
                if (events[i].events & EPOLLOUT) {
                    if (!connected) {
                        int err = 0;
                        socklen_t len = sizeof(int);
                        int gi = getsockopt(fd, SOL_SOCKET, SO_ERROR, &err, &len);
                        if (gi != -1) {
                            if (err == 0) {
                                struct epoll_event event1;
                                event1.events = EPOLLIN | EPOLLOUT;
                                event1.data.fd = fd; // user data
                                int nn = epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &event1);
                                if (nn == -1) {
                                    perror("epoll 2");
                                    close(epollfd);
                                    close(fd);
                                    return 1;
                                }
                            } else {
                                struct epoll_event event1;
                                event1.events = 0;
                                event1.data.fd = fd;
                                epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, &event1);
                                close(fd);
                            }
                        }
                    }
                }
            }

            if ((events[i].events & EPOLLOUT) && epollout == 0) {
                snprintf(buffer, 1000, "%s\n", message);

                int n = strlen(buffer);
                int nsend = 0;

                while (n > 0) {
                    nsend = write(events[i].data.fd, buffer + nsend, n);
                    if (nsend < 0 && errno != EAGAIN) {
                        close(events[i].data.fd);
                        return 0;
                    }
                    n -= nsend;
                }
                printf("Message sent: %s", buffer);
                epollout = 1;
            }

            if ((events[i].events & EPOLLIN) && epollin == 0) {
                bzero(buffer, 1000);

                int n = 0;
                int nrecv = 0;

                while (1) {
                    nrecv = read(events[i].data.fd, buffer + n, 999);
                    if (nrecv == -1 && errno != EAGAIN) {
                        perror("read error!");
                    }
                    if ((nrecv == -1 && errno == EAGAIN) || nrecv == 0) {
                        break;
                    }
                    n += nrecv;
                }
                loop = 1;
                epollin = 1;
                printf("Response: %s\n", buffer);
            }
        }
    }
    close(epollfd);
    close(fd);
    return 0;
}

