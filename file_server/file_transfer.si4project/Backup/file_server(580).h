#ifndef _FILE_SERVER_H_
#define _FILE_SERVER_H_

#define FILE_SERVER_OK 0
#define FILE_SERVER_ERROR -1


typedef struct sockaddr SA;
typedef struct sockaddr_in SA_I;

int init_socket(int iPort);
int deal_client_request(void);
int get_listenfd(void);




#endif


