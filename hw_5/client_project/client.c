#include <arpa/inet.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include<string.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>

void rm_spaces(void *src);

void rm_slash_n(void *src);

int send_safe(int socket, void *buffer, size_t length);

int get_safe(int socket_fd, char *message);

struct headers {
    char *Allow;
    char *Authorization;
    char *Content_Encoding;
    char *Content_Length;
    char *Content_Type;
    char *Date;
    char *Expires;
    char *From;
    char *If_Modified_Since;
    char *Last_Modified;
    char *Location;
    char *Pragma;
    char *Referer;
    char *Server;
    char *User_Agent;
    char *WWW_Authenticate;
    char *Host;
};

struct headers make_headers(char *content_legth, char *location, char *host) {
    time_t current_time;
    char *c_time_string;
    current_time = time(NULL);
    c_time_string = ctime(&current_time);
    rm_slash_n(c_time_string);

    return (struct headers) {"POST, GET, HEAD", NULL, NULL, content_legth, "text/html", c_time_string, NULL,
                             "kirill.vakhrushev@gmail.com", NULL, NULL, location, "no-cache", "localhost", "Linux",
                             "Linux / myClient", NULL, host};
}

struct request_line {
    char *method;
    char *uri;
    char *protocol;
};

struct request_line make_request_line(char *m, char *u) {
    return (struct request_line) {m, u, "HTTP/1.0"};
}

struct HTTPquery {
    struct request_line Request_Line;
    struct headers Message_Headers;
    char *Entity_Body;
};

struct HTTPquery make_HTTPquery(struct request_line request_line, struct headers hdrs, char *entity_body) {
    return (struct HTTPquery) {request_line, hdrs, entity_body};
}

void struct_HTTPquery_to_string(char *query, struct HTTPquery http_query){
    //char query[2000];
    sprintf(query,"%s %s %s\n",http_query.Request_Line.method,http_query.Request_Line.uri, http_query.Request_Line.protocol);

    if (http_query.Message_Headers.Host != NULL)
        sprintf(query,"%sHost: %s\n",query, http_query.Message_Headers.Host);

    if (http_query.Message_Headers.Allow != NULL)
        sprintf(query,"%sAllow: %s\n",query, http_query.Message_Headers.Allow);

    if (http_query.Message_Headers.Authorization != NULL)
        sprintf(query,"%sAuthorization: %s\n",query, http_query.Message_Headers.Authorization);

    if (http_query.Message_Headers.Content_Encoding != NULL)
        sprintf(query,"%sContent_Encoding: %s\n",query, http_query.Message_Headers.Content_Encoding);

    if (http_query.Message_Headers.Content_Length != NULL)
        sprintf(query,"%sContent_Length: %s\n",query, http_query.Message_Headers.Content_Length);

    if (http_query.Message_Headers.Content_Type != NULL)
        sprintf(query,"%sContent_Type: %s\n",query, http_query.Message_Headers.Content_Type);

    if (http_query.Message_Headers.Date != NULL)
        sprintf(query,"%sDate: %s\n",query, http_query.Message_Headers.Date);

    if (http_query.Message_Headers.Expires != NULL)
        sprintf(query,"%sExpires: %s\n",query, http_query.Message_Headers.Expires);

    if (http_query.Message_Headers.From != NULL)
        sprintf(query,"%sFrom: %s\n",query, http_query.Message_Headers.From);

    if (http_query.Message_Headers.Expires != NULL)
        sprintf(query,"%sExpires: %s\n",query, http_query.Message_Headers.Expires);

    if (http_query.Message_Headers.If_Modified_Since != NULL)
        sprintf(query,"%sIf_Modified_Since: %s\n",query, http_query.Message_Headers.If_Modified_Since);

    if (http_query.Message_Headers.Last_Modified != NULL)
        sprintf(query,"%sLast_Modified: %s\n",query, http_query.Message_Headers.Last_Modified);

    if (http_query.Message_Headers.Location != NULL)
        sprintf(query,"%sLocation: %s\n",query, http_query.Message_Headers.Location);

    if (http_query.Message_Headers.Pragma != NULL)
        sprintf(query,"%sPragma: %s\n",query, http_query.Message_Headers.Pragma);

    if (http_query.Message_Headers.Referer != NULL)
        sprintf(query,"%sReferer: %s\n",query, http_query.Message_Headers.Referer);

    if (http_query.Message_Headers.Server != NULL)
        sprintf(query,"%sServer: %s\n",query, http_query.Message_Headers.Server);

    if (http_query.Message_Headers.User_Agent != NULL)
        sprintf(query,"%sUser_Agent: %s\n",query, http_query.Message_Headers.User_Agent);

    if (http_query.Message_Headers.WWW_Authenticate != NULL)
        sprintf(query,"%sWWW_Authenticate: %s\n",query, http_query.Message_Headers.WWW_Authenticate);

    if (strlen(http_query.Entity_Body) > 0){
        sprintf(query,"%s\n%s",query, http_query.Entity_Body);
    }

    sprintf(query,"%s\t",query);
}

