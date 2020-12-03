#ifndef _FILE_CLIENT_H_
#define _FILE_CLIENT_H_

#define FILE_CLIENT_OK              0
#define FILE_CLIENT_ERROR          -1
#define FILE_CLIENT_RECV_PEER_DOWN  1
#define FILE_CLIENT_EWOULDBLOCK     2


typedef struct sockaddr SA_S;
typedef struct sockaddr_in SAI_S;

int Client_initSocket(void);

int Client_sendReq(int iPort, char* pcServerIp);

int Client_dataInteraction(void);

int Client_sendData(int iConnectFd, void* pBuf, int iBufSize);
int Client_recvData(int iConnectFd, void* pBuf, int iBufSize);








#endif

