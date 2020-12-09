#include <stdio.h>
#include <unistd.h>
#include "FileServer.h"
#include "FileServerDebug.h"

#define SERVER_PORT 8888     

int main(int argc, char *argv[])
{
    int iRet = FILE_SERVER_OK;
	int i = 0;

    iRet = Server_initSocket(SERVER_PORT);
    if (iRet == FILE_SERVER_ERROR) 
	{
        File_error("init the socket is failed!\n");
        return -1;
    }

    File_running("Tcp Server is listen port 8888:\n");

    while (1) 
	{
        iRet = Server_dealClientReq();
        if (iRet == FILE_SERVER_ERROR) 
		{
            File_error("Server_dealClientReq is failed!\n");
            return -1;    
        }
    }

    close(Server_getListenFd());
    return 0;
}






