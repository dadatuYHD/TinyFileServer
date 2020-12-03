#include "file_server.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <pthread.h>
#include <unistd.h>
#include "file_server_debug.h"
#include "file_server_datadeal.h"


#define BACKLOG                  1024
#define CLIENT_IP_STR_LEN        50
#define BUFSIZE                  1024
#define EPOLL_SERVER             1
#define MULTI_THREAD_SERVER      0


int g_iListenFd = 0;

int Server_recvData(int iConnectFd, void *pBuf, int iBufSize)
{
    assert(NULL != pBuf);

    int iRecvBytes = 0;
    int iTotalRecvBytes = 0;
    int iRet = FILE_SERVER_OK;

    while (iTotalRecvBytes < iBufSize) {
        iRecvBytes = recv(iConnectFd, (char *)pBuf, iBufSize - iTotalRecvBytes, 0);
        if (-1 == iRecvBytes) {
            perror("recv");
            close(iConnectFd);
            return FILE_SERVER_ERROR;
        } else if (0 ==iRecvBytes) {
            File_error("peer is shutdown");
            close(iConnectFd);
            return FILE_SERVER_RECV_PEER_DOWN;
        } else {
            iTotalRecvBytes += iRecvBytes;
        }     
    }

    return iTotalRecvBytes;
}

int server_sendData(int iConnectFd, void *pBuf, int iBufSize)
{
    assert(NULL != pBuf);

    int iSendBytes = 0;
    int iTotalSendBytes = 0;

    while (iTotalSendBytes < iBufSize) {
        iSendBytes = send(iConnectFd, (char *)pBuf, iBufSize - iTotalSendBytes, 0);
        if (-1 == iSendBytes) {
            perror("send");
            return FILE_SERVER_ERROR;
        }

        iTotalSendBytes += iSendBytes;
    }

    return iTotalSendBytes;
}

void * server_client_data_deal(void *arg)
{
    int iConnectFd = (int)arg;
    char cPackBuf[BUFSIZE];
    TestHdr_S* pstTestHdr = NULL;
    int iRet = FILE_SERVER_OK;
    int iRecvHdrBytes = 0;

    while (1) {
        /*recv uiData hdr*/
        File_running("start recv client uiData hdr!\n");
        pstTestHdr = DataDeal_getHdrAddress();
        memset(pstTestHdr, 0, sizeof(TestHdr_S));
        iRecvHdrBytes = Server_recvData(iConnectFd, pstTestHdr, sizeof(TestHdr_S));
        if (iRecvHdrBytes == FILE_SERVER_ERROR) {
            File_error("[%s]server_recv_hdr uiData is error, close server!!\n", __FUNCTION__);
            close(iConnectFd);
            return (void *)FILE_SERVER_ERROR;
        } else if (iRecvHdrBytes == FILE_SERVER_RECV_PEER_DOWN) {
            File_running("[%s]client close the connect!\n", __FUNCTION__);    
        } else {
            File_running("recv client uiData hdr is success!\n");
            File_printf("Server_recvData recv hdr uiData %d bytes\n", iRecvHdrBytes);
        }
        
        /*uiData hdr version check*/
        if (VERSION_ONE != pstTestHdr->m_enVersion) {
            File_error("[%s]uiData hdr version check error, close server!!\n", __FUNCTION__);
            File_running("please recv uiData hdr again!\n");
            return (void *)FILE_SERVER_ERROR;
        }
        File_running("uiData hdr version check success!\n");
        
        if (iRecvHdrBytes != pstTestHdr->m_usHdrLen) {
            File_error("[%s]uiData hdr  len check error, close server!!\n", __FUNCTION__);
            File_running("please recv uiData hdr again!\n");
            return (void *)FILE_SERVER_ERROR;    
        }
        File_running("uiData hdr len check success!\n");
        
        /*judge the TestHdr_S m_enCmd field*/
        if (pstTestHdr->m_enCmd == CMD_TEST_SET) {
            iRet = DataDeal_setFile(iConnectFd);
            if (iRet == FILEDATA_DEAL_RET_FAIL) {
                File_error("[%s]DataDeal_setFile is fail, close server!!\n", __FUNCTION__);
                close(iConnectFd);
                return (void *)FILE_SERVER_ERROR;
            }
        } else if (pstTestHdr->m_enCmd == CMD_TEST_GET) {
            iRet = DataDeal_getFile(iConnectFd);
            if (iRet == FILEDATA_DEAL_RET_FAIL) {
                File_error("[%s]DataDeal_getFile is fail, close server!!\n", __FUNCTION__);
                close(iConnectFd);
                return (void *)FILE_SERVER_ERROR;
            }
        } else if (pstTestHdr->m_enCmd == CMD_TEST_LIST) {
            iRet = datadeal_file_list(iConnectFd);
            if (iRet == FILEDATA_DEAL_RET_FAIL) {
                File_error("[%s]datadeal_file_list is fail, close server!!\n", __FUNCTION__);
                close(iConnectFd);
                return (void *)FILE_SERVER_ERROR;
            }
        } else if (pstTestHdr->m_enCmd == CMD_TEST_CLIENT_EXIT) {
            File_running("server close!\n");
            close(iConnectFd);
            return (void *)FILE_SERVER_OK;        
        }    
    }
}