void print_usage() {
    printf("wrong query, usage: \"[get|post|head] http://{hostname}/{path to file}(?{var1}={value1}&{var2}={value2}...)\" \n");
}

void reverse(char s[])
{
    int i, j;
    char c;

    for (i = 0, j = strlen(s)-1; i<j; i++, j--) {
        c = s[i];
        s[i] = s[j];
        s[j] = c;
    }
}

void itoa(int n, char s[])
{
    int i, sign;

    if ((sign = n) < 0)
        n = -n;
    i = 0;
    do {
        s[i++] = n % 10 + '0';
    } while ((n /= 10) > 0);
    if (sign < 0)
        s[i++] = '-';
    s[i] = '\0';
    reverse(s);
}

int main(int argc, char const *argv[]) {

    if (argc > 1 && strcmp(argv[1], "-help") == 0) {
        printf("Usage: ./client_project [host [port]](optional)\n");
        return 0;
    }

    int socket_file_dirictory;
    struct sockaddr_in server_address;

    char *host = "127.0.0.1";
    int port = 3030;

    printf("...Server initialization\n");

    if (argc > 3) {
        printf("Wrong argument, usage: ./client_project [host] [port]{0001-9999}\n");
        return 0;
    } else {
        if (argc > 1) {
            host = (char *) argv[1];
            printf("...Сonnecting to the specified host \"%s\"\n", host);
            if (argc > 2) {
                port = atoi(argv[2]);
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

    socket_file_dirictory = socket(AF_INET, SOCK_STREAM, 0);
    bzero(&server_address, sizeof server_address);

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    inet_pton(AF_INET, host, &(server_address.sin_addr));

    if (connect(socket_file_dirictory, (struct sockaddr *) &server_address, sizeof(server_address)) == -1) {
        perror("Connection failed\n");
        exit(0);
    }

    printf("...Connection established\n");
    printf("-------------------------\n");
    printf("This client_project allows to send queries to server that supported truncated http protocol. For example send the \"http://localhost/dir1/text1.txt?var1=value1&var2=3\" and you will receive the contents of the file with the name \"text1\" in the \"dir1\" dirictory.\n");
    printf("Example host: 127.0.0.1\nDirictories and files:\n-dir1\n--text1.txt\n--text2.html\n-dir2\n--text1.cvc\n\n");

    while (1) {
        printf("Send query$>"); //get http://google.com/text.txt?var1=value1

        char query[200];
        bzero(query, 200);

        if (fgets(query, 200, stdin) == NULL) {
            perror("input error, try again");
            continue;
        }

        if (strstr(query, "http://") == NULL) {
            print_usage();
            continue;
        }

        char *pos_name_method;
        char name_method[100];
        char method_body[100];

        bzero(name_method, 100);
        bzero(method_body, 100);

        if ((pos_name_method = strstr(query, " ")) == NULL) {
            print_usage();
            continue;
        }

        size_t len_name_method = pos_name_method - query;

        if (strncpy(name_method, query, len_name_method) == NULL) {
            print_usage();
            continue;
        }
        name_method[len_name_method] = '\0';

        if (strcpy(method_body, pos_name_method + 1) == NULL) {
            print_usage();
            continue;
        }

        rm_spaces(method_body);

        char query_host[100];
        char query_uri[100];
        char query_content[100];
        char query_uri_and_other[100];
        char query_host_and_other[100];

        bzero(query_host, 100);
        bzero(query_uri, 100);
        bzero(query_content, 100);
        bzero(query_uri_and_other, 100);
        bzero(query_host_and_other, 100);

        if (strcpy(query_host_and_other, method_body + 7) == NULL) {
            print_usage();
            continue;
        }

        char *pos_end_host;
        if ((pos_end_host = strstr(query_host_and_other, "/")) == NULL) {
            print_usage();
            continue;
        }

        size_t len_name_host = pos_end_host - query_host_and_other;
        if (strncpy(query_host, query_host_and_other, len_name_host) == NULL) {
            print_usage();
            continue;
        }
        query_host[len_name_host] = '\0';

        if (strcpy(query_uri_and_other, pos_end_host + 1) == NULL) {
            print_usage();
            continue;
        }

        char *pos_end_uri;
        if ((pos_end_uri = strstr(query_uri_and_other, "?")) == NULL) {

            pos_end_uri = query_uri_and_other + strlen(query_uri_and_other);

            size_t len_name_uri = pos_end_uri - query_uri_and_other;
            if (strncpy(query_uri, query_uri_and_other, len_name_uri) == NULL) {
                print_usage();
                continue;
            }

            query_content[0] = '\0';

        } else {

            size_t len_name_uri = pos_end_uri - query_uri_and_other;
            if (strncpy(query_uri, query_uri_and_other, len_name_uri) == NULL) {
                print_usage();
                continue;
            }

            query_uri[len_name_uri] = '\0';
            if (strcpy(query_content, pos_end_uri + 1) == NULL) {
                print_usage();
                continue;
            }
            rm_slash_n(query_content);
        }


        //printf("INFO _%s_%s_%s_%s", name_method, query_host, query_uri, query_content);

        struct request_line structrequest_line;
        struct headers structheaders;
        struct HTTPquery structHTTPquery;

        char location[200];
        sprintf(location, "%s/%s", query_host, query_uri);

        //choose method
        if (strcmp(name_method, "get") == 0) {
            char req_line[200];
            if (query_content!=NULL && strlen(query_content)>0){
                sprintf(req_line,"%s?%s",query_uri,query_content);
            } else {
                sprintf(req_line,"%s",query_uri);
            }
            structrequest_line = make_request_line("GET", req_line);
            structheaders = make_headers(0, location, query_host);
            structHTTPquery = make_HTTPquery(structrequest_line, structheaders, "");
        } else if (strcmp(name_method, "post") == 0) {
            structrequest_line = make_request_line("POST", query_uri);
            char number[200];
            itoa(strlen(query_content), number);
            structheaders = make_headers(number, location, query_host);
            structHTTPquery = make_HTTPquery(structrequest_line, structheaders, query_content);
        } else if (strcmp(name_method, "head") == 0) {
            char req_line[200];
            if (query_content!=NULL && strlen(query_content)>0){
                sprintf(req_line,"%s?%s",query_uri,query_content);
            } else {
                sprintf(req_line,"%s",query_uri);
            }
            structrequest_line = make_request_line("HEAD", req_line);
            structheaders = make_headers(0, location, query_host);
            structHTTPquery = make_HTTPquery(structrequest_line, structheaders, "");
        } else {
            printf("This method do not supported\n");
            continue;
        }

        char query_text[3000];
        struct_HTTPquery_to_string(query_text,structHTTPquery);
        //printf("GO:\n%s\n",query_text);
        //http://google.com/dir1/text1.txt?var1=value1&var2=3


        /*
         * SEND REQUEST
         *
         * */
        if (send_safe(socket_file_dirictory, query_text, strlen(query_text))) {
            perror("request was not sent, try again\n");
            break;
        } else {
            printf("...request sent, waiting for reply\n");
        }


        char recvline[3000];
        /*
         * GET RESPONSE
         *
         * */
        bzero(recvline, 3000);
        if (get_safe(socket_file_dirictory, recvline)) {
            perror("response was not received\n");
            break;
        } else {
            printf("...server response:\n\x1b[32m%s\x1b[0m\n", recvline);
        }
    }

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