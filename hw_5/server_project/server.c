#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <errno.h>

void *connection_handler(void *socket_desc);

int send_safe(int socket, void *buffer, size_t length);

int get_safe(int socket_fd, char *message);

void itoa(int n, char s[]);

void read_local_file(char *uri, char *body, char *response);

void rm_slash_n(void *src);

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

struct headers make_headers(char *content_legth) {
    time_t current_time;
    char *c_time_string;
    current_time = time(NULL);
    c_time_string = ctime(&current_time);
    rm_slash_n(c_time_string);

    return (struct headers) {"POST, GET, HEAD", NULL, NULL, content_legth, "text/html", c_time_string, NULL,
                             "kirill.vakhrushev@gmail.com", NULL, NULL, NULL, NULL, "127.0.0.1", "Linux",
                             "Linux / myServer", NULL, NULL};
}

struct request_line {
    char *method;
    char *uri;
    char *protocol;
};

struct request_line make_request_line(char *m, char *u, char *p) {
    return (struct request_line) {m, u, p};
}

struct response_line {
    char *protocol;
    char *code;
};

struct response_line make_response_line(char *p, char *c) {
    return (struct response_line) {p, c};
}

struct HTTPresponse {
    struct response_line Response_line;
    struct headers Message_Headers;
    char *Entity_Body;
};

struct HTTPresponse make_HTTPresponse(struct response_line response_line, struct headers hdrs, char *entity_body) {
    return (struct HTTPresponse) {response_line, hdrs, entity_body};
}


struct HTTPquery {
    struct request_line Request_Line;
    struct headers Message_Headers;
    char *Entity_Body;
};

struct HTTPquery make_HTTPquery(struct request_line request_line, struct headers hdrs, char *entity_body) {
    return (struct HTTPquery) {request_line, hdrs, entity_body};
}

void struct_HTTPresponse_to_string(char *query, struct HTTPresponse http_query) {
    //char query[2000];
    sprintf(query, "%s %s\n", http_query.Response_line.protocol, http_query.Response_line.code);

    if (http_query.Message_Headers.Host != NULL)
        sprintf(query, "%sHost: %s\n", query, http_query.Message_Headers.Host);

    if (http_query.Message_Headers.Allow != NULL)
        sprintf(query, "%sAllow: %s\n", query, http_query.Message_Headers.Allow);

    if (http_query.Message_Headers.Authorization != NULL)
        sprintf(query, "%sAuthorization: %s\n", query, http_query.Message_Headers.Authorization);

    if (http_query.Message_Headers.Content_Encoding != NULL)
        sprintf(query, "%sContent_Encoding: %s\n", query, http_query.Message_Headers.Content_Encoding);

    if (http_query.Message_Headers.Content_Length != NULL)
        sprintf(query, "%sContent_Length: %s\n", query, http_query.Message_Headers.Content_Length);

    if (http_query.Message_Headers.Content_Type != NULL)
        sprintf(query, "%sContent_Type: %s\n", query, http_query.Message_Headers.Content_Type);

    if (http_query.Message_Headers.Date != NULL)
        sprintf(query, "%sDate: %s\n", query, http_query.Message_Headers.Date);

    if (http_query.Message_Headers.Expires != NULL)
        sprintf(query, "%sExpires: %s\n", query, http_query.Message_Headers.Expires);

    if (http_query.Message_Headers.From != NULL)
        sprintf(query, "%sFrom: %s\n", query, http_query.Message_Headers.From);

    if (http_query.Message_Headers.Expires != NULL)
        sprintf(query, "%sExpires: %s\n", query, http_query.Message_Headers.Expires);

    if (http_query.Message_Headers.If_Modified_Since != NULL)
        sprintf(query, "%sIf_Modified_Since: %s\n", query, http_query.Message_Headers.If_Modified_Since);

    if (http_query.Message_Headers.Last_Modified != NULL)
        sprintf(query, "%sLast_Modified: %s\n", query, http_query.Message_Headers.Last_Modified);

    if (http_query.Message_Headers.Location != NULL)
        sprintf(query, "%sLocation: %s\n", query, http_query.Message_Headers.Location);

    if (http_query.Message_Headers.Pragma != NULL)
        sprintf(query, "%sPragma: %s\n", query, http_query.Message_Headers.Pragma);

    if (http_query.Message_Headers.Referer != NULL)
        sprintf(query, "%sReferer: %s\n", query, http_query.Message_Headers.Referer);

    if (http_query.Message_Headers.Server != NULL)
        sprintf(query, "%sServer: %s\n", query, http_query.Message_Headers.Server);

    if (http_query.Message_Headers.User_Agent != NULL)
        sprintf(query, "%sUser_Agent: %s\n", query, http_query.Message_Headers.User_Agent);

    if (http_query.Message_Headers.WWW_Authenticate != NULL)
        sprintf(query, "%sWWW_Authenticate: %s\n", query, http_query.Message_Headers.WWW_Authenticate);

    if (http_query.Entity_Body != NULL) {  //&& strlen(http_query.Entity_Body) > 0
        sprintf(query, "%s\n%s", query, http_query.Entity_Body);
    }

    sprintf(query, "%s\t", query);
}

