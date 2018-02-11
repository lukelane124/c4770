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
#include <sys/sendfile.h>
#include <sys/stat.h>

// TODO: Some string functions here are not thread safe, these need to be updated to either their thread safe counterparts, or to be used with a global mutex.


#define PORT 5555
#define MAX_BUFFER_SIZE 1024

const char webpage[] =
"HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=UTF-8\r\n\r\n<!DOCTYPE html>\r\n<html><head><title>Default</title></head><body>This is the default file for Tommy's Server 0.0.1</body></html>\r\n\r\n\0";
const char timepage[] =
"HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=UTF-8\r\n\r\n<!DOCTYPE html>\r\n<html><head><title>Default</title></head><body><h1>This is the default file for Tommy's Server 0.0.1</h1><br><h2>%s</h2></body></html>\r\n\r\n\0";
const char fofPage[] =
"HTTP/1.1 404 Not Found\r\nContent-Type: text/html; charset=UTF-8\r\n\r\n<!DOCTYPE html>\r\n<html><head><title>404 Not Found</title></head><body><h1>Error 404</h1><br>File not found. Closing connection</body></html>\r\n\r\n\0";
const char htmlHeader[] =
"HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=UTF-8\r\n\r\n";

void* connectHandler(void* args) {
  int clisock = *((int*) args);
  size_t r_msg_size = 1024;
  char* r_msg = (char *) malloc(1024*sizeof(char));
  int bytesRead = -5;
  bool cont = true;
  int returnZeroCount = 0;
whileloop:
  while(cont) {
    bytesRead = read(clisock, r_msg, r_msg_size);
    if (bytesRead < 0) {
      printf("Error reading from socket.\n");
      //close(clisock);
      pthread_exit(NULL);
    } else if(bytesRead == 0) {
      if (returnZeroCount > 20) 
        cont = false;
      sleep(.3);   
      returnZeroCount++;
      continue;
    } else if(bytesRead < r_msg_size) {
      printf("%s\n", r_msg);
    } else {
      printf("Header toooo long, failed with too large an input.\n");
      exit(-3);
    }
   
    char str[256] = {0};
    char* file;
    int count = sscanf(r_msg, "GET /%s %*s\n", &str);
    printf("%s\n", str);
    int requestedFD = 0;
    if(strcmp("/", str) == 0) {
      printf("root requested\n");
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
      goto whileloop;
    } else {
        char delim[] =  {"?#"};
        char* saveptr;
        file = strtok_r(str, delim, &saveptr);
        char fileName[256] = {0};
        /*for (int i = 1; (file[i-1] != '\0')&&(i < 256); i++) {
          fileName[i-1]=file[i];
        }*/
        requestedFD = open(file, O_RDONLY);
        char* kvp = strtok_r(0, delim, &saveptr);
        printf("requested file:%s\nKVPtoken: %s\n",file, kvp);
    }


    if (requestedFD == -1){
      printf("File Not found..\n");
      write(clisock, fofPage, sizeof(fofPage)-1);
      //close(clisock);
      //pthread_exit(NULL);
    }
    struct stat st;
    fstat(requestedFD, &st);
    write(clisock, htmlHeader, sizeof(htmlHeader)-1);
    sendfile(clisock, requestedFD, 0, st.st_size);
    close(requestedFD);
    memset(r_msg, 0, r_msg_size);
    memset(str, 0, 256);
  }
  close(clisock);
  pthread_exit(NULL);
}

void handleConnect(int clisock) {
  
  pthread_attr_t attribs;
  pthread_t thread;
  pthread_attr_init(&attribs);
  pthread_attr_setdetachstate(&attribs, PTHREAD_CREATE_DETACHED);
  pthread_create(&thread, &attribs, connectHandler, (void*)&clisock);   
} 



int main(int argc, char* argv[]) {
  int sockfd, newsockfd, clilen;
  struct sockaddr_in cli_addr, serv_addr;
  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) { 
    printf("There was an error getting a sockfd from the OS.");
    exit(-1);
  }
  memset((void *) &serv_addr, 0, sizeof(serv_addr));
  printf("argc: %i\n", argc);
  serv_addr.sin_family = AF_INET;                 //Set address family(ipv4)
  serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);  //Set address header for any incomming address.

  short int port;
  if (argc > 1) {
    port = atoi(argv[1]);
    serv_addr.sin_port = htons(port);
  }else { 
    serv_addr.sin_port = htons(PORT);               //Set listening port.
    port = PORT;
  }
  

  //bind our new "listening socket" with the params set above.
  while (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) { 
    printf("Error binding Sock to addr.\n");
    sleep(10);
    //exit(-1);
  }
  printf("Binding to listeing port %i completed successfully\n", port);
  //Tell OS we would like to start listening on this socket, we're not going to create a connection to a specific address.
  //  We want to keep this open so that we can be available for anyone who wants to talk.
  //Second param is "backlog" of connections the OS will pool for us.
  listen(sockfd, 20);

  for (;;) {  //Forever
    //How big is the header for internet address.
    clilen = sizeof(cli_addr);
    // sockfd = accept(sockfd(listening), sockaddrHeaderStruct address)
    newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, (socklen_t *) &clilen);

    if (newsockfd < 0) { 
      printf("Error accepting new client.");
      //exit(-1);
    }
    handleConnect(newsockfd);
  }
}