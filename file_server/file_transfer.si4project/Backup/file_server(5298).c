#include "file_server.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <sys/types.h>			/* See NOTES */
#include <sys/socket.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <pthread.h>
#include "file_debug.h"


#define BACKLOG           1024
#define CLIENT_IP_STR_LEN 50
#define BUFSIZE           1024

int g_iListenFd = 0;

void * client_data_deal(void *arg)
{
    int iConnectFd = (int)arg;
    char cBuf_a[BUFSIZE];
	int iRet = FILE_SERVER_OK;

	memset(cBuf_a, 0, sizeof(cBuf_a));
	iRet = recv(iConnectFd, cBuf_a, BUFSIZE, 0);
	if (-1 == iRet)
	{
        file_error("recv is failed!\n");
		close(iConnectFd);
		return (void *)FILE_SERVER_ERROR;
	}
	else if (0 ==iRet)
	{
        file_error("peer is shutdown");
		close(iConnectFd);
		return (void *)FILE_SERVER_OK;
	}
	else
	{
        file_printf("recv data is:%s\n", cBuf_a);
	}

	file_printf("client close!\n");
	close(iConnectFd);
    return (void *)FILE_SERVER_OK;
}


int get_listenfd(void)
{
    return g_iListenFd;
}

int init_socket(int iPort)
{
	SA_I stServerAddr;     
	int iRet = 0;

	memset(&stServerAddr, 0, sizeof(stServerAddr));
	stServerAddr.sin_family = AF_INET;
	stServerAddr.sin_port = htons(iPort); 
    stServerAddr.sin_addr.s_addr = INADDR_ANY;

	/*create the socket*/
	g_iListenFd = socket(AF_INET, SOCK_STREAM, 0);
	if (-1 == g_iListenFd)
	{
        file_error("socket create is faileed!\n");
		return FILE_SERVER_ERROR;
	}

    iRet = bind(g_iListenFd, (SA *)&stServerAddr, sizeof(stServerAddr));
	if (-1 == iRet)
	{
        file_error("bind is failed!\n");
		close(g_iListenFd);
		return FILE_SERVER_ERROR;
	}

	iRet = listen(g_iListenFd, BACKLOG);
	if (-1 == iRet)
	{
        file_error("listen is faileed!\n");
		close(g_iListenFd);
		return FILE_SERVER_ERROR;
	}

	return FILE_SERVER_OK;
}

int deal_client_request(void)
{
    int iConnectFd = 0;
	SA_I stClientAddr;
	socklen_t clientAddrlen = sizeof(stClientAddr);
	char cClientIpStr_a[CLIENT_IP_STR_LEN];
    pthread_t threadId;
	int iRet = FILE_SERVER_OK;
	

	memset(&stClientAddr, 0, sizeof(stClientAddr));
	iConnectFd = accept(g_iListenFd, (SA *)&stClientAddr, &clientAddrlen);
	if (-1 == iConnectFd)
	{
        file_error("accept  is faileed!\n");
		close(g_iListenFd);
	    return FILE_SERVER_ERROR;
	}

    file_printf("client connect is success!\n");
    memset(cClientIpStr_a, 0, sizeof(cClientIpStr_a));
	inet_ntop(AF_INET, &stClientAddr.sin_addr.s_addr, cClientIpStr_a, sizeof(cClientIpStr_a));
	file_printf("client port is: %d,  ip is: %s\n", ntohs(stClientAddr.sin_port), cClientIpStr_a);

    iRet = pthread_create(&threadId, NULL, (void *)client_data_deal, (void *)iConnectFd);
	if (0 != iRet)
	{
        file_error("pthread_create is failed!\n");
		close(g_iListenFd);
		close(iConnectFd);
		return FILE_SERVER_ERROR;
	}
	iRet = pthread_detach(threadId);
	if (0 != iRet)
	{
        file_error("pthread_detach is failed!\n");
		close(g_iListenFd);
		close(iConnectFd);
		return FILE_SERVER_ERROR;
	}

	return FILE_SERVER_OK;
}

