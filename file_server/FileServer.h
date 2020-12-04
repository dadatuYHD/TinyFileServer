#ifndef _FILE_SERVER_H_
#define _FILE_SERVER_H_

#define FILE_SERVER_OK             0
#define FILE_SERVER_ERROR         -1
#define FILE_SERVER_RECV_PEER_DOWN 1


typedef struct sockaddr SA_S;
typedef struct sockaddr_in SAI_S;

int Server_initSocket(int iPort);
int Server_dealClientReq(void);
int Server_getListenFd(void);
int Server_recvData(int iConnectFd, void* pBuf, int iBufSize);
int server_sendData(int iConnectFd, void* pBuf, int iBufSize);






#endif


