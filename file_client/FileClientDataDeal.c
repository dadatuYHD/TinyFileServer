#include <assert.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <cjson/cJSON.h>
#include "FileClientTlv.h"
#include "FileClientDataDeal.h"
#include "FileClientDebug.h"
#include "Message.pb-c.h"
#include "FileClient.h"
#include "FileClientState.h"
#include "FileClientInput.h"



#define BUFSIZE 1024

TestHdr_S g_stTestHdr;    //Global uiData header

int DataDeal_protoPack(FILEDATA* pstUnpackBuf, char** pcPackBuf);
int DataDeal_protoUnpack(char* pcPackBuf, FILEDATA* pstUnpackBuf, int iSize);
int DataDeal_fileWrite(int iFd, const void* pBuf, ssize_t nBytes);
int DataDeal_fileRead(int iFd, void* pBuf, ssize_t nBytes);

/************************************************************
* FUNCTION   :DataDeal_getHdr()
* Description:get value of g_stTestHdr 
* Arguments  :none
* return     :return g_stTestHdr
************************************************************/
TestHdr_S DataDeal_getHdr(void)
{
    return g_stTestHdr;
}

/************************************************************
* FUNCTION   :DataDeal_getHdrAddress()
* Description:get address value of g_stTestHdr  
* Arguments  :none
* return     :return g_st_test_hdr address
************************************************************/
TestHdr_S * DataDeal_getHdrAddress(void)
{
    return &g_stTestHdr;
}



/*******************************************************************
* FUNCTION        :DataDeal_setHdr()
* Description     :Set global variable g_stTestHdr between modules
* Arguments       :
* [pstTestHdr][IN]:Point to a TestHdr_S structure to store the 
*                  uiData to be set
* [enFlg][IN]     :Set the enFlg
* return          :success return FILEDATA_DEAL_RET_OK, return 
*                  FILEDATA_DEAL_RET_FAIL
*******************************************************************/
int DataDeal_setHdr(TestHdr_S* pstTestHdr, HDR_FIELD_FLG enFlg)
{
    assert(NULL != pstTestHdr);
        
    if (enFlg == HDR_FIELD_VERSION) 
	{
        g_stTestHdr.m_enVersion = pstTestHdr->m_enVersion;   
    } 
	else if (enFlg == HDR_FIELD_HDR_LEN) 
	{
        g_stTestHdr.m_usHdrLen = pstTestHdr->m_usHdrLen;   
    } 
	else if (enFlg == HDR_FIELD_DATA_LEN) 
	{
        g_stTestHdr.m_uiDataLen = pstTestHdr->m_uiDataLen;   
    } 
	else if (enFlg == HDR_FIELD_MODULE) 
	{
        g_stTestHdr.m_enModule = pstTestHdr->m_enModule;   
    } 
	else if (enFlg == HDR_FIELD_CMD) 
	{
        g_stTestHdr.m_enCmd = pstTestHdr->m_enCmd;   
    } else 
	{
        File_error("[%s]DataDeal_setHdr input flag is error!\n", __FUNCTION__);
        return FILEDATA_DEAL_RET_FAIL;   
    }

    return FILEDATA_DEAL_RET_OK;
}

/************************************************************
* FUNCTION    :DataDeal_fileRead()
* Description :Read file contents
* Arguments   :
* [iFd][IN]   :File descriptor
* [pBuf][OUT] :Buf to store the contents of the file
* [nBytes][IN]:File size to be read
* return      :Returns the total bytes read
************************************************************/
int DataDeal_fileRead(int iFd, void * pBuf, ssize_t nBytes)
{
    ssize_t totalReadBytes = 0;
    ssize_t readBytes = 0;

    while (totalReadBytes < nBytes) 
	{
        readBytes = read(iFd, pBuf, nBytes - totalReadBytes); 
        if (-1 == readBytes) 
		{
            perror("read");
            return FILEDATA_DEAL_RET_FAIL;
        }

        totalReadBytes += readBytes;
    }

    return totalReadBytes;
}

/************************************************************
* FUNCTION    :DataDeal_fileWrite()
* Description :Write uiData to the file specified by the file 
*              descriptor
* Arguments   :
* [iFd][IN]   :File descriptor
* [pBuf][IN]  :Buf to store the file uiData to be read
* [nBytes][IN]:The size of the file to be written
* return      :Return the total bytes written
************************************************************/
int DataDeal_fileWrite(int iFd, const void * pBuf, ssize_t nBytes)
{
    ssize_t totalWriteBytes = 0;
    ssize_t writeBytes = 0;

    while (totalWriteBytes < nBytes) 
	{
        writeBytes = write(iFd, pBuf, nBytes - totalWriteBytes); 
        if (-1 == writeBytes) 
		{
            perror("read");
            return FILEDATA_DEAL_RET_FAIL;
        }

        totalWriteBytes += writeBytes;
    }

    return totalWriteBytes;
}


