#include <stdio.h>
#include "file_client.h"
#include "file_debug.h"

#define SERVER_PORT 8888
#define SERVER_IP   "192.168.10.100" 

int main(int argc, char *argv[])
{
    int iRet = FILE_CLIENT_OK;

	file_trace();

	iRet = init_socket();
	if (iRet == FILE_CLIENT_ERROR)
	{
        file_error("init_socket is failed!\n");
		return FILE_CLIENT_ERROR;   
	}

	iRet = send_request(SERVER_PORT, SERVER_IP);
	if (iRet == FILE_CLIENT_ERROR)
	{
        file_error("send_request is failed!\n");
		return FILE_CLIENT_ERROR;   
	}

	file_printf("connect server is success!\n");
	file_printf("start interaction data!\n");

	while (1)
	{
        iRet = data_interaction();
	    if (iRet == FILE_CLIENT_ERROR)
	    {
            file_error("data_interaction is failed!\n");
		    return FILE_CLIENT_ERROR;   
	    }
	}
}


