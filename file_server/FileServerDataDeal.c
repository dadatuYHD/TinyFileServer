#include <assert.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <cjson/cJSON.h>
#include "FileServerTlv.h"
#include "Message.pb-c.h"
#include "FileServerDatadeal.h"
#include "FileServerDebug.h"
#include "FileServer.h"



#define BUFSIZE 1024

TestHdr_S  g_stTestHdr;     //Global uiData header
TestHdr_S* g_pstTestHdr;    //Global uiData header address


int DataDeal_protoPack(FILEDATA *pcUnpackBuf, char **pcPackBuf);
int DataDeal_protoUnpack(char *pcPackBuf, FILEDATA *pcUnpackBuf, int iSize);
int DataDeal_fileListDeal(const char * pcFileName, int iConnectFd);
int DataDeal_fileRead(int iFd, void * pBuf, ssize_t nBytes);
int DataDeal_fileWrite(int iFd, const void * pBuf, ssize_t nBytes);


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
* FUNCTION          :DataDeal_setHdr()
* Description       :Set global variable g_stTestHdr between modules
* Arguments         :
* [pstTestHdr][IN]:Point to a TestHdr_S structure to store the 
*                    uiData to be set
* [enFlg][IN]      :Set the enFlg
* return            :success return FILEDATA_DEAL_RET_OK, return 
*                    FILEDATA_DEAL_RET_FAIL
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
    }
    else
    {
        File_error("[%s]DataDeal_setHdr input flag is error!\n", __FUNCTION__);
        return FILEDATA_DEAL_RET_FAIL;   
    }

    return FILEDATA_DEAL_RET_OK;
}

/************************************************************
* FUNCTION            :DataDeal_protoPack()
* Description         :Encode uiData with protobuf protocol
* Arguments           :
* [pstUnpackBuf][IN]:Point to the memory where the original 
*                      uiData is stored
* [pcPackBuf][OUT]  :Save the packaged uiData
* return              :Returns the size of the packed uiData
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
        if (NULL == stUnpackData.m_pCmdBuf) {
            File_error("[%s]malloc is error!\n", __FUNCTION__);
            return FILEDATA_DEAL_RET_FAIL;  
        } 
        strncpy(stUnpackData.m_pCmdBuf, pstUnpackBuf->m_pCmdBuf, BUFSIZE);  
    } else {
        stUnpackData.m_pCmdBuf = NULL;
    }
    
    if (NULL != pstUnpackBuf->m_pFileNameBuf) {
        stUnpackData.m_pFileNameBuf = (char *)malloc(BUFSIZE);
        if (NULL == stUnpackData.m_pFileNameBuf) {
            File_error("[%s]malloc is error!\n", __FUNCTION__);
            return FILEDATA_DEAL_RET_FAIL;  
        } 
        strncpy(stUnpackData.m_pFileNameBuf, pstUnpackBuf->m_pFileNameBuf, BUFSIZE);    
    } else {
        stUnpackData.m_pFileNameBuf = NULL;
    }

    if (NULL != pstUnpackBuf->m_pFileDataBuf) {
        stUnpackData.m_pFileDataBuf = (char *)malloc(BUFSIZE);
        if (NULL == stUnpackData.m_pFileDataBuf) {
            File_error("[%s]malloc is error!\n", __FUNCTION__);
            return FILEDATA_DEAL_RET_FAIL;  
        } 
        strncpy(stUnpackData.m_pFileDataBuf, pstUnpackBuf->m_pFileDataBuf, BUFSIZE);    
    } else {
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

    if (NULL != stUnpackData.m_pCmdBuf) {
        free(stUnpackData.m_pCmdBuf);    
    }
    if (NULL != stUnpackData.m_pFileNameBuf) {
        free(stUnpackData.m_pFileNameBuf);     
    }
    if (NULL != stUnpackData.m_pFileDataBuf) {
        free(stUnpackData.m_pFileDataBuf);     
    }
    
    return fileDataLen;
}

/************************************************************
* FUNCTION             :DataDeal_protoUnpack()
* Description          :Decode uiData with protobuf protocol
* Arguments:
* [pcPackBuf][IN]    :Data to be unpacked
* [pstUnpackBuf][OUT]:Store the unpacked uiData
* return               :success return FILEDATA_DEAL_RET_OK 
*                       and fail return FILEDATA_DEAL_RET_FAIL
************************************************************/
int DataDeal_protoUnpack(char* pcPackBuf, FILEDATA* pstUnpackBuf, int iSize)
{
    assert(NULL != pcPackBuf);  
    assert(NULL != pstUnpackBuf);

    FILEDATA* stUnpackData = NULL;

    stUnpackData = file__data__unpack(NULL, iSize, pcPackBuf);
    if (NULL == stUnpackData) {
        File_error("[%s]file__data__unpack is error!\n", __FUNCTION__);
        return FILEDATA_DEAL_RET_FAIL;    
    }
  
    if (NULL != stUnpackData->m_pCmdBuf) {
        pstUnpackBuf->m_pCmdBuf = (char *)malloc(iSize + 1);
        if (NULL == pstUnpackBuf) {
            File_error("[%s]malloc is error!\n", __FUNCTION__);
            return FILEDATA_DEAL_RET_FAIL;    
        }
        strcpy(pstUnpackBuf->m_pCmdBuf, stUnpackData->m_pCmdBuf);    
    }

    if (NULL != stUnpackData->m_pFileNameBuf) {
        pstUnpackBuf->m_pFileNameBuf = (char *)malloc(iSize + 1);
        if (NULL == pstUnpackBuf) {
            File_error("[%s]malloc is error!\n", __FUNCTION__);
            return FILEDATA_DEAL_RET_FAIL;    
        }
        strcpy(pstUnpackBuf->m_pFileNameBuf, stUnpackData->m_pFileNameBuf);    
    }

    if (NULL != stUnpackData->m_pFileDataBuf) {
        pstUnpackBuf->m_pFileDataBuf = (char *)malloc(iSize + 1);
        if (NULL == pstUnpackBuf) {
            File_error("[%s]malloc is error!\n", __FUNCTION__);
            return FILEDATA_DEAL_RET_FAIL;    
        }
        strcpy(pstUnpackBuf->m_pFileDataBuf, stUnpackData->m_pFileDataBuf);    
    }

    file__data__free_unpacked(stUnpackData,NULL);

    return FILEDATA_DEAL_RET_OK;
}