/************************************************************
* FUNCTION        :DataDeal_setFile()
* Description     :upload files
* Arguments       :
* [iConnectFd][IN]:File descriptor after connection 
*                  establishment
* return          :success return FILEDATA_DEAL_RET_OK 
*                  and fail return FILEDATA_DEAL_RET_FAIL
************************************************************/
int DataDeal_setFile(int iConnectFd)
{
    int iRet = FILE_CLIENT_OK;
    int iSendBytes = 0;
    int iFileFd = 0;
    struct stat stFileInfo;
    ssize_t readBytes = 0;
        
    if (g_stTestHdr.m_enModule == MODULE_TEST_PROTO) 
	{
        FILEDATA stUnpackBuf;
        char* pcPackBuf = NULL;
        int iPackLen = 0; 

        memset(&stUnpackBuf, 0, sizeof(stUnpackBuf));  
        stUnpackBuf.m_pCmdBuf = (char *)malloc(BUFSIZE);
        if (NULL == stUnpackBuf.m_pCmdBuf) 
		{
            perror("malloc");
            File_error("[%s]malloc is fail!\n", __FUNCTION__);
            return FILEDATA_DEAL_RET_FAIL; 
        }
        stUnpackBuf.m_pFileNameBuf = (char *)malloc(BUFSIZE);
        if (NULL == stUnpackBuf.m_pFileNameBuf) 
		{
            perror("malloc");
            File_error("[%s]malloc is fail!\n", __FUNCTION__);
            return FILEDATA_DEAL_RET_FAIL; 
        }
        stUnpackBuf.m_pFileDataBuf = NULL;
        strncpy(stUnpackBuf.m_pCmdBuf, "S", 1);
        stUnpackBuf.m_pCmdBuf[1] = '\0';  

        /*read the filename*/
        File_running("Please input file name:\n");
        iRet = File_inputString(stUnpackBuf.m_pFileNameBuf);
        if (iRet == FILEINPUT_RET_FAIL) 
		{
            File_error("[%s]File_inputString is fail!\n", __FUNCTION__);
            return FILEDATA_DEAL_RET_FAIL;    
        }

        /*open the file*/
        iFileFd = open(stUnpackBuf.m_pFileNameBuf, O_RDONLY);
        if (-1 == iFileFd) 
		{
            perror("open");
            File_error("[%s]open is fail!\n", __FUNCTION__);
            return FILEDATA_DEAL_RET_FAIL; 
        }

        /*Get the size of file*/
        memset(&stFileInfo, 0, sizeof(stFileInfo));
        iRet = stat(stUnpackBuf.m_pFileNameBuf, &stFileInfo);
        if (-1 == iRet) 
		{
            perror("stat");
            File_error("[%s]stat is fail!\n", __FUNCTION__);
            return FILEDATA_DEAL_RET_FAIL; 
        }
        stUnpackBuf.m_uiFileSize = stFileInfo.st_size;
        stUnpackBuf.has_m_uiFileSize = 1;
        

        /*read the file to stUnpackBuf.m_pFileDataBuf*/
        stUnpackBuf.m_pFileDataBuf = (char *)malloc(stFileInfo.st_size);
        if (NULL == stUnpackBuf.m_pFileDataBuf) 
		{
            perror("malloc");
            File_error("[%s]malloc is fail!\n", __FUNCTION__);
            return FILEDATA_DEAL_RET_FAIL; 
        }

        readBytes = DataDeal_fileRead(iFileFd, stUnpackBuf.m_pFileDataBuf, stFileInfo.st_size);
        if (-1 == readBytes) 
		{
            File_error("[%s]DataDeal_fileRead is fail!\n", __FUNCTION__);
            return FILEDATA_DEAL_RET_FAIL;     
        }

        /*read file size check*/
        if (readBytes != stFileInfo.st_size) 
		{
            File_running("file read is fail!\n");   
            return FILEDATA_DEAL_RET_FAIL; 
        }

        iPackLen = DataDeal_protoPack(&stUnpackBuf, &pcPackBuf);
        if (iPackLen == FILEDATA_DEAL_RET_FAIL) 
		{
            File_error("[%s]DataDeal_protoPack is fail!\n", __FUNCTION__);
            return FILEDATA_DEAL_RET_FAIL;
        }

        if (NULL != stUnpackBuf.m_pCmdBuf) 
		{
            free(stUnpackBuf.m_pCmdBuf);
        } 
        if (NULL != stUnpackBuf.m_pFileNameBuf) 
		{
            free(stUnpackBuf.m_pFileNameBuf);   
        }
        if (NULL != stUnpackBuf.m_pFileDataBuf) 
		{
            free(stUnpackBuf.m_pFileDataBuf);   
        }
        close(iFileFd);

        /*pading the g_stTestHdr structure*/
        g_stTestHdr.m_enVersion = VERSION_ONE;
        g_stTestHdr.m_uiDataLen = iPackLen;
        g_stTestHdr.m_usHdrLen = sizeof(g_stTestHdr);

        /*sned the g_stTestHdr*/
        iSendBytes = Client_sendData(iConnectFd, (char *)&g_stTestHdr, sizeof(g_stTestHdr));
        if (iRet == FILE_CLIENT_ERROR) 
		{
            File_error("[%s]Client_sendData is fail\n", __FUNCTION__);
            return FILEDATA_DEAL_RET_FAIL;
        }
        File_printf("Client_sendData send hdr uiData %d bytes\n", iSendBytes);

        /*sned the pack uiData of cmd filename and file content */
        iSendBytes = Client_sendData(iConnectFd, pcPackBuf, iPackLen);
        if (iSendBytes == FILE_CLIENT_ERROR) 
		{
            File_error("[%s]Client_sendData is fail\n", __FUNCTION__);
            return FILEDATA_DEAL_RET_FAIL;
        }
        File_printf("Client_sendData send protobuf pack uiData %d bytes\n", iSendBytes);
        //File_printf("cPackBuf = %s\n", pcPackBuf);
        File_printf("cPackBuf size = %d\n", g_stTestHdr.m_uiDataLen);
        for (int i = 0; i < g_stTestHdr.m_uiDataLen; i++) 
		{
            File_printf("%hhX\n", pcPackBuf[i]);
        }
        
        if (NULL != pcPackBuf) 
		{
            free(pcPackBuf);
        }   
    }
    if (g_stTestHdr.m_enModule == MODULE_TEST_JSON) 
	{

        cJSON* pstCjsonRoot = cJSON_CreateObject();
        char* pcCjsonDataOut = NULL;
        int iCjsonDataSize = 0;
        char cFileNameBuf[BUFSIZE];
        char* pcFileData = NULL;

        /*read the filename*/
        File_running("Please input file name:\n");
        memset(&cFileNameBuf, 0, BUFSIZE);
        iRet = File_inputString(cFileNameBuf);
        if (iRet == FILEINPUT_RET_FAIL) 
		{
            File_error("[%s]File_inputString is fail!\n", __FUNCTION__);
            return FILEDATA_DEAL_RET_FAIL;    
        }

        /*open the file*/
        iFileFd = open(cFileNameBuf, O_RDONLY);
        if (-1 == iFileFd) 
		{
            perror("open");
            File_error("[%s]open is fail!\n", __FUNCTION__);
            return FILEDATA_DEAL_RET_FAIL; 
        }

        /*Get the size of file*/
        memset(&stFileInfo, 0, sizeof(stFileInfo));
        iRet = stat(cFileNameBuf, &stFileInfo);
        if (-1 == iRet) 
		{
            perror("stat");
            File_error("[%s]stat is fail!\n", __FUNCTION__);
            return FILEDATA_DEAL_RET_FAIL; 
        }

        /*read the file to pcFileData*/
        pcFileData = (char *)malloc(stFileInfo.st_size);
        if (NULL == pcFileData) 
		{
            perror("malloc");
            File_error("[%s]malloc is fail!\n", __FUNCTION__);
            return FILEDATA_DEAL_RET_FAIL; 
        }

        readBytes = DataDeal_fileRead(iFileFd, pcFileData, stFileInfo.st_size);
        if (-1 == readBytes) 
		{
            File_error("[%s]DataDeal_fileRead is fail!\n", __FUNCTION__);
            return FILEDATA_DEAL_RET_FAIL;     
        }

        /*read file size check*/
        if (readBytes != stFileInfo.st_size) 
		{
            File_running("file read is fail!\n");   
            return FILEDATA_DEAL_RET_FAIL; 
        }

        /*add the "S" cmd to the json structure*/
        cJSON_AddStringToObject(pstCjsonRoot, "string_cmd", "S");
        /*add the filename to the json structure*/
        cJSON_AddStringToObject(pstCjsonRoot, "string_file_name", cFileNameBuf);
        /*add the file content uiData to the json structure*/
        cJSON_AddStringToObject(pstCjsonRoot, "string_file_data", pcFileData);
        /*add the file size to the json structure*/
        cJSON_AddNumberToObject(pstCjsonRoot, "number_file_size", stFileInfo.st_size);
        
        /*output the json uiData to char*/
        pcCjsonDataOut = cJSON_PrintUnformatted(pstCjsonRoot);
        
        /*judge the uiData size of bytes*/
        iCjsonDataSize = strlen(pcCjsonDataOut);

        /*pading the g_stTestHdr structure*/
        g_stTestHdr.m_enVersion = VERSION_ONE;
        g_stTestHdr.m_uiDataLen = iCjsonDataSize + 1;
        g_stTestHdr.m_usHdrLen = sizeof(g_stTestHdr);

        /*sned the g_stTestHdr*/
        iSendBytes = Client_sendData(iConnectFd, (char *)&g_stTestHdr, sizeof(g_stTestHdr));
        if (iRet == FILE_CLIENT_ERROR) 
		{
            File_error("[%s]Client_sendData is fail\n", __FUNCTION__);
            return FILEDATA_DEAL_RET_FAIL;
        }
        File_printf("Client_sendData send hdr uiData %d bytes\n", iSendBytes);

        /*sned the cjson pack uiData*/
        iSendBytes = Client_sendData(iConnectFd, pcCjsonDataOut, iCjsonDataSize + 1);
        if (iSendBytes == FILE_CLIENT_ERROR) 
		{
            File_error("[%s]Client_sendData is fail\n", __FUNCTION__);
            return FILEDATA_DEAL_RET_FAIL;
        }

        File_printf("Client_sendData send cjson pack uiData %d bytes\n", iSendBytes);
        File_printf("pcCjsonDataOut = %s\n", pcCjsonDataOut);
        File_printf("pcCjsonDataOut size = %d\n", g_stTestHdr.m_uiDataLen);
        for (int i = 0; i < g_stTestHdr.m_uiDataLen; i++) 
		{
            File_printf("%hhX\n", pcCjsonDataOut[i]);
        }

        if (NULL != pstCjsonRoot) 
		{
            cJSON_Delete(pstCjsonRoot);
            pstCjsonRoot = NULL;
        }
        if (NULL != pcCjsonDataOut) 
		{
            free(pcCjsonDataOut);
            pcCjsonDataOut = NULL;
        }
        if (NULL != pcFileData) 
		{
            free(pcFileData);
            pcFileData = NULL;
        }  
    }
    if (g_stTestHdr.m_enModule == MODULE_TEST_TLV) 
	{
        
        FileData_S stFileData;
        char cPackBuf[BUFSIZE];
        unsigned int uiTlvTotolLen = 0; 
        int iRet = TLV_ENCODE_RET_OK;

        /*read the filename*/
        File_running("Please input file name:\n");
        memset(&stFileData, 0, sizeof(stFileData));
        iRet = File_inputString(stFileData.m_cFileName);
        if (iRet == FILEINPUT_RET_FAIL) 
		{
            File_error("[%s]File_inputString is fail!\n", __FUNCTION__);
            return FILEDATA_DEAL_RET_FAIL;    
        }

        /*open the file*/
        iFileFd = open(stFileData.m_cFileName, O_RDONLY);
        if (-1 == iFileFd) 
		{
            perror("open");
            File_error("[%s]open is fail!\n", __FUNCTION__);
            return FILEDATA_DEAL_RET_FAIL; 
        }

        /*Get the size of file*/
        memset(&stFileInfo, 0, sizeof(stFileInfo));
        iRet = stat(stFileData.m_cFileName, &stFileInfo);
        if (-1 == iRet) 
		{
            perror("stat");
            File_error("[%s]stat is fail!\n", __FUNCTION__);
            return FILEDATA_DEAL_RET_FAIL; 
        }

        /*read the file to pcFileData*/
        stFileData.m_pcFileContent = (char *)malloc(stFileInfo.st_size + 1);
        if (NULL == stFileData.m_pcFileContent) 
		{
            perror("malloc");
            File_error("[%s]malloc is fail!\n", __FUNCTION__);
            return FILEDATA_DEAL_RET_FAIL; 
        }

        readBytes = DataDeal_fileRead(iFileFd, stFileData.m_pcFileContent, stFileInfo.st_size);
        if (-1 == readBytes) 
		{
            File_error("[%s]DataDeal_fileRead is fail!\n", __FUNCTION__);
            return FILEDATA_DEAL_RET_FAIL;     
        }

        /*read file size check*/
        if (readBytes != stFileInfo.st_size) 
		{
            File_running("file read is fail!\n");   
            return FILEDATA_DEAL_RET_FAIL; 
        }

        /*pading the FileData_S structure*/
        strcpy(stFileData.m_cFileCmd, "S");
        stFileData.m_cFileCmd[1] = '\0';
        stFileData.m_uiFileSize = stFileInfo.st_size;
        stFileData.m_uiDataTotolSize = 12 + 12 + 4 + stFileData.m_uiFileSize; 

        /*tlv encode the stFileData*/
        iRet = tlvEncodeFile(&stFileData, cPackBuf, &uiTlvTotolLen, BUFSIZE);
        if (TLV_ENCODE_RET_FAIL == iRet) 
		{
            File_error("[%s]tlvEncodeFile is fail\n", __FUNCTION__); 
            return FILEDATA_DEAL_RET_FAIL;
        }

        /*pading the g_stTestHdr structure*/
        g_stTestHdr.m_enVersion = VERSION_ONE;
        g_stTestHdr.m_uiDataLen = uiTlvTotolLen;
        g_stTestHdr.m_usHdrLen = sizeof(g_stTestHdr);

        /*sned the g_stTestHdr*/
        iSendBytes = Client_sendData(iConnectFd, (char *)&g_stTestHdr, sizeof(g_stTestHdr));
        if (iRet == FILE_CLIENT_ERROR) 
		{
            File_error("[%s]Client_sendData is fail\n", __FUNCTION__);
            return FILEDATA_DEAL_RET_FAIL;
        }
        File_printf("Client_sendData send hdr uiData %d bytes\n", iSendBytes);

        /*sned the tlv pack uiData*/
        iSendBytes = Client_sendData(iConnectFd, cPackBuf, uiTlvTotolLen);
        if (iSendBytes == FILE_CLIENT_ERROR) 
		{
            File_error("[%s]Client_sendData is fail\n", __FUNCTION__);
            return FILEDATA_DEAL_RET_FAIL;
        }

        File_printf("Client_sendData send tlv pack uiData %d bytes\n", iSendBytes);
        File_printf("cPackBuf = %s\n", cPackBuf);
        File_printf("uiTlvTotolLen size = %d\n", g_stTestHdr.m_uiDataLen);
        for (int i = 0; i < g_stTestHdr.m_uiDataLen; i++) 
		{
            File_printf("%hhX\n", cPackBuf[i]);
        }

        if (NULL != stFileData.m_pcFileContent) 
		{
            free(stFileData.m_pcFileContent);
            stFileData.m_pcFileContent = NULL;
        }
        close(iFileFd);
    }    
}