void parse_and_process_query(char *q, char *final_response) {
    struct request_line structrequest_line = {};
    struct headers structheaders;
    struct HTTPquery structHTTPquery;

    char method[100];
    char uri[100];
    char protocol[100];

    char Allow[200];
    char Authorization[200];
    char Content_Encoding[200];
    char Content_Length[200];
    char Content_Type[200];
    char Date[200];
    char Expires[200];
    char From[200];
    char If_Modified_Since[200];
    char Last_Modified[200];
    char Location[200];
    char Pragma[200];
    char Referer[200];
    char Server[200];
    char User_Agent[200];
    char WWW_Authenticate[200];
    char Host[200];

    char Entity_Body[200];
    sprintf(Entity_Body," ");
    char *start_part;
    char *saveptr;
    int flag = 0;
    char code[100] = "none";
    start_part = strtok_r(q, "\n", &saveptr);
    while (start_part != NULL) {

        if (flag != 0) {
            start_part = strtok_r(NULL, "\n", &saveptr);
        }

        char part[500];
        sprintf(part, "%s", start_part);

        /*
         * PARSE ENTITY BODY
         * */
        if (flag != 0 && strstr(part, ": ") == NULL) {
            sprintf(Entity_Body, "%s", part);
            break;
        }

        /*
         * PARSE REQUEST LINE
         * */
        if (flag == 0) {
            char *start;
            char *ptr;
            start = strtok_r(part, " ", &ptr);

            if (start != NULL) {
                sprintf(method, "%s", start);
                start = strtok_r(NULL, " ", &ptr);
            } else {
                sprintf(code, "%s", "400 Bad Request");
            }

            if (start != NULL) {
                sprintf(uri, "%s", start);
                start = strtok_r(NULL, " ", &ptr);
            } else {
                sprintf(code, "%s", "400 Bad Request");
            }

            if (start != NULL) {
                sprintf(protocol, "%s", start);
            } else {
                sprintf(code, "%s", "400 Bad Request");
            }
            flag = 1;
            continue;
        }

        /*
         * PARSE HEADERS
         * */
        if (flag == 1) {
            char *pos_name_method;
            char name_method[100];
            char method_body[100];

            bzero(name_method, 100);
            bzero(method_body, 100);

            if ((pos_name_method = strstr(part, ": ")) == NULL) {
                sprintf(code, "%s", "400 Bad Request");
                continue;
            }

            size_t len_name_method = pos_name_method - part;

            if (strncpy(name_method, part, len_name_method) == NULL) {
                sprintf(code, "%s", "400 Bad Request");
                continue;
            }
            name_method[len_name_method] = '\0';

            if (strcpy(method_body, pos_name_method + 2) == NULL) {
                sprintf(code, "%s", "400 Bad Request");
                continue;
            }

            if (strcmp(name_method, "Allow") == 0) {
                sprintf(Allow, "%s", method_body);
            }
            if (strcmp(name_method, "Authorization") == 0) {
                sprintf(Authorization, "%s", method_body);
            }
            if (strcmp(name_method, "Content_Encoding") == 0) {
                sprintf(Content_Encoding, "%s", method_body);
            }
            if (strcmp(name_method, "Content_Length") == 0) {
                sprintf(Content_Length, "%s", method_body);
            }
            if (strcmp(name_method, "Content_Type") == 0) {
                sprintf(Content_Type, "%s", method_body);
            }
            if (strcmp(name_method, "Date") == 0) {
                sprintf(Date, "%s", method_body);
            }
            if (strcmp(name_method, "From") == 0) {
                sprintf(From, "%s", method_body);
            }
            if (strcmp(name_method, "If_Modified_Since") == 0) {
                sprintf(If_Modified_Since, "%s", method_body);
            }
            if (strcmp(name_method, "Last_Modified") == 0) {
                sprintf(Last_Modified, "%s", method_body);
            }
            if (strcmp(name_method, "Location") == 0) {
                sprintf(Location, "%s", method_body);
            }
            if (strcmp(name_method, "Pragma") == 0) {
                sprintf(Pragma, "%s", method_body);
            }
            if (strcmp(name_method, "Referer") == 0) {
                sprintf(Referer, "%s", method_body);
            }
            if (strcmp(name_method, "Server") == 0) {
                sprintf(Server, "%s", method_body);
            }
            if (strcmp(name_method, "User_Agent") == 0) {
                sprintf(User_Agent, "%s", method_body);
            }
            if (strcmp(name_method, "WWW_Authenticate") == 0) {
                sprintf(WWW_Authenticate, "%s", method_body);
            }
            if (strcmp(name_method, "Host") == 0) {
                sprintf(Host, "%s", method_body);
            }
        }
    }


    structrequest_line = make_request_line(method, uri, protocol);
    structheaders = (struct headers) {Allow, Authorization, Content_Encoding, Content_Length, Content_Type, Date,
                                      Expires, From, If_Modified_Since, Last_Modified,
                                      Location, Pragma, Referer, Server, User_Agent, WWW_Authenticate, Host};
    structHTTPquery = make_HTTPquery(structrequest_line, structheaders, Entity_Body);

    printf("...the request was successfully parsed\n");


    if (strlen(structHTTPquery.Request_Line.uri) > 100) {
        sprintf(code, "%s", "400 Bad Request");
    }

    if (strcmp(structHTTPquery.Request_Line.method, "DELETE") == 0 ||
        strcmp(structHTTPquery.Request_Line.method, "PUT") == 0) {
        sprintf(code, "%s", "405 Method Not Allowed ");
    } else if (strcmp(structHTTPquery.Request_Line.method, "GET") != 0 &&
               strcmp(structHTTPquery.Request_Line.method, "POST") != 0 &&
               strcmp(structHTTPquery.Request_Line.method, "HEAD") != 0) {
        sprintf(code, "%s", "501 Not Implemented");
    }

    if (strcmp(code, "none") != 0) {
        struct HTTPresponse http_response;
        http_response = make_HTTPresponse(make_response_line("HTTP/1.0", code), make_headers(0), "");
        char response[3000];
        struct_HTTPresponse_to_string(response, http_response);

        sprintf(final_response, "%s", response);
    }

    //printf("PR:\n-%s-%s-%s-\n", structHTTPquery.Request_Line.method, structHTTPquery.Request_Line.protocol, structHTTPquery.Request_Line.protocol);
    struct HTTPresponse http_response;

    if (strcmp(structHTTPquery.Request_Line.method, "GET") == 0) {
        char uri_text[200];
        char *pos_name_method;
        char name_method[100];
        char method_body[100];

        bzero(name_method, 100);
        bzero(method_body, 100);
        bzero(uri_text, 100);

        char body[1000];
        char code[1000];

        pos_name_method = strstr(structHTTPquery.Request_Line.uri, "?");

        if (pos_name_method != NULL){
            size_t len_name_method = pos_name_method - structHTTPquery.Request_Line.uri;
            strncpy(name_method, structHTTPquery.Request_Line.uri, len_name_method);
            name_method[len_name_method] = '\0';
            read_local_file(name_method, body, code);
        } else{
            read_local_file(structHTTPquery.Request_Line.uri, body, code);
        }


        char number[200];
        itoa(strlen(body), number);
        http_response = make_HTTPresponse(make_response_line("HTTP/1.0", code), make_headers(number), body);
    }
    if (strcmp(structHTTPquery.Request_Line.method, "POST") == 0) {
        char body[1000];
        char code[1000];
        read_local_file(structHTTPquery.Request_Line.uri, body, code);

        char number[200];
        itoa(strlen(body), number);
        http_response = make_HTTPresponse(make_response_line("HTTP/1.0", code), make_headers(number), body);

        //printf("i,m post\n%s\n%s\n",code,body);
    }
    if (strcmp(structHTTPquery.Request_Line.method, "HEAD") == 0) {
        char uri_text[200];       char *pos_name_method;
        char name_method[100];
        char method_body[100];

        bzero(name_method, 100);
        bzero(method_body, 100);
        bzero(uri_text, 100);

        char body[1000];
        char code[1000];

        pos_name_method = strstr(structHTTPquery.Request_Line.uri, "?");
        if (pos_name_method != NULL){
            size_t len_name_method = pos_name_method - structHTTPquery.Request_Line.uri;
            strncpy(name_method, structHTTPquery.Request_Line.uri, len_name_method);
            name_method[len_name_method] = '\0';
            read_local_file(name_method, body, code);
        } else{
            read_local_file(structHTTPquery.Request_Line.uri, body, code);
        }


        char number[200];
        itoa(strlen(body), number);
        http_response = make_HTTPresponse(make_response_line("HTTP/1.0", code), make_headers(0), "");
    }

    char response[3000];
    struct_HTTPresponse_to_string(response, http_response);

    sprintf(final_response, "%s", response);
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

    listen_file_dirictory = socket(AF_INET, SOCK_STREAM, 0);

    bzero(&server_address, sizeof(server_address));

    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htons(INADDR_ANY);
    server_address.sin_port = htons(port);

    bind(listen_file_dirictory, (struct sockaddr *) &server_address, sizeof(server_address));
    listen(listen_file_dirictory, 10);

    printf("...Server is up and pending\n");

    pthread_t thread_id;
    int client_sock;

    while ((client_sock = accept(listen_file_dirictory, (struct sockaddr *) NULL, NULL))) {
        int *fd = malloc(sizeof(int));
        (*fd) = client_sock;
        puts("new connection accepted");

        if (pthread_create(&thread_id, NULL, connection_handler, (void *) fd) < 0) {
            perror("ERROR: could not create thread");
            return 1;
        }
        //pthread_join( thread_id , NULL);
        puts("handler assigned");
    }

    if (client_sock < 0) {
        perror("ERROR: accept failed");
        return 1;
    }

    return 0;
}

