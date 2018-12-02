#ifndef _SERVER_H
#define _SERVER_H

typedef enum supported_f_s
{
  HTML_FILE,
  PNG_FILE,
  BINARY_FILE,
  LUA_FILE,
}supported_f_t;

typedef struct client_request_s
{
	int clientSocket;
	char* header;
}client_request_t;

extern int clisock;

int extensionHash(char* str);
bool sendFileOverSocket(int fd, int socket, const char* formatHeader);
void handlerGETRequest(client_request_t cliReq);
void handlerPOSTRequest(client_request_t cliReq);
void handlerPUTRequest(client_request_t cliReq);
void handlerDELETERequest(client_request_t cliReq);
#endif