/************************************************************
* FUNCTION        :DataDeal_setFile()
* Description     :download file
* Arguments       :
* [iConnectFd][IN]:File descriptor after connection 
*                  establishment
* return          :success return FILEDATA_DEAL_RET_OK 
*                  and fail return FILEDATA_DEAL_RET_FAIL
************************************************************/
int DataDeal_getFile(int iConnectFd)
{
    int iRet = FILE_CLIENT_OK;
    int iSendBytes = 0;
    int iRecvDataBytes = 0;
    int iFileFd = 0;
    TestHdr_S stTestHdr;  
    ssize_t writeBytes = 0;

    if (g_stTestHdr.m_enModule == MODULE_TEST_PROTO) 
	{
        
        FILEDATA stUnpackBuf;
        char* pcPackBuf = NULL;
        int iPackLen = 0;

        memset(&stUnpackBuf, 0, sizeof(stUnpackBuf));
        stUnpackBuf.m_pCmdBuf = (char *)malloc(BUFSIZE);
        if (NULL == stUnpackBuf.m_pCmdBuf) 
		{
            perror("malloc");
            File_error("[%s]malloc is fail!\n", __FUNCTION__);
            return FILEDATA_DEAL_RET_FAIL; 
        }
        stUnpackBuf.m_pFileNameBuf = (char *)malloc(BUFSIZE);
        if (NULL == stUnpackBuf.m_pFileNameBuf) 
		{
            perror("malloc");
            File_error("[%s]malloc is fail!\n", __FUNCTION__);
            return FILEDATA_DEAL_RET_FAIL; 
        }
        stUnpackBuf.m_pFileDataBuf = NULL;
        stUnpackBuf.has_m_uiFileSize = 0;
        strncpy(stUnpackBuf.m_pCmdBuf, "G", 1);
        stUnpackBuf.m_pCmdBuf[1] = '\0';  

        /*read the filename*/
        File_running("Please input file name:\n");
        iRet = File_inputString(stUnpackBuf.m_pFileNameBuf);
        if (iRet == FILEINPUT_RET_FAIL) 
		{
            File_error("[%s]File_inputString is fail!\n", __FUNCTION__);
            return FILEDATA_DEAL_RET_FAIL;    
        }

        /*pack the origin uiData*/
        iPackLen = DataDeal_protoPack(&stUnpackBuf, &pcPackBuf);
        if (iPackLen == FILEDATA_DEAL_RET_FAIL) 
		{
            File_error("[%s]DataDeal_protoPack is fail!\n", __FUNCTION__);
            return FILEDATA_DEAL_RET_FAIL;
        }

        if (NULL != stUnpackBuf.m_pCmdBuf) 
		{
            free(stUnpackBuf.m_pCmdBuf);
            stUnpackBuf.m_pCmdBuf = NULL;
        } 
        if (NULL != stUnpackBuf.m_pFileNameBuf) 
		{
            free(stUnpackBuf.m_pFileNameBuf);  
            stUnpackBuf.m_pFileNameBuf = NULL;
        }
        if (NULL != stUnpackBuf.m_pFileDataBuf) 
		{
            free(stUnpackBuf.m_pFileDataBuf);  
            stUnpackBuf.m_pFileDataBuf = NULL;
        }

        /*pading the g_stTestHdr structure*/
        g_stTestHdr.m_enVersion = VERSION_ONE;
        g_stTestHdr.m_uiDataLen = iPackLen;
        g_stTestHdr.m_usHdrLen = sizeof(g_stTestHdr);

        /*sned the g_stTestHdr*/
        iSendBytes = Client_sendData(iConnectFd, (char *)&g_stTestHdr, sizeof(g_stTestHdr));
        if (iRet == FILE_CLIENT_ERROR) 
		{
            File_error("[%s]Client_sendData is fail\n", __FUNCTION__);
            return FILEDATA_DEAL_RET_FAIL;
        }
        File_printf("Client_sendData send hdr uiData %d bytes\n", iSendBytes);

        /*sned the pack uiData*/
        iSendBytes = Client_sendData(iConnectFd, pcPackBuf, iPackLen);
        if (iSendBytes == FILE_CLIENT_ERROR) 
		{
            File_error("[%s]Client_sendData is fail\n", __FUNCTION__);
            return FILEDATA_DEAL_RET_FAIL;
        }
        File_printf("Client_sendData send protobuf pack uiData %d bytes\n", iSendBytes);
        //File_printf("cPackBuf = %s\n", pcPackBuf);
        File_printf("cPackBuf size = %d\n", g_stTestHdr.m_uiDataLen);
        for (int i = 0; i < g_stTestHdr.m_uiDataLen; i++) 
		{
            File_printf("%hhX\n", pcPackBuf[i]);
        }

        if (NULL != pcPackBuf) 
		{
            free(pcPackBuf);
            pcPackBuf = NULL;
        }

        /*recv file content size of hdr*/
        memset(&stTestHdr, 0, sizeof(stTestHdr));
        iRecvDataBytes = Client_recvData(iConnectFd, &stTestHdr, sizeof(stTestHdr)); 
        if (iRecvDataBytes == FILE_CLIENT_ERROR) 
		{
            File_error("[%s]Client_recvData is error, close server!!\n", __FUNCTION__);
            close(iConnectFd);
            return FILEDATA_DEAL_RET_FAIL;
        } else if (iRecvDataBytes == FILE_CLIENT_RECV_PEER_DOWN) 
		{
            File_running("[%s]SERVER close the connect!\n", __FUNCTION__);    
        } else 
		{
            File_running("recv server hdr is success!\n");
            File_printf("Client_recvData recv protobuf pack uiData %d bytes\n", iRecvDataBytes);
        } 

        /*recv pack file content uiData from sever*/
        pcPackBuf = (char *)malloc(stTestHdr.m_uiDataLen);
        if (NULL == pcPackBuf) 
		{
            perror("malloc");
            File_error("[%s]malloc is fail!\n", __FUNCTION__);
            return FILEDATA_DEAL_RET_FAIL;      
        }
        iRecvDataBytes = Client_recvData(iConnectFd, (char *)pcPackBuf, stTestHdr.m_uiDataLen); 
        if (iRecvDataBytes == FILE_CLIENT_ERROR) 
		{
            File_error("[%s]Client_recvData is error, close server!!\n", __FUNCTION__);
            close(iConnectFd);
            return FILEDATA_DEAL_RET_FAIL;
        } 
		else if (iRecvDataBytes == FILE_CLIENT_RECV_PEER_DOWN) 
		{
            File_running("[%s]SERVER close the connect!\n", __FUNCTION__);    
        } 
		else 
		{
            /*check having read file size */
            if (iRecvDataBytes != stTestHdr.m_uiDataLen) 
			{
                File_running("file size is checking fail!\n");  
                return FILEDATA_DEAL_RET_FAIL; 
            }
            File_running("recv server uiData is success!\n");
            File_printf("Client_recvData recv protobuf pack uiData %d bytes\n", iRecvDataBytes);
        } 
        
        /*unpack the pack uiData*/
        memset(&stUnpackBuf, 0, sizeof(stUnpackBuf));
        iRet = DataDeal_protoUnpack(pcPackBuf, &stUnpackBuf, stTestHdr.m_uiDataLen);
        if (iRet == FILEDATA_DEAL_RET_FAIL) 
		{
            File_error("[%s]DataDeal_protoUnpack is error!\n", __FUNCTION__);
            return FILEDATA_DEAL_RET_FAIL;      
        }

        if (NULL != pcPackBuf) 
		{
            free(pcPackBuf);
            pcPackBuf = NULL;
        }
    
        /*open the file*/
        while (1) 
		{
            iFileFd = open(stUnpackBuf.m_pFileNameBuf, O_WRONLY|O_CREAT|O_EXCL);
            if (-1 == iFileFd) 
			{
                if (errno == EEXIST) 
				{
                    iRet = remove(stUnpackBuf.m_pFileNameBuf);
                    if (-1 == iRet) 
					{
                        perror("remove");
                        File_error("[%s]remove is error!\n", __FUNCTION__);
                        return 0;
                    }
                    continue;
                } 
				else 
				{
                    perror("open");
                    File_error("[%s]open is fail!\n", __FUNCTION__);
                    return FILEDATA_DEAL_RET_FAIL;     
                }
            } 
			else 
			{
                break;
            }   
        }

        /*write uiData to filename*/
        if (1 == stUnpackBuf.has_m_uiFileSize) 
		{
            writeBytes = DataDeal_fileWrite(iFileFd, stUnpackBuf.m_pFileDataBuf, stUnpackBuf.m_uiFileSize);
            if (-1 == writeBytes) 
			{
                File_error("[%s]DataDeal_fileWrite is fail!\n", __FUNCTION__);
                return FILEDATA_DEAL_RET_FAIL;     
            }   
        } 
		else 
		{
            writeBytes = DataDeal_fileWrite(iFileFd, stUnpackBuf.m_pFileDataBuf, BUFSIZE);
            if (-1 == writeBytes) 
			{
                File_error("[%s]DataDeal_fileWrite is fail!\n", __FUNCTION__);
                return FILEDATA_DEAL_RET_FAIL;     
            }       
        }
        File_running("file write is success!\n");

        if (NULL != stUnpackBuf.m_pCmdBuf) 
		{
            free(stUnpackBuf.m_pCmdBuf);
            stUnpackBuf.m_pCmdBuf = NULL;
        }
        if (NULL != stUnpackBuf.m_pFileNameBuf) 
		{
            free(stUnpackBuf.m_pFileNameBuf); 
            stUnpackBuf.m_pCmdBuf = NULL;
        }
        if (NULL != stUnpackBuf.m_pFileDataBuf) 
		{
            free(stUnpackBuf.m_pFileDataBuf); 
            stUnpackBuf.m_pCmdBuf = NULL;
        }
        close(iFileFd);
        
    }
    if (g_stTestHdr.m_enModule == MODULE_TEST_JSON) 
	{

        cJSON* pstCjsonRoot = cJSON_CreateObject();
        char* pcCjsonDataOut = NULL;
        int iCjsonDataSize = 0;
        char cFileNameBuf[BUFSIZE];
        char* pcFileData = NULL; 

        /*read the filename*/
        File_running("Please input file name:\n");
        memset(&cFileNameBuf, 0, BUFSIZE);
        iRet = File_inputString(cFileNameBuf);
        if (iRet == FILEINPUT_RET_FAIL) {
            File_error("[%s]File_inputString is fail!\n", __FUNCTION__);
            return FILEDATA_DEAL_RET_FAIL;    
        }

        /*add the "G" cmd to the json structure*/
        cJSON_AddStringToObject(pstCjsonRoot, "string_cmd", "G");
        /*add the filename to the json structure*/
        cJSON_AddStringToObject(pstCjsonRoot, "string_file_name", cFileNameBuf);
        
        /*output the json uiData to char*/
        pcCjsonDataOut = cJSON_PrintUnformatted(pstCjsonRoot);
        
        /*judge the uiData size of bytes*/
        iCjsonDataSize = strlen(pcCjsonDataOut);

        /*pading the g_stTestHdr structure*/
        g_stTestHdr.m_enVersion = VERSION_ONE;
        g_stTestHdr.m_uiDataLen = iCjsonDataSize + 1;
        g_stTestHdr.m_usHdrLen = sizeof(g_stTestHdr);

        /*sned the g_stTestHdr*/
        iSendBytes = Client_sendData(iConnectFd, (char *)&g_stTestHdr, sizeof(g_stTestHdr));
        if (iRet == FILE_CLIENT_ERROR) 
		{
            File_error("[%s]Client_sendData is fail\n", __FUNCTION__);
            return FILEDATA_DEAL_RET_FAIL;
        }
        File_printf("Client_sendData send hdr uiData %d bytes\n", iSendBytes);

        /*sned the cjson pack uiData*/
        iSendBytes = Client_sendData(iConnectFd, pcCjsonDataOut, iCjsonDataSize + 1);
        if (iSendBytes == FILE_CLIENT_ERROR) 
		{
            File_error("[%s]Client_sendData is fail\n", __FUNCTION__);
            return FILEDATA_DEAL_RET_FAIL;
        }
        File_printf("Client_sendData send cjson pack uiData %d bytes\n", iSendBytes);
        File_printf("pcCjsonDataOut = %s\n", pcCjsonDataOut);
        File_printf("pcCjsonDataOut size = %d\n", g_stTestHdr.m_uiDataLen);
        for (int i = 0; i < g_stTestHdr.m_uiDataLen; i++) 
		{
            File_printf("%hhX\n", pcCjsonDataOut[i]);
        }

        cJSON_Delete(pstCjsonRoot);
        if (NULL != pcCjsonDataOut) 
		{
            free(pcCjsonDataOut);
            pcCjsonDataOut = NULL;
        }

        /*recv file content size of hdr*/
        memset(&stTestHdr, 0, sizeof(stTestHdr));
        iRecvDataBytes = Client_recvData(iConnectFd, &stTestHdr, sizeof(stTestHdr)); 
        if (iRecvDataBytes == FILE_CLIENT_ERROR) 
		{
            File_error("[%s]Client_recvData is error, close server!!\n", __FUNCTION__);
            close(iConnectFd);
            return FILEDATA_DEAL_RET_FAIL;
        } 
		else if (iRecvDataBytes == FILE_CLIENT_RECV_PEER_DOWN) 
		{
            File_running("[%s]SERVER close the connect!\n", __FUNCTION__);    
        } 
		else 
		{
            File_running("recv server hdr is success!\n");
            File_printf("Client_recvData recv hdr uiData %d bytes\n", iRecvDataBytes);
        } 

        cJSON* pstJson = NULL; 
        cJSON* pstJsonStrFileName = NULL; 
        cJSON* pstJsonStrFileData = NULL; 
        cJSON* pstJsonNumFileSize = NULL; 

        /*recv pack file content uiData from sever*/
        pcCjsonDataOut = (char *)malloc(stTestHdr.m_uiDataLen);
        if (NULL == pcCjsonDataOut) 
		{
            perror("malloc");
            File_error("[%s]malloc is fail!\n", __FUNCTION__);
            return FILEDATA_DEAL_RET_FAIL;      
        }
        iRecvDataBytes = Client_recvData(iConnectFd, (char *)pcCjsonDataOut, stTestHdr.m_uiDataLen); 
        if (iRecvDataBytes == FILE_CLIENT_ERROR) 
		{
            File_error("[%s]Client_recvData is error, close server!!\n", __FUNCTION__);
            return FILEDATA_DEAL_RET_FAIL;
        } 
		else if (iRecvDataBytes == FILE_CLIENT_RECV_PEER_DOWN) 
		{
            File_running("[%s]SERVER close the connect!\n", __FUNCTION__);    
        } 
		else 
		{
            /*check having read file size */
            if (iRecvDataBytes != stTestHdr.m_uiDataLen) 
			{
                File_running("file size is checking fail!\n");  
                return FILEDATA_DEAL_RET_FAIL; 
            }
            File_running("recv server file content of cjsont pack uiData is success!\n");
            File_printf("Client_recvData recv cjosn pack uiData %d bytes\n", iRecvDataBytes);
        }

        /*Parse the json uiData*/
        pstJson = cJSON_Parse(pcCjsonDataOut);
        if (NULL == pstJson) 
		{
            File_error("[%s]cJSON_Parse is error, close server!!\n", __FUNCTION__);  
             return FILEDATA_DEAL_RET_FAIL;  
        }

        /*get the item that the key name is string_file_data*/
        pstJsonStrFileData = cJSON_GetObjectItem(pstJson, "string_file_data");
        if (cJSON_IsString(pstJsonStrFileData)) 
		{
            if (!strncmp(pstJsonStrFileData->string, "string_file_data", 16)) 
			{
                File_running("[%s]json string_file_data check is success!\n", __FUNCTION__);    
            } 
			else 
			{
                File_running("[%s]json string_file_data check is fail!\n", __FUNCTION__); 
                return FILEDATA_DEAL_RET_FAIL; 
            } 
        }

        /*get the item that the key name is number_file_size*/
        pstJsonNumFileSize = cJSON_GetObjectItem(pstJson, "number_file_size");
        if (cJSON_IsNumber(pstJsonNumFileSize)) 
		{
            if (!strncmp(pstJsonNumFileSize->string, "number_file_size", 16)) 
			{
                File_running("[%s]json number_file_size check is success!\n", __FUNCTION__);    
            } 
			else 
			{
                File_running("[%s]json number_file_size check is fail!\n", __FUNCTION__); 
                return FILEDATA_DEAL_RET_FAIL; 
            } 
        }

        /*open the file*/
        while (1) 
		{
            iFileFd = open(cFileNameBuf, O_WRONLY|O_CREAT|O_EXCL);
            if (-1 == iFileFd) 
			{
                if (errno == EEXIST) 
				{
                    iRet = remove(cFileNameBuf);
                    if (-1 == iRet) 
					{
                        perror("remove");
                        File_error("[%s]remove is error!\n", __FUNCTION__);
                        return 0;
                    }
                    continue;
                } 
				else 
				{
                    perror("open");
                    File_error("[%s]open is fail!\n", __FUNCTION__);
                    return FILEDATA_DEAL_RET_FAIL;     
                }
            } 
			else 
			{
                break;
            }   
        }

        /*write uiData to filename*/      
        writeBytes = DataDeal_fileWrite(iFileFd, pstJsonStrFileData->valuestring, pstJsonNumFileSize->valueint);
        if (-1 == writeBytes) 
		{
            File_error("[%s]DataDeal_fileWrite is fail!\n", __FUNCTION__);
            return FILEDATA_DEAL_RET_FAIL;     
        }   
        File_running("file write is success!\n");

        close(iFileFd);
        if (NULL != pcCjsonDataOut) 
		{
            free(pcCjsonDataOut);
            pcCjsonDataOut = NULL;
        }
        if (NULL != pstJson) 
		{
            cJSON_Delete(pstJson);
            pstJson = NULL;
        }
    }
    if (g_stTestHdr.m_enModule == MODULE_TEST_TLV) 
	{
        
        FileData_S stFileData;
        char cPackBuf[BUFSIZE];
        unsigned int uiTlvTotolLen = 0; 
        int iRet = TLV_ENCODE_RET_OK;

        /*read the filename*/
        File_running("Please input file name:\n");
        memset(&stFileData, 0, sizeof(FileData_S));
        iRet = File_inputString(stFileData.m_cFileName);
        if (iRet == FILEINPUT_RET_FAIL) 
		{
            File_error("[%s]File_inputString is fail!\n", __FUNCTION__);
            return FILEDATA_DEAL_RET_FAIL;    
        }

        /*pading the FileData_S structure*/
        strcpy(stFileData.m_cFileCmd, "G");
        stFileData.m_cFileCmd[1] = '\0';
        stFileData.m_uiDataTotolSize = 12 + 12 + 4 + stFileData.m_uiFileSize; 

        /*tlv encode the stFileData*/
        memset(cPackBuf, 0, sizeof(cPackBuf));
        iRet = tlvEncodeFile(&stFileData, cPackBuf, &uiTlvTotolLen, BUFSIZE);
        if (TLV_ENCODE_RET_FAIL == iRet) 
		{
            File_error("[%s]tlvEncodeFile is fail\n", __FUNCTION__); 
            return FILEDATA_DEAL_RET_FAIL;
        }

        /*pading the g_stTestHdr structure*/
        g_stTestHdr.m_enVersion = VERSION_ONE;
        g_stTestHdr.m_uiDataLen = uiTlvTotolLen;
        g_stTestHdr.m_usHdrLen = sizeof(g_stTestHdr);

        /*sned the g_stTestHdr*/
        iSendBytes = Client_sendData(iConnectFd, (char *)&g_stTestHdr, sizeof(g_stTestHdr));
        if (iRet == FILE_CLIENT_ERROR) 
		{
            File_error("[%s]Client_sendData is fail\n", __FUNCTION__);
            return FILEDATA_DEAL_RET_FAIL;
        }
        File_printf("Client_sendData send hdr uiData %d bytes\n", iSendBytes);

        /*sned the tlv pack uiData*/
        iSendBytes = Client_sendData(iConnectFd, cPackBuf, uiTlvTotolLen);
        if (iSendBytes == FILE_CLIENT_ERROR) 
		{
            File_error("[%s]Client_sendData is fail\n", __FUNCTION__);
            return FILEDATA_DEAL_RET_FAIL;
        }

        File_printf("Client_sendData send tlv pack uiData %d bytes\n", iSendBytes);
        File_printf("cPackBuf = %s\n", cPackBuf);
        File_printf("uiTlvTotolLen size = %d\n", g_stTestHdr.m_uiDataLen);
        for (int i = 0; i < g_stTestHdr.m_uiDataLen; i++) 
		{
            File_printf("%hhX\n", cPackBuf[i]);
        }

        /*recv file content size of hdr*/
        memset(&stTestHdr, 0, sizeof(stTestHdr));
        iRecvDataBytes = Client_recvData(iConnectFd, &stTestHdr, sizeof(stTestHdr)); 
        if (iRecvDataBytes == FILE_CLIENT_ERROR) 
		{
            File_error("[%s]Client_recvData is error, close server!!\n", __FUNCTION__);
            close(iConnectFd);
            return FILEDATA_DEAL_RET_FAIL;
        } 
	    else if (iRecvDataBytes == FILE_CLIENT_RECV_PEER_DOWN) 
		{
            File_running("[%s]SERVER close the connect!\n", __FUNCTION__);    
        } 
		else 
		{
            File_running("recv server hdr is success!\n");
            File_printf("Client_recvData recv hdr uiData %d bytes\n", iRecvDataBytes);
        } 

        /*recv pack file content uiData from sever*/
        memset(cPackBuf, 0, sizeof(cPackBuf));
        iRecvDataBytes = Client_recvData(iConnectFd, (char *)cPackBuf, stTestHdr.m_uiDataLen); 
        if (iRecvDataBytes == FILE_CLIENT_ERROR) 
		{
            File_error("[%s]Client_recvData is error, close server!!\n", __FUNCTION__);
            return FILEDATA_DEAL_RET_FAIL;
        } 
		else if (iRecvDataBytes == FILE_CLIENT_RECV_PEER_DOWN) 
		{
            File_running("[%s]SERVER close the connect!\n", __FUNCTION__);    
        } 
		else 
		{
            /*check having read file size */
            if (iRecvDataBytes != stTestHdr.m_uiDataLen) 
			{
                File_running("file size is checking fail!\n");  
                return FILEDATA_DEAL_RET_FAIL; 
            }
            File_running("recv server file content of tlv pack uiData is success!\n");
            File_printf("Client_recvData recv tlv pack uiData %d bytes\n", iRecvDataBytes);
        }

        /*decode the tlv pack uiData*/
        memset(&stFileData, 0, sizeof(FileData_S));
        stFileData.m_pcFileContent = (char *)malloc(stTestHdr.m_uiDataLen);
        if (NULL == stFileData.m_pcFileContent) 
		{
            perror("malloc");
            File_error("[%s]malloc is fail!\n", __FUNCTION__);
            return FILEDATA_DEAL_RET_FAIL; 
        }
        iRet = tlvDecodeFile(cPackBuf, stTestHdr.m_uiDataLen, &stFileData);
        if (TLV_DECODE_RET_FAIL == iRet) 
		{
            File_error("[%s]tlvDecodeFile is fail!\n", __FUNCTION__);  
            return FILEDATA_DEAL_RET_FAIL;
        }

        /*open the file*/
        while (1) 
		{
            iFileFd = open(stFileData.m_cFileName, O_WRONLY|O_CREAT|O_EXCL);
            if (-1 == iFileFd) 
			{
                if (errno == EEXIST) 
				{
                    iRet = remove(stFileData.m_cFileName);
                    if (-1 == iRet) 
					{
                        perror("remove");
                        File_error("[%s]remove is error!\n", __FUNCTION__);
                        return 0;
                    }
                    continue;
                } 
				else 
				{
                    perror("open");
                    File_error("[%s]open is fail!\n", __FUNCTION__);
                    return FILEDATA_DEAL_RET_FAIL;     
                }
            } 
			else 
			{
                break;
            }   
        }

        /*write uiData to filename*/  
        writeBytes = DataDeal_fileWrite(iFileFd, stFileData.m_pcFileContent, stFileData.m_uiFileSize);
        if (-1 == writeBytes) 
		{
            File_error("[%s]DataDeal_fileWrite is fail!\n", __FUNCTION__);
            return FILEDATA_DEAL_RET_FAIL;     
        }
        File_running("file write is success!\n");

        close(iFileFd);
        if (NULL != stFileData.m_pcFileContent)
		{
            free(stFileData.m_pcFileContent);
            stFileData.m_pcFileContent = NULL;
        }
    }    
}