void *connection_handler(void *socket_desc) {
    char str[3000];
    //strcpy(str, "");
    int communicate_file_dirictory = *(int *) socket_desc;

    while (1) {
        bzero(str, 3000);

        if (get_safe(communicate_file_dirictory, str)) {
            perror("error with geting request\n");
            break;
        } else {
            printf("...new query:\n\x1b[32m%s\x1b[0m\n", str);
        }

        printf("...started processing query\n");

        char response[3000];
        bzero(response, 3000);
        parse_and_process_query(str, response);

        if (send_safe(communicate_file_dirictory, response, strlen(response))) {
            perror("request was not sent, try again\n");
            break;
        } else {
            printf("...response successfully sent\n");
        }

    }

    close(communicate_file_dirictory);
    free(socket_desc);
    return 0;
}

/*
 * UTILS
 *
 * */

void read_local_file(char *uri, char *body, char *code) {

    FILE *mf;
    char str[1000], *estr;
    strcpy(body, " ");
    strcpy(code, "200 OK\0");

    mf = fopen(uri, "r");

    if (mf == NULL) {
        strcpy(code, "404 Not Found\0");
        return;
    }

    while (1) {
        estr = fgets(str, sizeof(str), mf);

        if (estr == NULL) {
            if (feof(mf) != 0) {
                break;
            } else {
                strcpy(code, "520 Unknown Error \0");
                return;
            }
        }
        strcat(body, str);
    }

    if (fclose(mf) == EOF) {
        perror("file closing error\n");
    }
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

void reverse(char s[]) {
    int i, j;
    char c;

    for (i = 0, j = strlen(s) - 1; i < j; i++, j--) {
        c = s[i];
        s[i] = s[j];
        s[j] = c;
    }
}

void itoa(int n, char s[]) {
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