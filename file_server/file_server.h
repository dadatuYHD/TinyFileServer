#ifndef _FILE_SERVER_H_
#define _FILE_SERVER_H_

#define FILE_SERVER_OK             0
#define FILE_SERVER_ERROR         -1
#define FILE_SERVER_RECV_PEER_DOWN 1


typedef struct sockaddr SA;
typedef struct sockaddr_in SA_I;

int server_init_socket(int iPort);
int server_deal_client_request(void);
int server_get_listenfd(void);
int server_recv_data(int i_connect_fd, void *p_buf, int i_bufsize);





#endif