/************************************************************
* FUNCTION        :DataDeal_listFile()
* Description     :Get a list of file servers
* Arguments       :
* [iConnectFd][IN]:File descriptor after connection 
*                  establishment
* return          :success return FILEDATA_DEAL_RET_OK and 
*                  fail return FILEDATA_DEAL_RET_FAIL
************************************************************/
int DataDeal_listFile(int iConnectFd)
{  
    int iRet = FILE_CLIENT_OK;
    int iSendBytes = 0;
    char cFileNameBuf[BUFSIZE];

    if (g_stTestHdr.m_enModule == MODULE_TEST_PROTO) 
	{
        FILEDATA stUnpackBuf;
        char* pcPackBuf = NULL;
        int iPackLen = 0;
    
        memset(&stUnpackBuf, 0, sizeof(stUnpackBuf));
        stUnpackBuf.m_pCmdBuf = (char *)malloc(BUFSIZE);
        stUnpackBuf.m_pFileNameBuf = NULL;
        stUnpackBuf.m_pFileDataBuf = NULL;
        stUnpackBuf.has_m_uiFileSize = 0;
        strncpy(stUnpackBuf.m_pCmdBuf, "L", 1);
        stUnpackBuf.m_pCmdBuf[1] = '\0';  

        /*pack the origin uiData*/
        iPackLen = DataDeal_protoPack(&stUnpackBuf, &pcPackBuf);
        if (iPackLen == FILEDATA_DEAL_RET_FAIL)
		{
            File_error("[%s]DataDeal_protoPack is fail!\n", __FUNCTION__);
            return FILEDATA_DEAL_RET_FAIL;
        }

        if (NULL != stUnpackBuf.m_pCmdBuf) 
		{
            free(stUnpackBuf.m_pCmdBuf);
        } 
        if (NULL != stUnpackBuf.m_pFileNameBuf) 
		{
            free(stUnpackBuf.m_pFileNameBuf);   
        }
        if (NULL != stUnpackBuf.m_pFileDataBuf) 
		{
            free(stUnpackBuf.m_pFileDataBuf);   
        }

        /*pading the g_stTestHdr structure*/
        g_stTestHdr.m_enVersion = VERSION_ONE;
        g_stTestHdr.m_uiDataLen = iPackLen;
        g_stTestHdr.m_usHdrLen = sizeof(g_stTestHdr);

        /*sned the g_stTestHdr*/
        iSendBytes = Client_sendData(iConnectFd, (char *)&g_stTestHdr, sizeof(g_stTestHdr));
        if (iRet == FILE_CLIENT_ERROR) 
		{
            File_error("[%s]Client_sendData is fail\n", __FUNCTION__);
            return FILEDATA_DEAL_RET_FAIL;
        }
        File_printf("Client_sendData send hdr uiData %d bytes\n", iSendBytes);

        /*send the cmd pack uiData*/
        iSendBytes = Client_sendData(iConnectFd, pcPackBuf, iPackLen);
        if (iSendBytes == FILE_CLIENT_ERROR) 
		{
            File_error("[%s]Client_sendData is fail\n", __FUNCTION__);
            return FILEDATA_DEAL_RET_FAIL;
        }
        File_printf("Client_sendData send protobuf pack uiData %d bytes\n", iSendBytes);
        //File_printf("cPackBuf = %s\n", pcPackBuf);
        File_printf("cPackBuf size = %d\n", g_stTestHdr.m_uiDataLen);
        for (int i = 0; i < g_stTestHdr.m_uiDataLen; i++) 
		{
            File_printf("%hhX\n", pcPackBuf[i]);
        }

        if (NULL != pcPackBuf) 
		{
            free(pcPackBuf);
        }

        /*recv file description from server*/
        File_running("server file as follow:\n");
        while (1) 
		{    
            memset(cFileNameBuf, 0, sizeof(cFileNameBuf));
            iRet= Client_recvData(iConnectFd, cFileNameBuf, sizeof(cFileNameBuf));
            if (iRet == FILE_CLIENT_ERROR) 
			{
                File_error("[%s]Client_recvData is fail\n", __FUNCTION__); 
                return FILEDATA_DEAL_RET_OK;
            }
            
            if (strncmp(cFileNameBuf, "end", 3) == 0) 
			{
                File_printf("recv filename uiData complete!\n");
                break;
            }
            
            File_running("%s\n", cFileNameBuf);     
        }
    }
    if (g_stTestHdr.m_enModule == MODULE_TEST_JSON) 
	{
        
        cJSON* pstCjsonRoot = cJSON_CreateObject();
        char* pcCjsonDataOut = NULL;
        int iCjsonDataSize = 0;

        /*pack the "L" cmd to the json structure*/
        cJSON_AddStringToObject(pstCjsonRoot, "string_cmd", "L");
        
        /*output the json uiData to char*/
        pcCjsonDataOut = cJSON_PrintUnformatted(pstCjsonRoot);
        
        /*judge the uiData size of bytes*/
        iCjsonDataSize = strlen(pcCjsonDataOut);

        /*pading the g_stTestHdr structure*/
        g_stTestHdr.m_enVersion = VERSION_ONE;
        g_stTestHdr.m_uiDataLen = iCjsonDataSize + 1;
        g_stTestHdr.m_usHdrLen = sizeof(g_stTestHdr);

        /*sned the g_stTestHdr*/
        iSendBytes = Client_sendData(iConnectFd, (char *)&g_stTestHdr, sizeof(g_stTestHdr));
        if (iRet == FILE_CLIENT_ERROR) 
		{
            File_error("[%s]Client_sendData is fail\n", __FUNCTION__);
            return FILEDATA_DEAL_RET_FAIL;
        }
        File_printf("Client_sendData send hdr uiData %d bytes\n", iSendBytes);

        /*sned the cmd cjson pack uiData*/
        iSendBytes = Client_sendData(iConnectFd, pcCjsonDataOut, iCjsonDataSize + 1);
        if (iSendBytes == FILE_CLIENT_ERROR) 
		{
            File_error("[%s]Client_sendData is fail\n", __FUNCTION__);
            return FILEDATA_DEAL_RET_FAIL;
        }

        File_printf("Client_sendData send cjson pack uiData %d bytes\n", iSendBytes);
        File_printf("pcCjsonDataOut = %s\n", pcCjsonDataOut);
        File_printf("pcCjsonDataOut size = %d\n", g_stTestHdr.m_uiDataLen);
        for (int i = 0; i < g_stTestHdr.m_uiDataLen; i++) 
		{
            File_printf("%hhX\n", pcCjsonDataOut[i]);
        }

        cJSON_Delete(pstCjsonRoot);
        free(pcCjsonDataOut);

        /*recv file description from server*/
        File_running("server file as follow:\n");
        while (1) 
		{    
            memset(cFileNameBuf, 0, sizeof(cFileNameBuf));
            iRet= Client_recvData(iConnectFd, cFileNameBuf, sizeof(cFileNameBuf));
            if (iRet == FILE_CLIENT_ERROR) 
			{
                File_error("[%s]Client_recvData is fail\n", __FUNCTION__); 
                return FILEDATA_DEAL_RET_FAIL;
            }
            
            if (strncmp(cFileNameBuf, "end", 3) == 0)
			{
                File_printf("recv filename uiData complete!\n");
                break;
            }
            
            File_running("%s\n", cFileNameBuf);     
        }
    }
    if (g_stTestHdr.m_enModule == MODULE_TEST_TLV) 
	{
        
        FileData_S stFileData;
        char cPackBuf[BUFSIZE];
        unsigned int uiTlvTotolLen = 0; 
        int iRet = TLV_ENCODE_RET_OK;

        /*pading the FileData_S structure*/
        memset(&stFileData, 0, sizeof(FileData_S));
        strcpy(stFileData.m_cFileCmd, "L");
        stFileData.m_cFileCmd[1] = '\0';
        stFileData.m_uiDataTotolSize = 12 + 12 + 4 + stFileData.m_uiFileSize; 

        /*tlv encode the stFileData*/
        iRet = tlvEncodeFile(&stFileData, cPackBuf, &uiTlvTotolLen, BUFSIZE);
        if (TLV_ENCODE_RET_FAIL == iRet) 
		{
            File_error("[%s]tlvEncodeFile is fail\n", __FUNCTION__); 
            return FILEDATA_DEAL_RET_FAIL;
        }

        /*pading the g_stTestHdr structure*/
        g_stTestHdr.m_enVersion = VERSION_ONE;
        g_stTestHdr.m_uiDataLen = uiTlvTotolLen;
        g_stTestHdr.m_usHdrLen = sizeof(g_stTestHdr);

        /*sned the g_stTestHdr*/
        iSendBytes = Client_sendData(iConnectFd, (char *)&g_stTestHdr, sizeof(g_stTestHdr));
        if (iRet == FILE_CLIENT_ERROR) 
		{
            File_error("[%s]Client_sendData is fail\n", __FUNCTION__);
            return FILEDATA_DEAL_RET_FAIL;
        }
        File_printf("Client_sendData send hdr uiData %d bytes\n", iSendBytes);

        /*sned the cmd tlv pack uiData*/
        iSendBytes = Client_sendData(iConnectFd, cPackBuf, uiTlvTotolLen);
        if (iSendBytes == FILE_CLIENT_ERROR) 
		{
            File_error("[%s]Client_sendData is fail\n", __FUNCTION__);
            return FILEDATA_DEAL_RET_FAIL;
        }

        File_printf("Client_sendData send tlv pack uiData %d bytes\n", iSendBytes);
        //File_printf("cPackBuf = %s\n", cPackBuf);
        File_printf("uiTlvTotolLen size = %d\n", g_stTestHdr.m_uiDataLen);
        for (int i = 0; i < g_stTestHdr.m_uiDataLen; i++) 
		{
            File_printf("%hhX\n", cPackBuf[i]);
        }

        /*recv file description from server*/
        File_running("server file as follow:\n");
        while (1) 
		{    
            memset(cFileNameBuf, 0, sizeof(cFileNameBuf));
            iRet= Client_recvData(iConnectFd, cFileNameBuf, sizeof(cFileNameBuf));
            if (iRet == FILE_CLIENT_ERROR) 
			{
                File_error("[%s]Client_recvData is fail\n", __FUNCTION__); 
                return FILEDATA_DEAL_RET_FAIL;
            }
            
            if (strncmp(cFileNameBuf, "end", 3) == 0) 
			{
                File_printf("recv filename uiData complete!\n");
                break;
            }
            
            File_running("%s\n", cFileNameBuf);     
        }
    }   
}

