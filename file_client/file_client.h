#ifndef _FILE_CLIENT_H_
#define _FILE_CLIENT_H_

#define FILE_CLIENT_OK              0
#define FILE_CLIENT_ERROR          -1
#define FILE_CLIENT_RECV_PEER_DOWN  1
#define FILE_CLIENT_EWOULDBLOCK     2


typedef struct sockaddr SA;
typedef struct sockaddr_in SA_I;

int client_init_socket(void);

int client_send_request(int iPort, char *cpServerIp);

int client_data_interaction(void);

int client_send_data(int i_connect_fd, void *p_buf, int i_bufsize);
int client_recv_data(int i_connect_fd, void *p_buf, int i_bufsize);








#endif

