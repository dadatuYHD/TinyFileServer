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
#include <unistd.h>
#include "file_server_debug.h"
#include "file_server_datadeal.h"


#define BACKLOG           1024
#define CLIENT_IP_STR_LEN 50
#define BUFSIZE           1024


int g_iListenFd = 0;

int server_recv_data(int i_connect_fd, void *p_buf, int i_bufsize)
{
    assert(NULL != p_buf);

    int i_recv_bytes = 0;
	int i_total_recv_bytes = 0;
    int i_ret = FILE_SERVER_OK;

    while (i_total_recv_bytes < i_bufsize) {
        i_recv_bytes = recv(i_connect_fd, (char *)p_buf, i_bufsize - i_total_recv_bytes, 0);
	    if (-1 == i_recv_bytes) {
            perror("recv");
		    close(i_connect_fd);
		    return FILE_SERVER_ERROR;
	    } else if (0 ==i_recv_bytes) {
            file_error("peer is shutdown");
		    close(i_connect_fd);
		    return FILE_SERVER_RECV_PEER_DOWN;
		} else {
		    i_total_recv_bytes += i_recv_bytes;
	    }     
	}

    return i_total_recv_bytes;
}

int server_send_data(int i_connect_fd, void *p_buf, int i_bufsize)
{
    assert(NULL != p_buf);

	int i_send_bytes = 0;
	int i_total_send_bytes = 0;

    while (i_total_send_bytes < i_bufsize)
    {
        i_send_bytes = send(i_connect_fd, (char *)p_buf, i_bufsize - i_total_send_bytes, 0);
		if (-1 == i_send_bytes)
		{
            perror("send");
			return FILE_SERVER_ERROR;
		}

		i_total_send_bytes += i_send_bytes;
	}

	return i_total_send_bytes;
}

void * server_client_data_deal(void *arg)
{
    int i_connect_fd = (int)arg;
    char c_pack_buf_a[BUFSIZE];
	TEST_HDR_T * stp_test_hdr = NULL;
	int i_ret = FILE_SERVER_OK;
	int i_recv_hdr_bytes = 0;

    while (1) {
        /*recv data hdr*/
	    file_running("start recv client data hdr!\n");
        stp_test_hdr = datadeal_get_phdr();
        memset(stp_test_hdr, 0, sizeof(TEST_HDR_T));
        i_recv_hdr_bytes = server_recv_data(i_connect_fd, stp_test_hdr, sizeof(TEST_HDR_T));
        if (i_recv_hdr_bytes == FILE_SERVER_ERROR) {
            file_error("[%s]server_recv_hdr data is error, close server!!\n", __FUNCTION__);
            close(i_connect_fd);
            return (void *)FILE_SERVER_ERROR;
        } else if (i_recv_hdr_bytes == FILE_SERVER_RECV_PEER_DOWN) {
            file_running("[%s]client close the connect!\n", __FUNCTION__);    
        } else {
            file_running("recv client data hdr is success!\n");
            file_printf("server_recv_data recv hdr data %d bytes\n", i_recv_hdr_bytes);
        }
        
        /*data hdr version check*/
        if (VERSION_ONE != stp_test_hdr->en_version) {
            file_error("[%s]data hdr version check error, close server!!\n", __FUNCTION__);
            file_running("please recv data hdr again!\n");
            return (void *)FILE_SERVER_ERROR;
        }
        file_running("data hdr version check success!\n");
        
        if (i_recv_hdr_bytes != stp_test_hdr->us_hdr_len) {
            file_error("[%s]data hdr  len check error, close server!!\n", __FUNCTION__);
            file_running("please recv data hdr again!\n");
            return (void *)FILE_SERVER_ERROR;    
        }
        file_running("data hdr len check success!\n");
        
        /*judge the TEST_HDR_T en_cmd field*/
        if (stp_test_hdr->en_cmd == CMD_TEST_SET) {
            i_ret = datadeal_file_set(i_connect_fd);
            if (i_ret == FILEDATA_DEAL_RET_FAIL) {
                file_error("[%s]datadeal_file_set is fail, close server!!\n", __FUNCTION__);
                close(i_connect_fd);
                return (void *)FILE_SERVER_ERROR;
            }
        } else if (stp_test_hdr->en_cmd == CMD_TEST_GET) {
            i_ret = datadeal_file_get(i_connect_fd);
            if (i_ret == FILEDATA_DEAL_RET_FAIL) {
                file_error("[%s]datadeal_file_get is fail, close server!!\n", __FUNCTION__);
                close(i_connect_fd);
                return (void *)FILE_SERVER_ERROR;
            }
        } else if (stp_test_hdr->en_cmd == CMD_TEST_LIST) {
            i_ret = datadeal_file_list(i_connect_fd);
            if (i_ret == FILEDATA_DEAL_RET_FAIL) {
                file_error("[%s]datadeal_file_list is fail, close server!!\n", __FUNCTION__);
                close(i_connect_fd);
                return (void *)FILE_SERVER_ERROR;
            }
        } else if (stp_test_hdr->en_cmd == CMD_TEST_CLIENT_EXIT) {
            file_running("server close!\n");
            close(i_connect_fd);
            return (void *)FILE_SERVER_OK;        
		}    
	}
}


int server_get_listenfd(void)
{
    return g_iListenFd;
}

int server_init_socket(int iPort)
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

int server_deal_client_request(void)
{
    int iConnectFd = 0;
	SA_I stClientAddr;
	socklen_t clientAddrlen = sizeof(stClientAddr);
	char cClientIpStr_a[CLIENT_IP_STR_LEN];
    pthread_t threadId;
	int iRet = FILE_SERVER_OK;
	
	memset(&stClientAddr, 0, sizeof(stClientAddr));
	iConnectFd = accept(g_iListenFd, (SA *)&stClientAddr, &clientAddrlen);
	if (-1 == iConnectFd) {
        perror("accept");
		close(g_iListenFd);
	    return FILE_SERVER_ERROR;
	}

    file_running("client connect is success!\n");
    memset(cClientIpStr_a, 0, sizeof(cClientIpStr_a));
	inet_ntop(AF_INET, &stClientAddr.sin_addr.s_addr, cClientIpStr_a, sizeof(cClientIpStr_a));
	file_running("client port is: %d,  ip is: %s\n", ntohs(stClientAddr.sin_port), cClientIpStr_a);

    iRet = pthread_create(&threadId, NULL, (void *)server_client_data_deal, (void *)iConnectFd);
	if (0 != iRet)
	{
        perror("pthread_create");
		close(g_iListenFd);
		close(iConnectFd);
		return FILE_SERVER_ERROR;
	}
	iRet = pthread_detach(threadId);
	if (0 != iRet)
	{
        perror("pthread_detach");
		close(g_iListenFd);
		close(iConnectFd);
		return FILE_SERVER_ERROR;
	}

	return FILE_SERVER_OK;
}