/************************************************************
* FUNCTION    :DataDeal_fileRead()
* Description :Read file contents
* Arguments   :
* [iFd][IN]  :File descriptor
* [pBuf][OUT]   :Buf to store the contents of the file
* [nBytes][IN]:File size to be read
* return      :Returns the total bytes read
************************************************************/
int DataDeal_fileRead(int iFd, void * pBuf, ssize_t nBytes)
{
    ssize_t totalReadBytes = 0;
    ssize_t readBytes = 0;

    while (totalReadBytes < nBytes) {
        readBytes = read(iFd, pBuf, nBytes - totalReadBytes); 
        if (-1 == readBytes) {
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
* [iFd][IN]  :File descriptor
* [pBuf][IN]   :Buf to store the file uiData to be read
* [nBytes][IN]:The size of the file to be written
* return      :Return the total bytes written
************************************************************/
int DataDeal_fileWrite(int iFd, const void * pBuf, ssize_t nBytes)
{
    ssize_t totalWriteBytes = 0;
    ssize_t writeBytes = 0;

    while (totalWriteBytes < nBytes) {
        writeBytes = write(iFd, pBuf, nBytes - totalWriteBytes); 
        if (-1 == writeBytes) {
            perror("read");
            return FILEDATA_DEAL_RET_FAIL;
        }

        totalWriteBytes += writeBytes;
    }

    return totalWriteBytes;
}


/************************************************************
* FUNCTION          :DataDeal_listFile()
* Description       :Process "L" commands sent by the client
* Arguments         :
* [iConnectFd][IN]:File descriptor after connection 
*                    establishment
* return            :success return FILEDATA_DEAL_RET_OK and 
*                    fail return FILEDATA_DEAL_RET_FAIL
************************************************************/
int DataDeal_listFile(int iConnectFd)
{
    int iRet = FILEDATA_DEAL_RET_OK;
    int iRecvDataBytes = 0;
    int iSendBytes = 0;
    char c_end_buf_a[BUFSIZE];
    
    if (g_stTestHdr.m_enModule == MODULE_TEST_PROTO) {

        char cPackBuf[g_stTestHdr.m_uiDataLen + 1];
        FILEDATA stUnpackBuf;
    
        /*recv cmd uiData from client*/
        memset(cPackBuf, 0, sizeof(cPackBuf));
        iRecvDataBytes = Server_recvData(iConnectFd, (char *)cPackBuf, g_stTestHdr.m_uiDataLen); 
        if (iRecvDataBytes == FILE_SERVER_ERROR) {
            File_error("[%s]Server_recvData is error, close server!!\n", __FUNCTION__);
            return FILEDATA_DEAL_RET_FAIL;
        } else if (iRecvDataBytes == FILE_SERVER_RECV_PEER_DOWN) {
            File_running("[%s]client close the connect!\n", __FUNCTION__);    
        } else {
            File_running("recv client uiData is success!\n");
            File_printf("Server_recvData recv protobuf pack uiData %d bytes\n", iRecvDataBytes);
        }

        File_printf("cPackBuf = %s\n", cPackBuf);
        File_printf("cPackBuf size = %d\n", sizeof(cPackBuf));
        for (int i = 0; i < g_stTestHdr.m_uiDataLen; i++) {
            File_printf("%X\n", cPackBuf[i]);
        }   
        
        /*unpack the cmd uiData*/
        memset(&stUnpackBuf, 0, sizeof(stUnpackBuf));
        iRet = DataDeal_protoUnpack(cPackBuf, &stUnpackBuf, g_stTestHdr.m_uiDataLen);
        if (iRet == FILEDATA_DEAL_RET_FAIL) {
            File_error("[%s]DataDeal_protoUnpack is error!\n", __FUNCTION__);
            return FILEDATA_DEAL_RET_FAIL;      
        }

        /*judge the cmd uiData*/
        if (NULL != stUnpackBuf.m_pCmdBuf) {
            if (stUnpackBuf.m_pCmdBuf[0] != 'L') {
                File_printf("recv cmd list, \"L\" cmd check fail!\n");
                return FILEDATA_DEAL_RET_FAIL;
            } else {
                File_printf("recv cmd list, \"L\" cmd check success!\n");
                iRet = DataDeal_fileListDeal(".", iConnectFd);
                if (iRet == FILEDATA_DEAL_RET_FAIL) {
                    File_error("[%s]DataDeal_fileListDeal is error!\n", __FUNCTION__);
                    return FILEDATA_DEAL_RET_FAIL;
                }
            }
        } 

        if (NULL != stUnpackBuf.m_pCmdBuf) {
            free(stUnpackBuf.m_pCmdBuf);
        }
        if (NULL != stUnpackBuf.m_pFileNameBuf) {
            free(stUnpackBuf.m_pFileNameBuf);    
        }
        if (NULL != stUnpackBuf.m_pFileDataBuf) {
            free(stUnpackBuf.m_pFileDataBuf);    
        }

        /*send file describe end mark*/
        memset(c_end_buf_a, 0, sizeof(c_end_buf_a));
        strncpy(c_end_buf_a, "end", 3);
        c_end_buf_a[strlen(c_end_buf_a)] = '\0';
        iSendBytes = server_sendData(iConnectFd, c_end_buf_a, sizeof(c_end_buf_a));
        if (FILE_SERVER_ERROR == iSendBytes) {
            File_error("[%s]server_sendData is fail\n", __FUNCTION__);
            return FILEDATA_DEAL_RET_FAIL;   
        }   
    } else if (g_stTestHdr.m_enModule == MODULE_TEST_JSON) {
    
        cJSON * pstJson = NULL;  
        cJSON * stp_json_str_cmd = NULL; 
        char c_cjson_out_data_a[g_stTestHdr.m_uiDataLen];

        /*recv cmd uiData from client*/
        memset(c_cjson_out_data_a, 0, sizeof(c_cjson_out_data_a));
        iRecvDataBytes = Server_recvData(iConnectFd, (char *)c_cjson_out_data_a, g_stTestHdr.m_uiDataLen); 
        if (iRecvDataBytes == FILE_SERVER_ERROR) {
            File_error("[%s]Server_recvData is error, close server!!\n", __FUNCTION__);
            return FILEDATA_DEAL_RET_FAIL;
        } else if (iRecvDataBytes == FILE_SERVER_RECV_PEER_DOWN) {
            File_running("[%s]client close the connect!\n", __FUNCTION__);    
        } else {
            File_running("recv client uiData is success!\n");
            File_printf("Server_recvData recv CJSON pack uiData %d bytes\n", iRecvDataBytes);
        }
        
        File_printf("c_cjson_out_data_a = %s\n", c_cjson_out_data_a);
        File_printf("c_cjson_out_data_a size = %d\n", sizeof(c_cjson_out_data_a));
        for (int i = 0; i < g_stTestHdr.m_uiDataLen; i++) {
            File_printf("%X\n", c_cjson_out_data_a[i]);
        } 

        /*Parse the json uiData*/
        pstJson = cJSON_Parse(c_cjson_out_data_a);
        if (NULL == pstJson) {
            File_error("[%s]cJSON_Parse is error, close server!!\n", __FUNCTION__);  
             return FILEDATA_DEAL_RET_FAIL;  
        }

        /*get the item that the key name is string_cmd*/
        stp_json_str_cmd = cJSON_GetObjectItem(pstJson, "string_cmd");
        if (cJSON_IsString(stp_json_str_cmd)) {
            if (!strncmp(stp_json_str_cmd->string, "string_cmd", 10)) {
                File_running("[%s]json string_cmd check is success!\n", __FUNCTION__);    
            } else {
                File_running("[%s]json string_cmd check is fail!\n", __FUNCTION__); 
                return FILEDATA_DEAL_RET_FAIL; 
            } 
        }

        /*judge the cmd uiData*/
        if (NULL != stp_json_str_cmd->valuestring) {
            if (strncmp(stp_json_str_cmd->valuestring, "L", 1)) {
                File_printf("recv cmd list, \"L\" cmd check fail!\n");
                return FILEDATA_DEAL_RET_FAIL;
            } else {
                File_printf("recv cmd list, \"L\" cmd check success!\n");
                iRet = DataDeal_fileListDeal(".", iConnectFd);
                if (iRet == FILEDATA_DEAL_RET_FAIL) {
                    File_error("[%s]DataDeal_fileListDeal is error!\n", __FUNCTION__);
                    return FILEDATA_DEAL_RET_FAIL;
                }
            }
        } 

        /*send file describe end mark*/
        memset(c_end_buf_a, 0, sizeof(c_end_buf_a));
        strncpy(c_end_buf_a, "end", 3);
        c_end_buf_a[strlen(c_end_buf_a)] = '\0';
        iSendBytes = server_sendData(iConnectFd, c_end_buf_a, sizeof(c_end_buf_a));
        if (FILE_SERVER_ERROR == iSendBytes) {
            File_error("[%s]server_sendData is fail\n", __FUNCTION__);
            return FILEDATA_DEAL_RET_FAIL;   
        }
        
        cJSON_Delete(pstJson);     
    } else if (g_stTestHdr.m_enModule == MODULE_TEST_TLV) {
        char cPackBuf[BUFSIZE];
        FileData_S stFileData;
        int iRet = TLV_DECODE_RET_OK;

        /*recv cmd uiData from client*/
        memset(cPackBuf, 0, sizeof(cPackBuf));
        iRecvDataBytes = Server_recvData(iConnectFd, (char *)cPackBuf, g_stTestHdr.m_uiDataLen); 
        if (iRecvDataBytes == FILE_SERVER_ERROR) {
            File_error("[%s]Server_recvData is error, close server!\n", __FUNCTION__);
            return FILEDATA_DEAL_RET_FAIL;
        } else if (iRecvDataBytes == FILE_SERVER_RECV_PEER_DOWN) {
            File_running("[%s]client close the connect!\n", __FUNCTION__);    
        } else {
            File_running("recv client uiData is success!\n");
            File_printf("Server_recvData recv tlv pack uiData %d bytes\n", iRecvDataBytes);
        }
        
        File_printf("cPackBuf = %s\n", cPackBuf);
        File_printf("cPackBuf size = %d\n", g_stTestHdr.m_uiDataLen);
        for (int i = 0; i < g_stTestHdr.m_uiDataLen; i++) {
            File_printf("%X\n", cPackBuf[i]);
        } 

        /*tlv decode the cmd of "L" pack uiData*/
        memset(&stFileData, 0, sizeof(stFileData));
        iRet = tlvDecodeFile(cPackBuf, g_stTestHdr.m_uiDataLen, &stFileData);
        if (TLV_DECODE_RET_FAIL == iRet) {
            File_error("[%s]tlvDecodeFile is fail!\n", __FUNCTION__);  
            return FILEDATA_DEAL_RET_FAIL;
        }

        /*judge the cmd uiData*/
        if (strncmp(stFileData.m_cFileCmd, "L", 1)) {
            File_printf("recv cmd list, \"L\" cmd check fail!\n");
            return FILEDATA_DEAL_RET_FAIL;
        } else {
            File_printf("recv cmd list, \"L\" cmd check success!\n");
            iRet = DataDeal_fileListDeal(".", iConnectFd);
            if (iRet == FILEDATA_DEAL_RET_FAIL) {
                File_error("[%s]DataDeal_fileListDeal is error!\n", __FUNCTION__);
                return FILEDATA_DEAL_RET_FAIL;
            }
        }

        /*send file describe end mark*/
        memset(c_end_buf_a, 0, sizeof(c_end_buf_a));
        strncpy(c_end_buf_a, "end", 3);
        c_end_buf_a[strlen(c_end_buf_a)] = '\0';
        iSendBytes = server_sendData(iConnectFd, c_end_buf_a, sizeof(c_end_buf_a));
        if (FILE_SERVER_ERROR == iSendBytes) {
            File_error("[%s]server_sendData is fail\n", __FUNCTION__);
            return FILEDATA_DEAL_RET_FAIL;   
        }   
    } else {
        /*end of else*/
    }

    return FILEDATA_DEAL_RET_OK;  
}

/************************************************************
* FUNCTION          :DataDeal_fileListDeal()
* Description       :Recursively describe the files in the 
*                    directory
* Arguments         :
* [pcFileName][IN] :Directory name
* [iConnectFd][IN]:File descriptor after connection establishment
* return            :success return FILEDATA_DEAL_RET_OK and 
*                    fail return FILEDATA_DEAL_RET_FAIL
************************************************************/
int DataDeal_fileListDeal(const char * pcFileName, int iConnectFd)
{
    DIR * p_mydir = NULL;
    struct dirent * stp_dir_item = NULL;
    int iRet = FILEDATA_DEAL_RET_OK;
    int iSendBytes = 0;
    struct stat stFileInfo;
    char cFileNameBuf[BUFSIZE];

    p_mydir = opendir(pcFileName);
    if (NULL == p_mydir) {
        perror("opendir"); 
        return FILEDATA_DEAL_RET_FAIL;
    } 

    while (NULL != (stp_dir_item = readdir(p_mydir))) {
        if ((strcmp(stp_dir_item->d_name, ".") == 0) || (strcmp(stp_dir_item->d_name, "..") == 0)) {
            continue;
        }

        memset(&stFileInfo, 0, sizeof(stFileInfo));
        sprintf(cFileNameBuf, "%s/%s", pcFileName, stp_dir_item->d_name);
        iRet = stat(cFileNameBuf, &stFileInfo);
        if (-1 == iRet) {
            perror("stat");
            return FILEDATA_DEAL_RET_FAIL; 
        }

        if (S_ISDIR(stFileInfo.st_mode)) {
            File_printf("[DIR]stp_dir_item->d_name = %s\n", cFileNameBuf);
            iRet = DataDeal_fileListDeal(cFileNameBuf, iConnectFd);
            if (FILEDATA_DEAL_RET_FAIL == iRet) {
                File_error("[%s]DataDeal_fileListDeal is error!\n", __FUNCTION__);
                return FILEDATA_DEAL_RET_FAIL;    
            }
        } else {
            iSendBytes = server_sendData(iConnectFd, cFileNameBuf, sizeof(cFileNameBuf));
            if (FILE_SERVER_ERROR == iSendBytes) {
                File_error("[%s]server_sendData is fail\n", __FUNCTION__);
                return FILEDATA_DEAL_RET_FAIL;   
            }
            File_printf("Client_sendData send filename uiData %d bytes\n", iSendBytes);    
        }
    }   
    closedir(p_mydir);
}

/************************************************************
* FUNCTION          :DataDeal_setFile()
* Description       :Process the "G" command sent by the client
* Arguments         :
* [iConnectFd][IN]:File descriptor after connection 
*                    establishment
* return            :success return FILEDATA_DEAL_RET_OK 
*                    and fail return FILEDATA_DEAL_RET_FAIL
************************************************************/
int DataDeal_getFile(int iConnectFd)
{
    int iRet = FILEDATA_DEAL_RET_OK;
    int iRecvDataBytes = 0;
    int iSendBytes = 0; 
    int iFileFd = 0;
    struct stat stFileInfo;
    ssize_t readBytes = 0;

    if (g_stTestHdr.m_enModule == MODULE_TEST_PROTO) {

        char cPackBuf[g_stTestHdr.m_uiDataLen + 1];
        char *pcPackBuf = NULL;
        FILEDATA stUnpackBuf;
        int iPackLen = 0;
        
        /*recv pack uiData from client*/
        memset(cPackBuf, 0, sizeof(cPackBuf));
        iRecvDataBytes = Server_recvData(iConnectFd, (char *)cPackBuf, g_stTestHdr.m_uiDataLen); 
        if (iRecvDataBytes == FILE_SERVER_ERROR) {
            File_error("[%s]Server_recvData is error, close server!!\n", __FUNCTION__);
            return FILEDATA_DEAL_RET_FAIL;
        } else if (iRecvDataBytes == FILE_SERVER_RECV_PEER_DOWN) {
            File_running("[%s]client close the connect!\n", __FUNCTION__);    
        } else {
            /*check having read file size */
            if (iRecvDataBytes != g_stTestHdr.m_uiDataLen) {
                File_running("file size is checking fail!\n");  
                return FILEDATA_DEAL_RET_FAIL; 
            }
            File_running("recv client uiData is success!\n");
            File_printf("Server_recvData recv protobuf pack uiData %d bytes\n", iRecvDataBytes);
        }

        File_printf("cPackBuf = %s\n", cPackBuf);
        File_printf("cPackBuf size = %d\n", sizeof(cPackBuf));
        for (int i = 0; i < g_stTestHdr.m_uiDataLen; i++) {
            File_printf("%X\n", cPackBuf[i]);
        }   
        
        /*unpack the pack uiData*/
        memset(&stUnpackBuf, 0, sizeof(stUnpackBuf));
        iRet = DataDeal_protoUnpack(cPackBuf, &stUnpackBuf, g_stTestHdr.m_uiDataLen);
        if (iRet == FILEDATA_DEAL_RET_FAIL) {
            File_error("[%s]DataDeal_protoUnpack is error!\n", __FUNCTION__);
            return FILEDATA_DEAL_RET_FAIL;      
        }

        /*judge the cmd uiData*/
        if (NULL != stUnpackBuf.m_pCmdBuf) {
            if (stUnpackBuf.m_pCmdBuf[0] != 'G') {
                File_printf("recv cmd Get, \"G\" cmd check fail!\n");
                return FILEDATA_DEAL_RET_FAIL;
            } else {
                File_printf("recv cmd Get, \"G\" cmd check success!\n");
            }
        } 

        /*open the file*/
        iFileFd = open(stUnpackBuf.m_pFileNameBuf, O_RDONLY);
        if (-1 == iFileFd) {
            perror("open");
            File_error("[%s]open is fail!\n", __FUNCTION__);
            return FILEDATA_DEAL_RET_FAIL; 
        }

        /*Get the size of file*/
        memset(&stFileInfo, 0, sizeof(stFileInfo));
        iRet = stat(stUnpackBuf.m_pFileNameBuf, &stFileInfo);
        if (-1 == iRet) {
            perror("stat");
            File_error("[%s]stat is fail!\n", __FUNCTION__);
            return FILEDATA_DEAL_RET_FAIL; 
        }
        stUnpackBuf.m_uiFileSize = stFileInfo.st_size;
        stUnpackBuf.has_m_uiFileSize = 1;
        if (NULL != stUnpackBuf.m_pCmdBuf) {
            free(stUnpackBuf.m_pCmdBuf);
            stUnpackBuf.m_pCmdBuf = NULL;
        }
        
        /*read the file to stUnpackBuf.m_pFileDataBuf*/
        stUnpackBuf.m_pFileDataBuf = (char *)malloc(stFileInfo.st_size);
        if (NULL == stUnpackBuf.m_pFileDataBuf) {
            perror("malloc");
            File_error("[%s]malloc is fail!\n", __FUNCTION__);
            return FILEDATA_DEAL_RET_FAIL; 
        }

        readBytes = DataDeal_fileRead(iFileFd, stUnpackBuf.m_pFileDataBuf, stFileInfo.st_size);
        if (-1 == readBytes) {
            File_error("[%s]DataDeal_fileRead is fail!\n", __FUNCTION__);
            return FILEDATA_DEAL_RET_FAIL;     
        }

        /*read file size check*/
        if (readBytes != stFileInfo.st_size) {
            File_running("file read is fail!\n");   
            return FILEDATA_DEAL_RET_FAIL; 
        }

        /*pack the file content uiData*/
        iPackLen = DataDeal_protoPack(&stUnpackBuf, &pcPackBuf);
        if (iPackLen == FILEDATA_DEAL_RET_FAIL) {
            File_error("[%s]DataDeal_protoPack is fail!\n", __FUNCTION__);
            return FILEDATA_DEAL_RET_FAIL;
        }
        
        if (NULL != stUnpackBuf.m_pFileDataBuf) {
            free(stUnpackBuf.m_pFileDataBuf); 
            stUnpackBuf.m_pFileDataBuf = NULL;
        }
        if (NULL != stUnpackBuf.m_pFileNameBuf) {
            free(stUnpackBuf.m_pFileNameBuf);    
            stUnpackBuf.m_pFileNameBuf = NULL;
        }

        /*pading the g_stTestHdr structure*/
        memset(&g_stTestHdr, 0, sizeof(g_stTestHdr));
        g_stTestHdr.m_uiDataLen = iPackLen;

        /*sned the g_stTestHdr*/
        iSendBytes = server_sendData(iConnectFd, (char *)&g_stTestHdr, sizeof(g_stTestHdr));
        if (iRet == FILE_SERVER_ERROR) {
            File_error("[%s]server_sendData is fail\n", __FUNCTION__);
            return FILEDATA_DEAL_RET_FAIL;
        }
        File_printf("server_sendData send hdr uiData %d bytes\n", iSendBytes);

        /*sned the pack uiData of file content response cmd G*/
        iSendBytes = server_sendData(iConnectFd, pcPackBuf, iPackLen);
        if (iSendBytes == FILE_SERVER_ERROR) {
            File_error("[%s]server_sendData is fail\n", __FUNCTION__);
            return FILEDATA_DEAL_RET_FAIL;
        }
        File_printf("server_sendData send protobuf pack uiData %d bytes\n", iSendBytes);
        File_printf("cPackBuf = %s\n", pcPackBuf);
        File_printf("cPackBuf size = %d\n", g_stTestHdr.m_uiDataLen);
        for (int i = 0; i < g_stTestHdr.m_uiDataLen; i++) {
            File_printf("%hhX\n", pcPackBuf[i]);
        }
        
        if (NULL != pcPackBuf) {
            free(pcPackBuf);
            pcPackBuf = NULL;
        }
        close(iFileFd);
    } else if (g_stTestHdr.m_enModule == MODULE_TEST_JSON) {
        
        cJSON * pstJson = NULL;  
        cJSON * stp_json_str_cmd = NULL; 
        cJSON * pstJsonStrFileName = NULL; 
        char c_cjson_out_data_a[g_stTestHdr.m_uiDataLen];

        /*recv json pack uiData from client*/
        memset(c_cjson_out_data_a, 0, sizeof(c_cjson_out_data_a));
        iRecvDataBytes = Server_recvData(iConnectFd, (char *)c_cjson_out_data_a, g_stTestHdr.m_uiDataLen); 
        if (iRecvDataBytes == FILE_SERVER_ERROR) {
            File_error("[%s]Server_recvData is error, close server!!\n", __FUNCTION__);
            return FILEDATA_DEAL_RET_FAIL;
        } else if (iRecvDataBytes == FILE_SERVER_RECV_PEER_DOWN) {
            File_running("[%s]client close the connect!\n", __FUNCTION__);    
        } else {
            /*check having read file size */
            if (iRecvDataBytes != g_stTestHdr.m_uiDataLen) {
                File_running("file size is checking fail!\n");  
                return FILEDATA_DEAL_RET_FAIL; 
            }
            File_running("recv client uiData is success!\n");
            File_printf("Server_recvData recv json pack uiData %d bytes\n", iRecvDataBytes);
        }

        File_printf("c_cjson_out_data_a = %s\n", c_cjson_out_data_a);
        File_printf("c_cjson_out_data_a size = %d\n", sizeof(c_cjson_out_data_a));
        for (int i = 0; i < g_stTestHdr.m_uiDataLen; i++) {
            File_printf("%X\n", c_cjson_out_data_a[i]);
        }   

        /*Parse the json uiData*/
        pstJson = cJSON_Parse(c_cjson_out_data_a);
        if (NULL == pstJson) {
            File_error("[%s]cJSON_Parse is error, close server!!\n", __FUNCTION__);  
             return FILEDATA_DEAL_RET_FAIL;  
        }

        /*get the item that the key name is string_cmd*/
        stp_json_str_cmd = cJSON_GetObjectItem(pstJson, "string_cmd");
        if (cJSON_IsString(stp_json_str_cmd)) {
            if (!strncmp(stp_json_str_cmd->string, "string_cmd", 10)) {
                File_running("[%s]json string_cmd check is success!\n", __FUNCTION__);    
            } else {
                File_running("[%s]json string_cmd check is fail!\n", __FUNCTION__); 
                return FILEDATA_DEAL_RET_FAIL; 
            } 
        }

        /*get the item that the key name is string_file_name*/
        pstJsonStrFileName = cJSON_GetObjectItem(pstJson, "string_file_name");
        if (cJSON_IsString(pstJsonStrFileName)) {
            if (!strncmp(pstJsonStrFileName->string, "string_file_name", 10)) {
                File_running("[%s]json string_file_name check is success!\n", __FUNCTION__);    
            } else {
                File_running("[%s]json string_file_name check is fail!\n", __FUNCTION__); 
                return FILEDATA_DEAL_RET_FAIL; 
            } 
        }

        /*judge the cmd uiData*/
        if (NULL != stp_json_str_cmd->valuestring) {
            if (strncmp(stp_json_str_cmd->valuestring, "G", 1)) {
                File_printf("recv cmd list, \"G\" cmd check fail!\n");
                return FILEDATA_DEAL_RET_FAIL;
            } else {
                File_printf("recv cmd list, \"G\" cmd check success!\n");
            }
        }

        cJSON *pstCjsonRoot = cJSON_CreateObject();
        char *pcCjsonDataOut = NULL;
        int iCjsonDataSize = 0;   
        char *pcFileData = NULL;

        /*open the file*/
        iFileFd = open(pstJsonStrFileName->valuestring, O_RDONLY);
        if (-1 == iFileFd) {
            perror("open");
            File_error("[%s]open is fail!\n", __FUNCTION__);
            return FILEDATA_DEAL_RET_FAIL; 
        }

        /*Get the size of file*/
        memset(&stFileInfo, 0, sizeof(stFileInfo));
        iRet = stat(pstJsonStrFileName->valuestring, &stFileInfo);
        if (-1 == iRet) {
            perror("stat");
            File_error("[%s]stat is fail!\n", __FUNCTION__);
            return FILEDATA_DEAL_RET_FAIL; 
        }
        
        /*read the file to stUnpackBuf.m_pFileDataBuf*/
        pcFileData = (char *)malloc(stFileInfo.st_size);
        if (NULL == pcFileData) {
            perror("malloc");
            File_error("[%s]malloc is fail!\n", __FUNCTION__);
            return FILEDATA_DEAL_RET_FAIL; 
        }

        readBytes = DataDeal_fileRead(iFileFd, pcFileData, stFileInfo.st_size);
        if (-1 == readBytes) {
            File_error("[%s]DataDeal_fileRead is fail!\n", __FUNCTION__);
            return FILEDATA_DEAL_RET_FAIL;     
        }

        /*read file size check*/
        if (readBytes != stFileInfo.st_size) {
            File_running("file read is fail!\n");   
            return FILEDATA_DEAL_RET_FAIL; 
        }

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
        iSendBytes = server_sendData(iConnectFd, (char *)&g_stTestHdr, sizeof(g_stTestHdr));
        if (iRet == FILE_SERVER_ERROR) {
            File_error("[%s]server_sendData is fail\n", __FUNCTION__);
            return FILEDATA_DEAL_RET_FAIL;
        }
        File_printf("Client_sendData send hdr uiData %d bytes\n", iSendBytes);

        /*sned the cjson pack uiData*/
        iSendBytes = server_sendData(iConnectFd, pcCjsonDataOut, iCjsonDataSize + 1);
        if (iSendBytes == FILE_SERVER_ERROR) {
            File_error("[%s]server_sendData is fail\n", __FUNCTION__);
            return FILEDATA_DEAL_RET_FAIL;
        }

        File_printf("server_sendData send cjson pack uiData %d bytes\n", iSendBytes);
        File_printf("pcCjsonDataOut = %s\n", pcCjsonDataOut);
        File_printf("pcCjsonDataOut size = %d\n", g_stTestHdr.m_uiDataLen);
        for (int i = 0; i < g_stTestHdr.m_uiDataLen; i++) {
            File_printf("%hhX\n", pcCjsonDataOut[i]);
        }

        if (NULL != pstCjsonRoot) {
            cJSON_Delete(pstCjsonRoot);
            pstCjsonRoot = NULL;
        }
        if (NULL != pcCjsonDataOut) {
            free(pcCjsonDataOut);
            pcCjsonDataOut = NULL;
        }
        if (NULL != pcFileData) {
            free(pcFileData);
            pcFileData = NULL;
        } 
        if (NULL != pstJson) {
            free(pstJson);
            pstJson = NULL;
        }  
    } else if (g_stTestHdr.m_enModule == MODULE_TEST_TLV) {
        char cPackBuf[BUFSIZE];
        FileData_S stFileData;
        int iRet = TLV_DECODE_RET_OK;
        unsigned int uiTlvTotolLen = 0;

        /*recv tlv pack uiData from client*/
        memset(cPackBuf, 0, sizeof(cPackBuf));
        iRecvDataBytes = Server_recvData(iConnectFd, (char *)cPackBuf, g_stTestHdr.m_uiDataLen); 
        if (iRecvDataBytes == FILE_SERVER_ERROR) {
            File_error("[%s]Server_recvData is error, close server!!\n", __FUNCTION__);
            return FILEDATA_DEAL_RET_FAIL;
        } else if (iRecvDataBytes == FILE_SERVER_RECV_PEER_DOWN) {
            File_running("[%s]client close the connect!\n", __FUNCTION__);    
        } else {
            /*check having read file size */
            if (iRecvDataBytes != g_stTestHdr.m_uiDataLen) {
                File_running("file size is checking fail!\n");  
                return FILEDATA_DEAL_RET_FAIL; 
            }
            File_running("recv client uiData is success!\n");
            File_printf("Server_recvData recv tlv pack uiData %d bytes\n", iRecvDataBytes);
        }

        File_printf("cPackBuf = %s\n", cPackBuf);
        File_printf("cPackBuf size = %d\n", g_stTestHdr.m_uiDataLen);
        for (int i = 0; i < g_stTestHdr.m_uiDataLen; i++) {
            File_printf("%X\n", cPackBuf[i]);
        } 

        /*decode the tlv pack uiData*/
        memset(&stFileData, 0, sizeof(stFileData));
        iRet = tlvDecodeFile(cPackBuf, g_stTestHdr.m_uiDataLen, &stFileData);
        if (TLV_DECODE_RET_FAIL == iRet) {
            File_error("[%s]tlvDecodeFile is fail!\n", __FUNCTION__);  
            return FILEDATA_DEAL_RET_FAIL;
        }

        /*judge the cmd uiData*/
        if (strncmp(stFileData.m_cFileCmd, "G", 1)) {
            File_printf("recv cmd set, \"G\" cmd check fail!\n");
            return FILEDATA_DEAL_RET_FAIL;
        } else {
            File_printf("recv cmd set, \"G\" cmd check success!\n");
        }

        /*open the file*/
        iFileFd = open(stFileData.m_cFileName, O_RDONLY);
        if (-1 == iFileFd) {
            perror("open");
            File_error("[%s]open is fail!\n", __FUNCTION__);
            return FILEDATA_DEAL_RET_FAIL; 
        }

        /*Get the size of file*/
        memset(&stFileInfo, 0, sizeof(stFileInfo));
        iRet = stat(stFileData.m_cFileName, &stFileInfo);
        if (-1 == iRet) {
            perror("stat");
            File_error("[%s]stat is fail!\n", __FUNCTION__);
            return FILEDATA_DEAL_RET_FAIL; 
        }

        /*read the file to stFileData.m_pcFileContent*/
        stFileData.m_pcFileContent = (char *)malloc(stFileInfo.st_size + 1);
        if (NULL == stFileData.m_pcFileContent) {
            perror("malloc");
            File_error("[%s]malloc is fail!\n", __FUNCTION__);
            return FILEDATA_DEAL_RET_FAIL; 
        }

        readBytes = DataDeal_fileRead(iFileFd, stFileData.m_pcFileContent, stFileInfo.st_size);
        if (-1 == readBytes) {
            File_error("[%s]DataDeal_fileRead is fail!\n", __FUNCTION__);
            return FILEDATA_DEAL_RET_FAIL;     
        }

        /*read file size check*/
        if (readBytes != stFileInfo.st_size) {
            File_running("file read is fail!\n");   
            return FILEDATA_DEAL_RET_FAIL; 
        }

        /*tlv encode the stFileData*/
        stFileData.m_uiFileSize = stFileInfo.st_size;
        stFileData.m_uiDataTotolSize = 12 + 12 + 4 + stFileData.m_uiFileSize; 
        memset(cPackBuf, 0, sizeof(cPackBuf));
        iRet = tlvEncodeFile(&stFileData, cPackBuf, &uiTlvTotolLen, BUFSIZE);
        if (TLV_ENCODE_RET_FAIL == iRet) {
            File_error("[%s]tlvEncodeFile is fail\n", __FUNCTION__); 
            return FILEDATA_DEAL_RET_FAIL;
        }
        
        /*pading the g_stTestHdr structure*/
        g_stTestHdr.m_enVersion = VERSION_ONE;
        g_stTestHdr.m_uiDataLen = uiTlvTotolLen;
        g_stTestHdr.m_usHdrLen = sizeof(g_stTestHdr);

        /*sned the g_stTestHdr*/
        iSendBytes = server_sendData(iConnectFd, (char *)&g_stTestHdr, sizeof(g_stTestHdr));
        if (iRet == FILE_SERVER_ERROR) {
            File_error("[%s]server_sendData is fail\n", __FUNCTION__);
            return FILEDATA_DEAL_RET_FAIL;
        }
        File_printf("Client_sendData send hdr uiData %d bytes\n", iSendBytes);

        /*sned the tlv pack uiData*/
        iSendBytes = server_sendData(iConnectFd, cPackBuf, uiTlvTotolLen);
        if (iSendBytes == FILE_SERVER_ERROR) {
            File_error("[%s]server_sendData is fail\n", __FUNCTION__);
            return FILEDATA_DEAL_RET_FAIL;
        }

        File_printf("server_sendData send tlv pack uiData %d bytes\n", iSendBytes);
        File_printf("cPackBuf = %s\n", cPackBuf);
        File_printf("cPackBuf size = %d\n", g_stTestHdr.m_uiDataLen);
        for (int i = 0; i < g_stTestHdr.m_uiDataLen; i++) {
            File_printf("%hhX\n", cPackBuf[i]);
        }

        if (NULL != stFileData.m_pcFileContent) {
            free(stFileData.m_pcFileContent);
            stFileData.m_pcFileContent = NULL;
        }
        close(iFileFd);
    } else {
        /*end of else*/
    }

    return FILEDATA_DEAL_RET_OK;  
}

/************************************************************
* FUNCTION          :DataDeal_setFile()
* Description       :Process the "S" command sent by the client
* Arguments         :
* [iConnectFd][IN]:File descriptor after connection 
*                    establishment
* return            :success return FILEDATA_DEAL_RET_OK 
*                    and fail return FILEDATA_DEAL_RET_FAIL
************************************************************/
int DataDeal_setFile(int iConnectFd)
{
    int iRet = FILEDATA_DEAL_RET_OK;
    int iRecvDataBytes = 0;
    int iSendBytes = 0;
    int iFileFd = 0;
    ssize_t writeBytes = 0;
    
    if (g_stTestHdr.m_enModule == MODULE_TEST_PROTO) {

        char cPackBuf[g_stTestHdr.m_uiDataLen + 1];
        FILEDATA stUnpackBuf;
    
        /*recv pack uiData from client*/
        memset(cPackBuf, 0, sizeof(cPackBuf));
        iRecvDataBytes = Server_recvData(iConnectFd, (char *)cPackBuf, g_stTestHdr.m_uiDataLen); 
        if (iRecvDataBytes == FILE_SERVER_ERROR) {
            File_error("[%s]Server_recvData is error, close server!!\n", __FUNCTION__);
            return FILEDATA_DEAL_RET_FAIL;
        } else if (iRecvDataBytes == FILE_SERVER_RECV_PEER_DOWN) {
            File_running("[%s]client close the connect!\n", __FUNCTION__);    
        } else {
            /*check having read file size */
            if (iRecvDataBytes != g_stTestHdr.m_uiDataLen) {
                File_running("file size is checking fail!\n");  
                return FILEDATA_DEAL_RET_FAIL; 
            }
            File_running("recv client uiData is success!\n");
            File_printf("Server_recvData recv protobuf pack uiData %d bytes\n", iRecvDataBytes);
        }

        File_printf("cPackBuf = %s\n", cPackBuf);
        File_printf("cPackBuf size = %d\n", sizeof(cPackBuf));
        for (int i = 0; i < g_stTestHdr.m_uiDataLen; i++) {
            File_printf("%X\n", cPackBuf[i]);
        }   
        
        /*unpack the pack uiData*/
        memset(&stUnpackBuf, 0, sizeof(stUnpackBuf));
        iRet = DataDeal_protoUnpack(cPackBuf, &stUnpackBuf, g_stTestHdr.m_uiDataLen);
        if (iRet == FILEDATA_DEAL_RET_FAIL) {
            File_error("[%s]DataDeal_protoUnpack is error!\n", __FUNCTION__);
            return FILEDATA_DEAL_RET_FAIL;      
        }

        /*check the cmd uiData*/
        if (NULL != stUnpackBuf.m_pCmdBuf) {
            if (stUnpackBuf.m_pCmdBuf[0] != 'S') {
                File_printf("recv set file ,  \"S\" cmd check fail!\n");
                return FILEDATA_DEAL_RET_FAIL;
            } else {
                File_printf("recv set file ,  \"S\" cmd check success!\n");
            }
        }

        while (1) {
            /*open the file*/
            iFileFd = open(stUnpackBuf.m_pFileNameBuf, O_WRONLY|O_CREAT|O_EXCL);
            if (-1 == iFileFd) {
                if (errno == EEXIST) {
                    iRet = remove(stUnpackBuf.m_pFileNameBuf);
                    if (-1 == iRet) {
                        perror("remove");
                        File_error("[%s]remove is error!\n", __FUNCTION__);
                        return 0;
                    }
                    continue;
                } else {
                    perror("open");
                    File_error("[%s]open is fail!\n", __FUNCTION__);
                    return FILEDATA_DEAL_RET_FAIL;     
                }
            } else {
                break;
            }   
        }


        /*write uiData to filename*/
        if (1 == stUnpackBuf.has_m_uiFileSize) {
            writeBytes = DataDeal_fileWrite(iFileFd, stUnpackBuf.m_pFileDataBuf, stUnpackBuf.m_uiFileSize);
            if (-1 == writeBytes) {
                File_error("[%s]DataDeal_fileWrite is fail!\n", __FUNCTION__);
                return FILEDATA_DEAL_RET_FAIL;     
            }   
        } else {
            writeBytes = DataDeal_fileWrite(iFileFd, stUnpackBuf.m_pFileDataBuf, BUFSIZE);
            if (-1 == writeBytes) {
                File_error("[%s]DataDeal_fileWrite is fail!\n", __FUNCTION__);
                return FILEDATA_DEAL_RET_FAIL;     
            }       
        }
        File_running("file write is success!\n");

        if (NULL != stUnpackBuf.m_pCmdBuf) {
            free(stUnpackBuf.m_pCmdBuf);
        }
        if (NULL != stUnpackBuf.m_pFileNameBuf) {
            free(stUnpackBuf.m_pFileNameBuf);    
        }
        if (NULL != stUnpackBuf.m_pFileDataBuf) {
            free(stUnpackBuf.m_pFileDataBuf);    
        }
        close(iFileFd);
    } else if (g_stTestHdr.m_enModule == MODULE_TEST_JSON) {

        cJSON * pstJson = NULL;  
        cJSON * stp_json_str_cmd = NULL; 
        cJSON * pstJsonStrFileName = NULL; 
        cJSON * pstJsonStrFileData = NULL; 
        cJSON * pstJsonNumFileSize = NULL; 
        char c_cjson_out_data_a[g_stTestHdr.m_uiDataLen];  

        /*recv json pack uiData from client*/
        memset(c_cjson_out_data_a, 0, sizeof(c_cjson_out_data_a));
        iRecvDataBytes = Server_recvData(iConnectFd, (char *)c_cjson_out_data_a, g_stTestHdr.m_uiDataLen); 
        if (iRecvDataBytes == FILE_SERVER_ERROR) {
            File_error("[%s]Server_recvData is error, close server!!\n", __FUNCTION__);
            return FILEDATA_DEAL_RET_FAIL;
        } else if (iRecvDataBytes == FILE_SERVER_RECV_PEER_DOWN) {
            File_running("[%s]client close the connect!\n", __FUNCTION__);    
        } else {
            /*check having read file size */
            if (iRecvDataBytes != g_stTestHdr.m_uiDataLen) {
                File_running("file size is checking fail!\n");  
                return FILEDATA_DEAL_RET_FAIL; 
            }
            File_running("recv client uiData is success!\n");
            File_printf("Server_recvData recv cjson pack uiData %d bytes\n", iRecvDataBytes);
        }

        File_printf("c_cjson_out_data_a = %s\n", c_cjson_out_data_a);
        File_printf("c_cjson_out_data_a size = %d\n", sizeof(c_cjson_out_data_a));
        for (int i = 0; i < g_stTestHdr.m_uiDataLen; i++) {
            File_printf("%X\n", c_cjson_out_data_a[i]);
        } 

        /*Parse the json uiData*/
        pstJson = cJSON_Parse(c_cjson_out_data_a);
        if (NULL == pstJson) {
            File_error("[%s]cJSON_Parse is error, close server!!\n", __FUNCTION__);  
             return FILEDATA_DEAL_RET_FAIL;  
        }

        /*get the item that the key name is string_cmd*/
        stp_json_str_cmd = cJSON_GetObjectItem(pstJson, "string_cmd");
        if (cJSON_IsString(stp_json_str_cmd)) {
            if (!strncmp(stp_json_str_cmd->string, "string_cmd", 10)) {
                File_running("[%s]json string_cmd check is success!\n", __FUNCTION__);    
            } else {
                File_running("[%s]json string_cmd check is fail!\n", __FUNCTION__); 
                return FILEDATA_DEAL_RET_FAIL; 
            } 
        }

        /*get the item that the key name is string_file_name*/
        pstJsonStrFileName = cJSON_GetObjectItem(pstJson, "string_file_name");
        if (cJSON_IsString(pstJsonStrFileName)) {
            if (!strncmp(pstJsonStrFileName->string, "string_file_name", 10)) {
                File_running("[%s]json string_file_name check is success!\n", __FUNCTION__);    
            } else {
                File_running("[%s]json string_file_name check is fail!\n", __FUNCTION__); 
                return FILEDATA_DEAL_RET_FAIL; 
            } 
        }

        /*get the item that the key name is string_file_data*/
        pstJsonStrFileData = cJSON_GetObjectItem(pstJson, "string_file_data");
        if (cJSON_IsString(pstJsonStrFileData)) {
            if (!strncmp(pstJsonStrFileData->string, "string_file_data", 10)) {
                File_running("[%s]json string_file_data check is success!\n", __FUNCTION__);    
            } else {
                File_running("[%s]json string_file_data check is fail!\n", __FUNCTION__); 
                return FILEDATA_DEAL_RET_FAIL; 
            } 
        }

        /*get the item that the key name is number_file_size*/
        pstJsonNumFileSize = cJSON_GetObjectItem(pstJson, "number_file_size");
        if (cJSON_IsNumber(pstJsonNumFileSize)) {
            if (!strncmp(pstJsonNumFileSize->string, "number_file_size", 10)) {
                File_running("[%s]json number_file_size check is success!\n", __FUNCTION__);    
            } else {
                File_running("[%s]json number_file_size check is fail!\n", __FUNCTION__); 
                return FILEDATA_DEAL_RET_FAIL; 
            } 
        }

        /*judge the cmd uiData*/
        if (NULL != stp_json_str_cmd->valuestring) {
            if (strncmp(stp_json_str_cmd->valuestring, "S", 1)) {
                File_printf("recv cmd list, \"S\" cmd check fail!\n");
                return FILEDATA_DEAL_RET_FAIL;
            } else {
                File_printf("recv cmd list, \"S\" cmd check success!\n");
            }
        } 

        while (1) {
            /*open the file*/
            iFileFd = open(pstJsonStrFileName->valuestring, O_WRONLY|O_CREAT|O_EXCL);
            if (-1 == iFileFd) {
                if (errno == EEXIST) {
                    iRet = remove(pstJsonStrFileName->valuestring);
                    if (-1 == iRet) {
                        perror("remove");
                        File_error("[%s]remove is error!\n", __FUNCTION__);
                        return 0;
                    }
                    continue;
                } else {
                    perror("open");
                    File_error("[%s]open is fail!\n", __FUNCTION__);
                    return FILEDATA_DEAL_RET_FAIL;     
                }
            } else {
                break;
            }   
        }

        /*write uiData to filename*/
        writeBytes = DataDeal_fileWrite(iFileFd, pstJsonStrFileData->valuestring, pstJsonNumFileSize->valueint);
        if (-1 == writeBytes) {
            File_error("[%s]DataDeal_fileWrite is fail!\n", __FUNCTION__);
            return FILEDATA_DEAL_RET_FAIL;     
        }
        File_running("file write is success!\n");

        cJSON_Delete(pstJson); 
        close(iFileFd);
            
    } else if (g_stTestHdr.m_enModule == MODULE_TEST_TLV) {
        char cPackBuf[BUFSIZE];
        FileData_S stFileData;
        int iRet = TLV_DECODE_RET_OK;

        /*recv tlv pack uiData from client*/
        memset(cPackBuf, 0, sizeof(cPackBuf));
        iRecvDataBytes = Server_recvData(iConnectFd, (char *)cPackBuf, g_stTestHdr.m_uiDataLen); 
        if (iRecvDataBytes == FILE_SERVER_ERROR) {
            File_error("[%s]Server_recvData is error, close server!!\n", __FUNCTION__);
            return FILEDATA_DEAL_RET_FAIL;
        } else if (iRecvDataBytes == FILE_SERVER_RECV_PEER_DOWN) {
            File_running("[%s]client close the connect!\n", __FUNCTION__);    
        } else {
            /*check having read file size */
            if (iRecvDataBytes != g_stTestHdr.m_uiDataLen) {
                File_running("file size is checking fail!\n");  
                return FILEDATA_DEAL_RET_FAIL; 
            }
            File_running("recv client uiData is success!\n");
            File_printf("Server_recvData recv tlv pack uiData %d bytes\n", iRecvDataBytes);
        }

        File_printf("cPackBuf = %s\n", cPackBuf);
        File_printf("cPackBuf size = %d\n", g_stTestHdr.m_uiDataLen);
        for (int i = 0; i < g_stTestHdr.m_uiDataLen; i++) {
            File_printf("%X\n", cPackBuf[i]);
        } 

        /*tlv decode the cmd of pack uiData*/
        memset(&stFileData, 0, sizeof(stFileData));
        stFileData.m_pcFileContent = (char *)malloc(g_stTestHdr.m_uiDataLen);
        if (NULL == stFileData.m_pcFileContent) {
            perror("malloc");
            File_error("[%s]malloc is fail!\n", __FUNCTION__);
            return FILEDATA_DEAL_RET_FAIL; 
        }

        iRet = tlvDecodeFile(cPackBuf, g_stTestHdr.m_uiDataLen, &stFileData);
        if (TLV_DECODE_RET_FAIL == iRet) {
            File_error("[%s]tlvDecodeFile is fail!\n", __FUNCTION__);  
            return FILEDATA_DEAL_RET_FAIL;
        }

        /*judge the cmd uiData*/
        if (strncmp(stFileData.m_cFileCmd, "S", 1)) {
            File_printf("recv cmd set, \"S\" cmd check fail!\n");
            return FILEDATA_DEAL_RET_FAIL;
        } else {
            File_printf("recv cmd set, \"S\" cmd check success!\n");
        }

        while (1) {
            /*open the file*/
            iFileFd = open(stFileData.m_cFileName, O_WRONLY|O_CREAT|O_EXCL);
            if (-1 == iFileFd) {
                if (errno == EEXIST) {
                    iRet = remove(stFileData.m_cFileName);
                    if (-1 == iRet) {
                        perror("remove");
                        File_error("[%s]remove is error!\n", __FUNCTION__);
                        return 0;
                    }
                    continue;
                } else {
                    perror("open");
                    File_error("[%s]open is fail!\n", __FUNCTION__);
                    return FILEDATA_DEAL_RET_FAIL;     
                }
            } else {
                break;
            }   
        }

        /*write uiData to filename*/
        writeBytes = DataDeal_fileWrite(iFileFd, stFileData.m_pcFileContent, stFileData.m_uiFileSize);
        if (-1 == writeBytes) {
            File_error("[%s]DataDeal_fileWrite is fail!\n", __FUNCTION__);
            return FILEDATA_DEAL_RET_FAIL;     
        }   
        File_running("file write is success!\n");

        if (NULL != stFileData.m_pcFileContent) {
            free(stFileData.m_pcFileContent);
            stFileData.m_pcFileContent = NULL;
        }
        close(iFileFd);
        
    } else {
        /*end of else*/
    }

    return FILEDATA_DEAL_RET_OK;                  
}
