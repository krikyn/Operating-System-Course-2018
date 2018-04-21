#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>

void *connection_handler(void *socket_desc);

void read_message(int socket_fd, char *message);

int send_all(int socket, void *buffer, size_t length);

void read_local_file(char uri[100], char * body[50], int * isOk, char * response[50]);

int main(int argc,char **argv)
{
    if (argc > 1 && strcmp(argv[1],"-help") == 0){
        printf("Usage: ./server [port](optional)\n");
        return 0;
    }

    int listen_file_dirictory;
    struct sockaddr_in server_address;
    int port = 3030;

    printf("...Server initialization\n");

    if (argc > 1){
        port  = atoi(argv[1]);
        if (port < 1 || port > 9999){
            printf("Wrong argument, usage: ./server [port]{0001-9999}\n");
            exit(0);
        }
        printf("...The specified %d port will be used\n", port);
    } else {
        printf("...The default %d port will be used\n", port);
    }

    listen_file_dirictory = socket(AF_INET, SOCK_STREAM, 0);
 
    bzero( &server_address, sizeof(server_address));

    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htons(INADDR_ANY);
    server_address.sin_port = htons(port);
 
    bind(listen_file_dirictory, (struct sockaddr *) &server_address, sizeof(server_address));
    listen(listen_file_dirictory, 10);
    
    printf("...Server is up and pending\n");

    pthread_t thread_id;
    int client_sock;

    while( (client_sock = accept(listen_file_dirictory, (struct sockaddr*) NULL, NULL)) )
    {
        int * fd = malloc(sizeof(int));
        (*fd) = client_sock;
        puts("new connection accepted");
         
        if(pthread_create( &thread_id , NULL ,  connection_handler , (void*) fd) < 0)
        {
            perror("ERROR: could not create thread");
            return 1;
        }
        //pthread_join( thread_id , NULL);
        puts("handler assigned");
    }
     
    if (client_sock < 0)
    {
        perror("ERROR: accept failed");
        return 1;
    }

    return 0;
}

void *connection_handler(void *socket_desc)
{
    char str[100];
    strcpy( str, "");
    int communicate_file_dirictory = *(int*)socket_desc;
     
        while(1)
        {
            bzero( str, 100);
            read_message(communicate_file_dirictory, (void *)&str);
            printf("new query - \"%s\"\n", str);

            //GET %s HTTP/1.0 Host: %s

           //char str2 [10]="GET";
           char *istr1, *istr2, *istr3;
           istr1 = strstr (str,"GET");
           int isOk = 1;
           char body[50], report[50], response[100], dirfile[100];

           //Вывод результата поиска на консоль
           if ( istr1 == NULL){
              strcpy((char *)response, "HTTP/1.0 400 Bad Request\0"); 
              isOk = 0;
           } else {
                istr2 = strstr (str,"HTTP/1.0");
                if (istr2 == NULL){
                    strcpy((char *)response, "HTTP/1.0 505 HTTP Version Not Supported\0"); 
                    isOk = 0;
                } else {
                    istr3 = strstr(str,"Host:");
                    if (istr3 == NULL){
                        strcpy((char *)response, "HTTP/1.0 400 Bad Request\0"); 
                        isOk = 0;
                    } else {
                        strncpy(dirfile, istr3 + 6,100);
                    }
                }
           }

            //printf("HOST '%s' - '%s'\n", dirfile, report);

            ////////////////////////////////////////////////////////////////    
            
            if (isOk == 1){
            read_local_file(dirfile, (void *)body, &isOk, (void *)report);
            }
            
            if (isOk == 1){
                sprintf(response, "%s\n\n%s", report, body);
            } else {
                sprintf(response, "%s", report);
            }
            
            send(communicate_file_dirictory, response, 100, 0);
        }

        close(communicate_file_dirictory);
    
    free(socket_desc);
    return 0;
} 

void read_message(int socket_fd, char *message) {
    char data;
    ssize_t data_read;
 
    while ((data_read = recv(socket_fd, &data, 1, 0)) > 0 && data != '\n')
        *message++ = data;
      
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

void read_local_file(char uri[100], char * body[50], int * isOk, char * response[50])
{

    FILE *mf;
    char str[50], *estr;
    strcpy((char *)body, "");
    strcpy((char *)response, "HTTP/1.0 200 OK\0"); 

    mf = fopen (uri,"r");
   
    if (mf == NULL) {
     strcpy((char *)response, "HTTP/1.0 404 Not Found\0");
     isOk = 0;
     return;
    }

    while (1)
    {
        estr = fgets (str,sizeof(str),mf);
 
       if (estr == NULL)
        {
        if ( feof (mf) != 0)
            {  
                break;
            } else 
            {
                strcpy((char *)response, "HTTP/1.0 520 Unknown Error \0");
                isOk = 0;
                return;
            }
        }
        strcat((char *)body, str);
    }

    if ( fclose (mf) == EOF)
    printf ("LOG: file closing error\n");

   return;
}

/*char response[100];
            
            sprintf(response,"HTTP/1.0 200 OK%c", '\0');
            response[strlen(response)] = '\0';
            
            int report = send_all(communicate_file_dirictory, &response, strlen(response));
            if (report == 1){
                printf("Reply sent\n");
            } else {
                printf("ERROR: Reply doesn't sent\n");
            }*/