int DataDeal_exitFile(int iConnectFd)
{
    int iRet = FILE_CLIENT_OK;
    int iSendBytes = 0;

    /*pading the g_stTestHdr structure*/
    g_stTestHdr.m_enVersion = VERSION_ONE;
    g_stTestHdr.m_uiDataLen = 0;
    g_stTestHdr.m_usHdrLen = sizeof(g_stTestHdr);

    /*sned the g_stTestHdr*/
    iSendBytes = Client_sendData(iConnectFd, (char *)&g_stTestHdr, sizeof(g_stTestHdr));
    if (iRet == FILE_CLIENT_ERROR) 
	{
        File_error("[%s]Client_sendData is fail\n", __FUNCTION__);
        return FILEDATA_DEAL_RET_FAIL;
    }
    File_printf("Client_sendData send hdr uiData %d bytes\n", iSendBytes); 

    return FILESTATE_MAX;
}

/************************************************************
* FUNCTION          :DataDeal_protoPack()
* Description       :Encode uiData with protobuf protocol
* Arguments         :
* [pstUnpackBuf][IN]:Point to the memory where the original 
*                    uiData is stored
* [pcPackBuf][OUT]  :Save the packaged uiData
* return            :Returns the size of the packed uiData
************************************************************/
int DataDeal_protoPack(FILEDATA *pstUnpackBuf, char **pcPackBuf)
{
    assert(NULL != pstUnpackBuf);
    assert(NULL != pcPackBuf);

    FILEDATA stUnpackData;
    size_t fileDataLen = 0;

    /*init the stUnpackData*/
    file__data__init(&stUnpackData);

    /*padding uiData*/
    if (NULL != pstUnpackBuf->m_pCmdBuf)
    {    
        stUnpackData.m_pCmdBuf = (char *)malloc(BUFSIZE);
        if (NULL == stUnpackData.m_pCmdBuf) 
		{
            File_error("[%s]malloc is error!\n", __FUNCTION__);
            return FILEDATA_DEAL_RET_FAIL;  
        } 
        strncpy(stUnpackData.m_pCmdBuf, pstUnpackBuf->m_pCmdBuf, BUFSIZE);  
    } 
	else 
	{
        stUnpackData.m_pCmdBuf = NULL;
    }
    
    if (NULL != pstUnpackBuf->m_pFileNameBuf) 
	{
        stUnpackData.m_pFileNameBuf = (char *)malloc(BUFSIZE);
        if (NULL == stUnpackData.m_pFileNameBuf) 
		{
            File_error("[%s]malloc is error!\n", __FUNCTION__);
            return FILEDATA_DEAL_RET_FAIL;  
        } 
        strncpy(stUnpackData.m_pFileNameBuf, pstUnpackBuf->m_pFileNameBuf, BUFSIZE);    
    } 
	else 
	{
        stUnpackData.m_pFileNameBuf = NULL;
    }

    if (NULL != pstUnpackBuf->m_pFileDataBuf) 
	{
        stUnpackData.m_pFileDataBuf = (char *)malloc(BUFSIZE);
        if (NULL == stUnpackData.m_pFileDataBuf) 
		{
            File_error("[%s]malloc is error!\n", __FUNCTION__);
            return FILEDATA_DEAL_RET_FAIL;  
        } 
        strncpy(stUnpackData.m_pFileDataBuf, pstUnpackBuf->m_pFileDataBuf, BUFSIZE);    
    } 
	else
	{
        stUnpackData.m_pFileDataBuf = NULL;
    }

    /*get size of pack uiData*/
    fileDataLen = file__data__get_packed_size(&stUnpackData);

    *pcPackBuf = (char *)malloc(fileDataLen);
    if (NULL == *pcPackBuf)
    {
        File_error("[%s]malloc is error!\n", __FUNCTION__);
        return FILEDATA_DEAL_RET_FAIL;
    }

    file__data__pack(&stUnpackData, *pcPackBuf);

    if (NULL != stUnpackData.m_pCmdBuf) 
	{
        free(stUnpackData.m_pCmdBuf);    
    }
    if (NULL != stUnpackData.m_pFileNameBuf) 
	{
        free(stUnpackData.m_pFileNameBuf);     
    }
    if (NULL != stUnpackData.m_pFileDataBuf) 
	{
        free(stUnpackData.m_pFileDataBuf);     
    }
    
    return fileDataLen;
}

