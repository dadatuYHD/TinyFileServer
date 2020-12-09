#include <errno.h>
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
#include "FileClientDebug.h"
#include "FileClientDataDeal.h"
#include "FileClientState.h"
#include "FileClient.h"

#define CLIENT_PORT 8889 
#define CLIENT_IP   "192.168.199.219" 
#define BUF_SIZE    1024

int g_iConnectFd = 0;

int Client_initSocket(void)
{
    int iRet = 0;
#if 0
    SAI_S stClientAddr;

    memset(&stClientAddr, 0, sizeof(stClientAddr));
    stClientAddr.sin_family = AF_INET;
    stClientAddr.sin_port = htons(CLIENT_PORT); 
    inet_pton(AF_INET, CLIENT_IP, &stClientAddr.sin_addr.s_addr);
#endif
    /*create the socket*/
    g_iConnectFd = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == g_iConnectFd)
    {
        File_error("socket create is faileed!\n");
        return FILE_CLIENT_ERROR;
    }
#if 0
    iRet = bind(g_iConnectFd, (SA_S *)&stClientAddr, sizeof(stClientAddr));
    if (-1 == iRet)
    {
        File_error("bind is failed!\n");
        close(g_iConnectFd);
        return FILE_CLIENT_ERROR;
    }
#endif
    return FILE_CLIENT_OK;
}

int Client_sendReq(int iPort, char* pcServerIp)
{
    SAI_S stServerAddr;
    int iRet = FILE_CLIENT_OK;

    memset(&stServerAddr, 0, sizeof(stServerAddr));
    stServerAddr.sin_family = AF_INET;
    stServerAddr.sin_port = htons(iPort); 
    inet_pton(AF_INET, pcServerIp, &stServerAddr.sin_addr.s_addr);

    iRet = connect(g_iConnectFd, (SA_S *)&stServerAddr, sizeof(stServerAddr));
    if (-1 == iRet)
    {
        File_error("connect is failed!\n");
        close(g_iConnectFd);
        return FILE_CLIENT_ERROR;
    }

    return FILE_CLIENT_OK;
}

int Client_dataInteraction(void)
{
    int iRet = FILE_CLIENT_OK;   
    char cBuf[BUF_SIZE];
    TestHdr_S stTestHdr;

    memset(&stTestHdr, 0, sizeof(stTestHdr));
    stTestHdr = DataDeal_getHdr();
    if (stTestHdr.m_enCmd == CMD_TEST_SET)
    {
        iRet = DataDeal_setFile(g_iConnectFd);
        if (iRet == FILEDATA_DEAL_RET_FAIL)
        {
            File_error("[%s]DataDeal_setFile is failed!\n", __FUNCTION__);
            return FILE_CLIENT_ERROR;
        }
    }
    if (stTestHdr.m_enCmd == CMD_TEST_GET)
    {
        iRet = DataDeal_getFile(g_iConnectFd);
        if (iRet == FILEDATA_DEAL_RET_FAIL)
        {
            File_error("[%s]DataDeal_getFile is failed!\n", __FUNCTION__);
            return FILE_CLIENT_ERROR;
        }    
    }
    if (stTestHdr.m_enCmd == CMD_TEST_LIST)
    {
        iRet = DataDeal_listFile(g_iConnectFd);
        if (iRet == FILEDATA_DEAL_RET_FAIL)
        {
            File_error("[%s]DataDeal_listFile is failed!\n", __FUNCTION__);
            return FILE_CLIENT_ERROR;
        }     
    }
    if (stTestHdr.m_enCmd == CMD_TEST_CLIENT_EXIT)
    {
        iRet = DataDeal_exitFile(g_iConnectFd);
        if (iRet == FILEDATA_DEAL_RET_FAIL)
        {
            File_error("[%s]DataDeal_listFile is failed!\n", __FUNCTION__);
            return FILE_CLIENT_ERROR;
        } 
        else if (FILESTATE_MAX == iRet) 
        {
            return  FILESTATE_MAX;  
        } 
        else 
        {
            /*************/
        }
    }
    
    return FILE_CLIENT_OK;
}

/************************************************************
* FUNCTION          :Client_sendData
* Description       :by socket file descriptor send uiData
* Arguments         :
* [iConnectFd][IN]:file descriptor
* [pBuf][IN]       :Store uiData to be send
* [pBuf][IN]       :size of to be send
* return            :success return bytes of send，
*                    fail return FILE_CLIENT_ERROR
************************************************************/
int Client_sendData(int iConnectFd, void* pBuf, int iBufSize)
{
    assert(NULL != pBuf);

    int iSendBytes = 0;
    int iTotalSendBytes = 0;

    while (iTotalSendBytes < iBufSize)
    {
        iSendBytes = send(iConnectFd, (char *)pBuf, iBufSize - iTotalSendBytes, 0);
        if (-1 == iSendBytes)
        {
            perror("send");
            return FILE_CLIENT_ERROR;
        }

        iTotalSendBytes += iSendBytes;
    }

    return iTotalSendBytes;
}

/************************************************************
* FUNCTION          :Client_recvData
* Description       :by socket file descriptor recv uiData
* Arguments         :
* [iConnectFd][IN]:file descriptor
* [pBuf][IN]       :Store uiData to be recv
* [pBuf][IN]       :size of to be recv
* return            :success return bytes of recv，fail 
*                    return FILE_CLIENT_ERROR
************************************************************/
int Client_recvData(int iConnectFd, void* pBuf, int iBufSize)
{
    assert(NULL != pBuf);

    int iRecvBytes = 0;
    int iTotalRecvBytes = 0;
    int iRet = FILE_CLIENT_OK;

    while (iTotalRecvBytes < iBufSize) 
    {
        iRecvBytes = recv(iConnectFd, (char *)pBuf, iBufSize - iTotalRecvBytes, 0);
        if (-1 == iRecvBytes) 
        {
            perror("recv");
            File_error("[%s]recv is failed!\n", __FUNCTION__);
            close(iConnectFd);
            return FILE_CLIENT_ERROR;    
        } 
        else if (0 == iRecvBytes) 
        {
            File_running("peer is shutdown\n");
            close(iConnectFd);
            return FILE_CLIENT_RECV_PEER_DOWN;
        } 
        else 
        {
            iTotalRecvBytes += iRecvBytes;
        }     
    }

    return iTotalRecvBytes;
}