int Server_getListenFd(void)
{
    return g_iListenFd;
}

int Server_initSocket(int iPort)
{
    SAI_S stServerAddr;     
    int iRet = 0;

    memset(&stServerAddr, 0, sizeof(stServerAddr));
    stServerAddr.sin_family = AF_INET;
    stServerAddr.sin_port = htons(iPort); 
    stServerAddr.sin_addr.s_addr = INADDR_ANY;

    /*create the socket*/
    g_iListenFd = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == g_iListenFd) {
        File_error("socket create is faileed!\n");
        return FILE_SERVER_ERROR;
    }

    iRet = bind(g_iListenFd, (SA_S *)&stServerAddr, sizeof(stServerAddr));
    if (-1 == iRet) {
        File_error("bind is failed!\n");
        close(g_iListenFd);
        return FILE_SERVER_ERROR;
    }

    iRet = listen(g_iListenFd, BACKLOG);
    if (-1 == iRet) {
        File_error("listen is faileed!\n");
        close(g_iListenFd);
        return FILE_SERVER_ERROR;
    }

    return FILE_SERVER_OK;
}

#if EPOLL_SERVER
int Server_dealClientReq(void) 
{
    
}

#elif MULTI_THREAD_SERVER
int Server_dealClientReq(void)
{
    int iConnectFd = 0;
    SAI_S stClientAddr;
    socklen_t clientAddrlen = sizeof(stClientAddr);
    char cClientIpStr[CLIENT_IP_STR_LEN];
    pthread_t threadId;
    int iRet = FILE_SERVER_OK;
    
    memset(&stClientAddr, 0, sizeof(stClientAddr));
    iConnectFd = accept(g_iListenFd, (SA_S *)&stClientAddr, &clientAddrlen);
    if (-1 == iConnectFd) {
        perror("accept");
        close(g_iListenFd);
        return FILE_SERVER_ERROR;
    }

    File_running("client connect is success!\n");
    memset(cClientIpStr, 0, sizeof(cClientIpStr));
    inet_ntop(AF_INET, &stClientAddr.sin_addr.s_addr, cClientIpStr, sizeof(cClientIpStr));
    File_running("client port is: %d,  ip is: %s\n", ntohs(stClientAddr.sin_port), cClientIpStr);

    iRet = pthread_create(&threadId, NULL, (void *)server_client_data_deal, (void *)iConnectFd);
    if (0 != iRet) {
        perror("pthread_create");
        close(g_iListenFd);
        close(iConnectFd);
        return FILE_SERVER_ERROR;
    }
    iRet = pthread_detach(threadId);
    if (0 != iRet) {
        perror("pthread_detach");
        close(g_iListenFd);
        close(iConnectFd);
        return FILE_SERVER_ERROR;
    }

    return FILE_SERVER_OK;
}
#endif

