#include <stdio.h>
#include <unistd.h>
#include "file_server.h"
#include "file_server_debug.h"

#define SERVER_PORT 8888     

int main(int argc, char *argv[])
{
    int iRet = FILE_SERVER_OK;

    iRet = server_init_socket(SERVER_PORT);
	if (iRet == FILE_SERVER_ERROR)
	{
        file_error("init the socket is failed!\n");
		return -1;
	}

    file_printf("Tcp Server is listen port 8888:\n");

	while (1)
	{
        iRet = server_deal_client_request();
		if (iRet == FILE_SERVER_ERROR) {
            file_error("server_deal_client_request is failed!\n");
		    return -1;    
		}
	}

	close(server_get_listenfd());
    return 0;
}






