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

#define PORT 5555
#define MAX_BUFFER_SIZE 1024

//Data structs for addressing
// struct sockaddr {
//   unsigned short sa_family;       // address family, AF_xxx
//   char sa_data[14];               // 14 bytes of protocol address
// }; 

// struct sockaddr_in {
//   short int sin_family;           // Address family
//   unsigned short int sin_port;    // Port number
//   struct in_addr sin_addr;        // Internet address
//   unsigned char sin_zero[8];      // Same size as struct sockaddr
// }; 

//Dealing with address and byte orders.

// struct in_addr {
//   // that's a 32-bit long, or 4 bytes  
//   unsigned long s_addr; 
// }; 

const char webpage[] =
"HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=UTF-8\r\n\r\n<!DOCTYPE html>\r\n<html><head><title>This is a test...</title></head><body>Hello world</body></html>\r\n\0";

void* connectHandler(void* args) {
  int clisock = *((int*) args);
  //int clisock = (int) args;
  // FILE *fp;
  // if ((fp = fdopen(clisock, "r+")) == NULL) {
  //   printf("Error opening fd for socket passed from listening thread.");
  //   exit(-2);
  // }


  size_t r_msg_size = 1024;
  char* r_msg = (char *) malloc(1024*sizeof(char));
  int bytesRead = -5;
  bool cont = true;
  while(cont) {
    bytesRead = read(clisock, r_msg, r_msg_size);
    if (bytesRead < 0) {
      printf("Error reading from socket.\n");
      pthread_exit(NULL);
    } else if(bytesRead < r_msg_size) {
      printf("Size: %i\nMSG: %s\n", bytesRead, r_msg);
     // bytesRead = 0;
      //break;
    } else if(bytesRead == 0) {
      sleep(.3);     
    } else {
      printf("Header toooo long, failed with too large an input.\n");
      exit(-3);
    }
    cont = true;
    printf("Made it past \n");
    /*char* parsable_msg = (char*) malloc(sizeof(r_msg));
    for (int i = 0; (i < r_msg_size)||(r_msg[i] == '\0'); i++) {
      parsable_msg[i] = r_msg[i];
    }
    int offset = scanf(parsable_msg, "^GET");*/
    int offset = scanf(r_msg, "^GET");
    printf("Offset: %i\n", offset);
    //free(parsable_msg);
    write(clisock, webpage, sizeof(webpage)-1);
    //memset(void *s, int c, size_t n)
    // sets n bytes indexed starting at location s, with value c.
    memset(r_msg, 0, r_msg_size);
    sleep(5);
   // shutdown(clisock, 2);
  }
  
  //fputs(webpage, fp);

  //fclose(fp);
  pthread_exit(NULL);
}

void handleConnect(int clisock) {
  printf("Inside handleConnect\n");
  pthread_attr_t attribs;
  pthread_t thread;
  pthread_attr_init(&attribs);
  pthread_attr_setdetachstate(&attribs, PTHREAD_CREATE_DETACHED);
  pthread_create(&thread, &attribs, connectHandler, (void*)&clisock);   
} 


/*
Converting byte orders

htons() -- "Host to Network Short"

htonl() -- "Host to Network Long"

ntohs() -- "Network to Host Short"

ntohl() -- "Network to Host Long"
*/

//Dealing with addresses
//struct sockaddr_in my_addr;

//my_addr.sin_family = AF_INET;        // host byte order
//my_addr.sin_port = htons(MYPORT);    // short, network byte order

// If we want to specify our interface:
//inet_aton("10.12.110.57", &(my_addr.sin_addr));
// Or, if we don’t care which interface:
// my_addr.sin_addr.s_addr = htonl(INADDR_ANY);

// zero the rest of the struct:
//memset(&(my_addr.sin_zero), '\0', 8);  

// We like to deal with names: br206-02
//Struct hostent *he = gethostbyname(“br206-02.csc.tntech.edu”);
//theirAddr.sin_addr = *((struct in_addr *) he->h_addr);


// Creating and binding socket

//int socket(int domain, int type, int protocol);

//int bind(9int ockfd, struct sockaddr *my_addr, int addrlen);

//Making and accepting connections

//clients
//int connect(int sockfd, struct sockaddr *serv_addr, int addrlen);

//server
// int listen(int sockfd, int backlog);
// int accept(int sockfd, void *addr, int *addrlen);

//Sending and recieving data ROGERS SUGGESTED DIFFERENT 2-9-18
// int send(int sockfd, cont void *msg, int leng, ing flags);
// int recv(int sockfd, void *buf, int len, unsigned int flags);

//Server Code, HEADERS AT TOP.


int main(int argc, char* argv[]) {
  int sockfd, newsockfd, clilen;
  struct sockaddr_in cli_addr, serv_addr;
      //sockfd = socket(domain(AF_INET == ipv4), type(SOCK_STREAM == "reliable duplex"), protocol(0 == SCK_STRM.TCP));
               // protocol is 0 becuase only 1 protocol support reliable duplex,
  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) { 
    printf("There was an error getting a sockfd from the OS.");
    exit(-1);
  }
  //memset(void *s, int c, size_t n)
  // sets n bytes indexed starting at location s, with value c.
  memset((void *) &serv_addr, 0, sizeof(serv_addr));

/*
struct sockaddr {
  unsigned short sa_family;       // address family, AF_xxx
  char sa_data[14];               // 14 bytes of protocol address
}; 

*/
  serv_addr.sin_family = AF_INET;                 //Set address family(ipv4)
  serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);  //Set address header for any incomming address.
  if (argc > 1) {
    serv_addr.sin_port = htons(htonl(atoi(argv[2])));
  }else { 
    serv_addr.sin_port = htons(PORT);               //Set listening port.
  }
  

  //bind our new "listening socket" with the params set above.
  while (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) { 
    //printf("Error binding Sock to addr.");
    sleep(1);
    //exit(-1);
  }
  printf("Binding successfull\n");
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