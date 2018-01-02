#ifndef _FILE_CLIENT_H_
#define _FILE_CLIENT_H_

#define FILE_CLIENT_OK 0
#define FILE_CLIENT_ERROR -1


typedef struct sockaddr SA;
typedef struct sockaddr_in SA_I;

int init_socket(void);

int send_request(int iPort, char *cpServerIp);

int data_interaction(void);






#endif

