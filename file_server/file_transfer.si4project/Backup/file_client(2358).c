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
#include "file_client_datadeal.h"

#define CLIENT_PORT 8889 
#define CLIENT_IP   "192.168.10.100" 
#define BUF_SIZE    1024

int g_iConnectFd = 0;

int client_init_socket(void)
{
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

int client_send_request(int iPort, char *cpServerIp)
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

int client_data_interaction(void)
{
    int i_ret = FILE_CLIENT_OK;   
    char cBuf_a[BUF_SIZE];
	TEST_HDR_T st_test_hdr;

    memset(&st_test_hdr, 0, sizeof(st_test_hdr));
    st_test_hdr = datadeal_get_hdr();
	if (st_test_hdr.en_cmd == CMD_TEST_SET)
	{
	    i_ret = datadeal_file_set(g_iConnectFd);
		if (i_ret == FILEDATA_DEAL_RET_FAIL)
		{
            file_error("[%s]datadeal_file_set is failed!\n", __FUNCTION__);
			return FILE_CLIENT_ERROR;
		}
	}
	if (st_test_hdr.en_cmd == CMD_TEST_GET)
	{
	    i_ret = datadeal_file_get(g_iConnectFd);
		if (i_ret == FILEDATA_DEAL_RET_FAIL)
		{
            file_error("[%s]datadeal_file_get is failed!\n", __FUNCTION__);
			return FILE_CLIENT_ERROR;
		}    
	}
	if (st_test_hdr.en_cmd == CMD_TEST_LIST)
	{
	    i_ret = datadeal_file_list(g_iConnectFd);
		if (i_ret == FILEDATA_DEAL_RET_FAIL)
		{
            file_error("[%s]datadeal_file_list is failed!\n", __FUNCTION__);
			return FILE_CLIENT_ERROR;
		}     
	}
	
	return FILE_CLIENT_OK;
}


int client_send_data(int i_connect_fd, char *cp_buf, int i_bufsize)
{
    assert(NULL != cp_buf);

	int i_send_bytes = 0;
	int i_total_send_bytes = 0;

    while (i_total_send_bytes < i_bufsize)
    {
        i_send_bytes = send(i_connect_fd, cp_buf, i_bufsize - i_send_bytes, 0);
		if (-1 == i_send_bytes)
		{
            perror("send");
			return FILE_CLIENT_ERROR;
		}

		i_total_send_bytes += i_send_bytes;
	}

	return FILE_CLIENT_OK;
}


