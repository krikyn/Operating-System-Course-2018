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


void rm_spaces(void *src);

void rm_slash_n(void *src);

int send_safe(int socket, void *buffer, size_t length);

int get_safe(int socket_fd, char *message);


void set_non_block(int fd)
{
    int flag = fcntl ( fd, F_GETFL, 0 );
    fcntl ( fd, F_SETFL, flag | O_NONBLOCK );
}

int main(int argc, char** argv)
{

    if (argc < 2 || (argc > 1 && strcmp(argv[1], "-help")) == 0) {
        printf("Usage: ./client_project message [host [port]](optional)\n");
        return 0;
    }

    char message[1000];

    sprintf(message, "%s",argv[1]);
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


    int sk = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);


    struct sockaddr_in sa = {0};
    sa.sin_family = AF_INET;
    sa.sin_port = htons(port);

    struct sockaddr_in *psa = &sa;

    inet_pton(AF_INET, host, &psa->sin_addr.s_addr);

    if(connect(sk, (struct sockaddr*)&sa, sizeof(sa)) < 0)
    {
        printf("Connection failed");
        close(sk);
        return 0;
    }

    set_non_block(sk);

    int efd;
    efd = epoll_create(10);
    if(efd == -1)
    {
        perror("Error with epoll create");
        exit(1);
    }

    struct epoll_event event;
    struct epoll_event events[10];
    event.events = EPOLLOUT | EPOLLIN | EPOLLET;
    event.data.fd = sk;
    epoll_ctl(efd, EPOLL_CTL_ADD, sk, &event);


    int loop = 0;
    int epollout = 0;
    int epollin = 0;
    while(1)
    {
        char buffer[1000];
        int n = 5;

        if(loop == 1)
        {
            break;
        }


        n = epoll_wait(efd, events, 10, -1);
        //printf("%d\n", n);

        for(int i = 0; i < n; i++)
        {
            if((events[i].events & EPOLLOUT) && epollout==0)
            {
                snprintf(buffer, 1000, "%s\n", message);

                int n = strlen(buffer);
                int nsend = 0;

                while(n > 0)
                {
                    nsend = write(events[i].data.fd, buffer + nsend, n);
                    if(nsend < 0 && errno != EAGAIN)
                    {
                        close(events[i].data.fd);
                        return 0;
                    }
                    n -= nsend;
                }
                printf("Message sent: %s", buffer);
                epollout = 1;
            }

            if((events[i].events & EPOLLIN) && epollin == 0)
            {
                bzero(buffer,1000);

                int n = 0;
                int nrecv = 0;

                while(1){
                    nrecv = read(events[i].data.fd, buffer + n, 999) ;
                    if(nrecv == -1 && errno != EAGAIN)
                    {
                        perror("read error!");
                    }
                    if((nrecv == -1 && errno == EAGAIN) || nrecv == 0)
                    {
                        break;
                    }
                    n += nrecv;
                }
                loop=1;
                epollin = 1;
                printf("Response: %s\n", buffer);
            }
        }
    }
    close(sk);
    close(efd);
    return 0;
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
    return;
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
    return;
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
        if (data[data_read - 1] == '\t') {
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

