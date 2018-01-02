#include "file_server.h"
#include <stdio.h>

#define SERVER_PORT 8888     

int main(int argc, char *argv[])
{
    int iRet = FILE_SERVER_OK;

    iRet = init_socket(SERVER_PORT);
	if (iRet == FILE_SERVER_ERROR)
	{
        printf("init the socket is failed!\n");
		return -1;
	}

    printf("Tcp Server is listen port 8888:\n");

	while (1)
	{
        iRet = deal_client_request();
	}

	close(get_listenfd());
    return 0;
}






