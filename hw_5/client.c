#include <arpa/inet.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include<string.h>
#include <stdlib.h>
 
int send_all(int socket, void *buffer, size_t length);

void read_message(int socket_fd, char *message);

void rm_spaces( void * src );

int main(int argc, char const *argv[])
{   

    if (argc > 1 && strcmp(argv[1],"-help") == 0){
        printf("Usage: ./client [host [port]](optional)\n");
        return 0;
    }

    int socket_file_dirictory;
    char sendline[100];
    char recvline[100];
    struct sockaddr_in server_address;

    char *host = "127.0.0.1";
    int port = 3030;

    printf("...Server initialization\n");

    if (argc > 3){
        printf("Wrong argument, usage: ./client [host] [port]{0001-9999}\n");
        return 0;
    } else {
        if (argc > 1){
            host = (char *)argv[1];
            printf("...Сonnecting to the specified host \"%s\"\n", host);
            if (argc > 2){
                port  = atoi(argv[2]);
                if (port < 1 || port > 9999){
                    printf("Wrong argument, usage: ./client [host] [port]{0001-9999}\n");
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
 
    socket_file_dirictory=socket(AF_INET,SOCK_STREAM,0);
    bzero(&server_address,sizeof server_address);
 
    server_address.sin_family=AF_INET;
    server_address.sin_port=htons(port);
    inet_pton(AF_INET,host,&(server_address.sin_addr));
 
    if (connect(socket_file_dirictory,(struct sockaddr *)&server_address,sizeof(server_address)) == -1){
        perror("Connection failed\n");
        exit(0);
    }
    
    printf("...Connection established\n");
    printf("-------------------------\n");
    printf("This client allows to send queries to server that supported truncated http protocol. For example send the \"localhost/dir1/text1.text\" and you will receive the contents of the file with the name \"text1\" in the \"dir1\" dirictory.\n");
    printf("Example host: 127.0.0.1\nDirictories and files:\n-dir1\n--text1.txt\n--text2.html\n-dir2\n--text1.cvc\n\n");
    printf("Send query$>");

    while(1)
    {
        bzero( sendline, 100);
        bzero( recvline, 100);
        fgets(sendline,100,stdin);

        rm_spaces(sendline);
        if (strcmp(sendline, "\n") == 0 || strlen(sendline) == 0){
            printf("usage: hostname/[path/../file.x]<- optional\n");
            printf("Send query$>");
            continue;
        }

    	if(strcmp("exit\n", sendline)==0) {
    		close(socket_file_dirictory);
    		exit(0);
    	}

        char host_part[100], dir_part[100];
        strcpy(host_part, "");
        strcpy(dir_part, "");

        //http://localhost/dir1/text1.text
        int pointer = 0;
        
        while(pointer+1 < strlen(sendline)-1 && sendline[pointer+1] != '/'){
            pointer++;
        }

        
        /*if (pointer+2 < strlen(sendline)-1 && sendline[pointer] == ':' && sendline[pointer + 2] == '/'){
            int pointer2 = pointer + 3;
            while(pointer2+1 < strlen(sendline)-1 && sendline[pointer2+1] != '/'){
                pointer2++;
            }
            strncpy(host_part, sendline + pointer + 3, pointer2-pointer-2);
            host_part[pointer2-pointer-2] = '\0';
            strncpy(dir_part, sendline + pointer2+2, strlen(sendline) - pointer2 - 3);
        }else {*/
            
            if (pointer == strlen(sendline)-2){
                //strncpy(host_part, sendline, strlen(sendline));
                //host_part[strlen(sendline)] = '\0';
                printf("usage: hostname/[path/../file.x](optional)\n");
                printf("Send query$>");
                continue;
            } else {
                strncpy(host_part, sendline, pointer+1);
                host_part[pointer+1] = '\0';
                strncpy(dir_part, sendline + pointer+2, strlen(sendline) - pointer - 1); 
            }
        //}

        char query[100];
        sprintf(query,"GET %s HTTP/1.0 Host: %s", host_part, dir_part);

        send_all(socket_file_dirictory, &query, strlen(query));
        printf("...request sent, waiting for reply\n");
        
        bzero( recvline, 100);
        //read_message(socket_file_dirictory, (void *)&recvline);
	    recv(socket_file_dirictory, recvline, 100, 0);

        printf("...server response: %s\n", recvline);
        //printf("%s",recvline);

        printf("\nSend query$>");
    }
    return 0;
 
}

void read_message(int socket_fd, char *message) {
    char data;
    ssize_t data_read;
 
    while ((data_read = recv(socket_fd, &data, 1, 0)) > 0 && data != '\n')
        {   
            //printf("^^^ %c          -      %li\n", data, data_read);
            *message++ = data;
        }
    
    //printf("FINISH %c          -      %li\n", data, data_read);

    if (data_read == -1) {
        perror("ERROR recv()");
        exit(EXIT_FAILURE);
    }
 
    *message = '\0';
}

int send_all(int socket, void *buffer, size_t length)
{
    char *ptr = (char*) buffer;
    while (length > 0)
    {
        int i = send(socket, ptr, length, 0);
        if (i < 1) return 0;
        ptr += i;
        length -= i;
    }
    return 1;
}

void rm_spaces( void * src )
{
    char * from = src;
    char * to = src;

    while( *from )
    {
        if( *from != ' ' )
        {
            *to = *from;
            to++;
        }
        from++;
    }
    *to = '\0';
    return;
}