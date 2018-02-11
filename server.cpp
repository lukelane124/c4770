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

#define PORT 5555
#define MAX_BUFFER_SIZE 1024

const char webpage[] =
"HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=UTF-8\r\n\r\n<!DOCTYPE html>\r\n<html><head><title>Default</title></head><body>This is the default file for Tommy's Server 0.0.1</body></html>\r\n\r\n\0";
const char fofPage[] =
"HTTP/1.1 404 Not Found\r\nContent-Type: text/html; charset=UTF-8\r\n\r\n<!DOCTYPE html>\r\n<html><head><title>404 Not Found</title></head><body><h1>Error 404</h1><br>File not found. Closing connection</body></html>\r\n\r\n\0";


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
      pthread_exit(NULL);
    } else if(bytesRead == 0) {
      if (returnZeroCount > 20) 
        cont = false;
      sleep(.3);   
      returnZeroCount++;
      continue;
    } else if(bytesRead < r_msg_size) {
      
    } else {
      printf("Header toooo long, failed with too large an input.\n");
      exit(-3);
    }
   
    char str[256] = {0};
    int count = sscanf(r_msg, "GET %s %*s\n", &str);
    printf("%s\n", str);
    int requestedFD;
    printf("Requested File: %s\n", str);
    printf("Match: %i", strcmp("/", str), 20);
    if(strcmp("/", str) == 0) {
      printf("root requested\n");
      write(clisock, webpage, sizeof(webpage)-1);
    } else {
        requestedFD = open(str, O_RDONLY);
        printf("file requested on /\n");
    }


    if (requestedFD == -1){
      printf("File Not found..\n");
      write(clisock, fofPage, sizeof(fofPage)-1);
      close(clisock);
      pthread_exit(NULL);
    }

    //sendfile(clisock, requestedFD, 0, 0);
    
    memset(r_msg, 0, r_msg_size);
    memset(str, 0, 256);
    sleep(1);
   // shutdown(clisock, 2);
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

  serv_addr.sin_family = AF_INET;                 //Set address family(ipv4)
  serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);  //Set address header for any incomming address.
  if (argc > 1) {
    serv_addr.sin_port = htons(htonl(atoi(argv[2])));
  }else { 
    serv_addr.sin_port = htons(PORT);               //Set listening port.
  }
  

  //bind our new "listening socket" with the params set above.
  while (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) { 
    printf("Error binding Sock to addr.\n");
    sleep(10);
    //exit(-1);
  }
  printf("Binding to listeing port successfull\n");
  //Tell OS we would like to start listening on this socket, we're not going to create a connection to a specific address.
  //  We want to keep this open so that we can be available for anyone who wants to talk.
  //Second param is "backlog" of connections the OS will pool for us.
  listen(sockfd, 5);

  for (;;) {  //Forever
    //How big is the header for internet address.
    clilen = sizeof(cli_addr);
    // sockfd = accept(sockfd(listening), sockaddrHeaderStruct address)
    newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, (socklen_t *) &clilen);

    if (newsockfd < 0) { 
      printf("Error accepting new client.");
      exit(-1);
    }
    handleConnect(newsockfd);
  }
}