/************************************************************
* FUNCTION           :DataDeal_protoUnpack()
* Description        :Decode uiData with protobuf protocol
* Arguments          :
* [pcPackBuf][IN]    :Data to be unpacked
* [pstUnpackBuf][OUT]:Store the unpacked uiData
* return             :success return FILEDATA_DEAL_RET_OK 
*                     and fail return FILEDATA_DEAL_RET_FAIL
************************************************************/
int DataDeal_protoUnpack(char *pcPackBuf, FILEDATA *pstUnpackBuf, int iSize)
{
    assert(NULL != pcPackBuf);  
    assert(NULL != pstUnpackBuf);

    FILEDATA* stUnpackData = NULL;

    stUnpackData = file__data__unpack(NULL, iSize, pcPackBuf);
    if (NULL == stUnpackData) 
	{
        File_error("[%s]file__data__unpack is error!\n", __FUNCTION__);
        return FILEDATA_DEAL_RET_FAIL;    
    }
  
    if (NULL != stUnpackData->m_pCmdBuf) 
	{
        pstUnpackBuf->m_pCmdBuf = (char *)malloc(iSize + 1);
        if (NULL == pstUnpackBuf) 
		{
            File_error("[%s]malloc is error!\n", __FUNCTION__);
            return FILEDATA_DEAL_RET_FAIL;    
        }
        strcpy(pstUnpackBuf->m_pCmdBuf, stUnpackData->m_pCmdBuf);    
    }

    if (NULL != stUnpackData->m_pFileNameBuf) 
	{
        pstUnpackBuf->m_pFileNameBuf = (char *)malloc(iSize + 1);
        if (NULL == pstUnpackBuf) {
            File_error("[%s]malloc is error!\n", __FUNCTION__);
            return FILEDATA_DEAL_RET_FAIL;    
        }
        strcpy(pstUnpackBuf->m_pFileNameBuf, stUnpackData->m_pFileNameBuf);    
    }

    if (NULL != stUnpackData->m_pFileDataBuf) 
	{
        pstUnpackBuf->m_pFileDataBuf = (char *)malloc(iSize + 1);
        if (NULL == pstUnpackBuf) 
		{
            File_error("[%s]malloc is error!\n", __FUNCTION__);
            return FILEDATA_DEAL_RET_FAIL;    
        }
        strcpy(pstUnpackBuf->m_pFileDataBuf, stUnpackData->m_pFileDataBuf);    
    }

    file__data__free_unpacked(stUnpackData,NULL);

    return FILEDATA_DEAL_RET_OK;
}

