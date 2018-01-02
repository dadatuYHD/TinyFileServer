#include "file_client.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <sys/types.h>			/* See NOTES */
#include <sys/socket.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <pthread.h>
#include <unistd.h>
#include "file_debug.h"



#define CLIENT_PORT 8889 
#define CLIENT_IP   "192.168.10.100" 
#define BUF_SIZE    1024

int g_iConnectFd = 0;

int init_socket(void)
{
    file_trace();

	int iRet = 0;
#if 0
	SA_I stClientAddr;

    memset(&stClientAddr, 0, sizeof(stClientAddr));
	stClientAddr.sin_family = AF_INET;
	stClientAddr.sin_port = htons(CLIENT_PORT); 
    inet_pton(AF_INET, CLIENT_IP, &stClientAddr.sin_addr.s_addr);
#endif
	/*create the socket*/
	g_iConnectFd = socket(AF_INET, SOCK_STREAM, 0);
	if (-1 == g_iConnectFd)
	{
        file_error("socket create is faileed!\n");
		return FILE_CLIENT_ERROR;
	}
#if 0
    iRet = bind(g_iConnectFd, (SA *)&stClientAddr, sizeof(stClientAddr));
	if (-1 == iRet)
	{
        file_error("bind is failed!\n");
		close(g_iConnectFd);
		return FILE_CLIENT_ERROR;
	}
#endif
	return FILE_CLIENT_OK;
}

int send_request(int iPort, char *cpServerIp)
{
    file_trace();
    SA_I stServerAddr;
	int iRet = FILE_CLIENT_OK;

    memset(&stServerAddr, 0, sizeof(stServerAddr));
	stServerAddr.sin_family = AF_INET;
	stServerAddr.sin_port = htons(iPort); 
    inet_pton(AF_INET, cpServerIp, &stServerAddr.sin_addr.s_addr);

    iRet = connect(g_iConnectFd, (SA *)&stServerAddr, sizeof(stServerAddr));
	if (-1 == iRet)
	{
        file_error("connect is failed!\n");
		close(g_iConnectFd);
		return FILE_CLIENT_ERROR;
	}

	return FILE_CLIENT_OK;
}

int data_interaction(void)
{
    int iRet = FILE_CLIENT_OK;   
    char cBuf_a[BUF_SIZE];


    file_printf("please input data:\n");  
	memset(cBuf_a, 0, BUF_SIZE);
	iRet = read(STDIN_FILENO, cBuf_a, BUF_SIZE);
    if (-1 == iRet)
    {
        file_error("read is failed!\n");
		close(g_iConnectFd);
		return FILE_CLIENT_ERROR;        
	}
    else if (0 == iRet)
    {
        file_printf("end of file!\n");  
	}
	else
	{
        iRet = send(g_iConnectFd, cBuf_a, BUF_SIZE, 0); 
	    if (-1 == iRet)
	    { 
            file_error("send is failed!\n");
		    close(g_iConnectFd);
		    return FILE_CLIENT_ERROR;   
	    }
	}

	return FILE_CLIENT_OK;
}


