#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>

int main(int argc, char const *argv[])
{
   FILE *mf;
   char str[50];
   char body[50];
   strcpy(body, "");
   char *estr;
   int isOk = 1;
   char response[50] = "HTTP/1.0 200 OK\0"; 

   mf = fopen (argv[1],"r");
   
   if (mf == NULL) {
   	strcpy(response, "HTTP/1.0 404 Not Found\0");
   	isOk = 0;
   	return 0;
   }

   while (1)
   {
      estr = fgets (str,sizeof(str),mf);

      if (estr == NULL)
      {
         if ( feof (mf) != 0)
         {  
            break;
         }
         else
         {
            strcpy(response, "520 Unknown Error \0");
   			isOk = 0;
   			return 0;
         }
      }
      strcat(body, str);
   }

   if ( fclose (mf) == EOF)
   	printf ("LOG: file closing error\n");

   return 0;
}

//gcc -Wall -fsanitize=address -o server -pthread server.c