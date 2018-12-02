#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include "server.h"

const char webpage[] =
"HTTP/1.1 200 OK\r\nConnection: close\r\nContent-Type: text/html; charset=UTF-8\r\n\r\n<!DOCTYPE html>\r\n<html><head><title>%s</title></head><body>%s</body></html>\r\n\r\n\0";
const char timepage[] =
"HTTP/1.1 200 OK\r\nConnection: close\r\nContent-Type: text/html; charset=UTF-8\r\n\r\n<!DOCTYPE html>\r\n<html><head><title>Default</title></head><body><h1>This is the default file for Tommy's Server 0.0.1</h1><br><h2>%s</h2></body></html>\r\n\r\n\0";
const char fofPage[] =
"HTTP/1.1 404 Not Found\r\nConnection: close\r\nContent-Type: text/html; charset=UTF-8\r\n\r\n<!DOCTYPE html>\r\n<html><head><title>404 Not Found</title></head><body><h1>Error 404</h1><br>File not found. Closing connection</body></html>\r\n\r\n\0";
const char htmlHeader[] =
"HTTP/1.1 200 OK\r\nConnection: close\r\nContent-Type: text/html; \r\n\r\n";
const char pngHeader[] =
"HTTP/1.1 200 OK\r\nConnection: close\r\nContent-Type: image/png\r\nContent-Length: %i\r\n\r\n";
const char textHeader[] =
"HTTP/1.1 200 OK\r\nConnection: close\r\nContent-Type: text/text; \r\n\r\n";
extern int clisock;

int extensionHash(char* str) {
  if (strncmp(str, ".html", 5) == 0){
    printf("HTML_FILE\n");
    return HTML_FILE;
  } else if (strncmp(str, ".png", 4) == 0){
    printf("PNG_FILE\n");
    return PNG_FILE;
  } else if (strncmp(str, ".bin", 4) == 0) {
    printf("BINARY_FILE\n");
    return BINARY_FILE;
  } else if (strncmp(str, ".lua", 4) == 0){
    printf("LUA_FILE\n");
    return LUA_FILE;
  } else {
    return -1;
  }
}

void handlerGETRequest(client_request_t cliReq)
{
  char* r_msg = cliReq.header;
  int clisock = cliReq.clientSocket;
  bool cont = true;
  size_t r_msg_size = strlen(r_msg);
whileloop:
  //while(cont)
  {
      char str[256] = {0};
      char* file;
      int count = sscanf(r_msg, "GET %s %*s\n", &str);
      printf("GET header: \n%s\n", str);
      int requestedFD = 0;
    if(strcmp("/", str) == 0) 
    {
      printf("root file requested\n");
      time_t t = time(NULL);
      struct tm *tm = localtime(&t);
      char s[64] = {0};
      strftime(s, sizeof(s), "%c", tm);
      char dateTime[512] = {0};
      sprintf(dateTime, timepage, s);
      printf("TimePage%s\n", dateTime);
      write(clisock, dateTime, sizeof(dateTime)-1);
      memset(r_msg, 0, r_msg_size);
      memset(str, 0, 256);
      //goto whileloop;
    } 
    else 
    {
      char delim[] =  {"?#"};
      char* saveptr;
      file = strtok_r(str, delim, &saveptr);
      char fileName[256] = {0};
      for (int i = 1; (file[i-1] != '\0')&&(i < 257); i++) {
        fileName[i-1]=file[i];
      }
      // memcpy((void*) file+1, (void*) fileName, strlen(file)-1);
      requestedFD = open(fileName, O_RDONLY);
      char* extension = strrchr(fileName, '.');
      char* kvp = strtok_r(0, delim, &saveptr);
      printf("requested file:%s\nKVPtoken: %s\nFileExtension: %s\n",fileName, kvp, extension);
      if (requestedFD == -1)
      {
        printf("File Not found..\n");
        write(clisock, fofPage, sizeof(fofPage)-1);
        pthread_exit(NULL);
      }
      struct stat st;
      fstat(requestedFD, &st);
      if (extensionHash(extension) != HTML_FILE) 
      {
        switch(extensionHash(extension)) 
        {
          case PNG_FILE:{
              write(clisock, pngHeader, sizeof(pngHeader)-1);

            //bool sendFileOverSocket(int fd, FILE* socket, char* formatHeader) {
            printf("PNG file requested\n");
            sendFileOverSocket(requestedFD, clisock, pngHeader);
            
            break;
          }
          default:{}
        }
      } 
      else 
      {
        write(clisock, htmlHeader, sizeof(htmlHeader)-1);
      }

      long unsigned int accum = 0;
      while (accum < st.st_size) {
        accum += sendfile(clisock, requestedFD, 0, st.st_size);
        printf("Writing with Acum Value: %li\n", accum);
      }
      char name[60]={0};
      sprintf(name, "/proc/self/fd/%i", requestedFD);
      char name2[60]={0};
      readlink(name, name2, 60);
      printf("File being sent over socket: %s\n", name2);
       // close(requestedFD);
       // printf("Thread closing\n");
        //goto end;
    }

  }
}