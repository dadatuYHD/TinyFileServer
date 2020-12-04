#include <stdio.h>   
#include <string.h>  
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <assert.h>
#include "FileServerTlv.h"
#include "FileServerDebug.h"

int writeBlock(char** pDst, unsigned int uiWriteLen, void* pSrc, char*pcEndData)
{
    memcpy(*pDst, pSrc, uiWriteLen);
    *pDst += uiWriteLen;

    return (*pDst <= pcEndData) ? TLV_ENCODE_RET_OK: TLV_ENCODE_RET_FAIL;
}
int writeInt(unsigned int uiData, char** pDst, char* pcEndData)
{
    unsigned int i_net_data = htonl(uiData);
    
    int iRet = TLV_ENCODE_RET_OK;
    
    iRet = writeBlock(pDst, sizeof(int), &i_net_data, pcEndData);
    if (TLV_ENCODE_RET_FAIL == iRet) {
        File_error("[%s]writeBlock over lap!\n", __FUNCTION__);
        return TLV_ENCODE_RET_FAIL;
    }

    return TLV_ENCODE_RET_OK;
}

int readBlock(void* pDst, char** pSrc, unsigned int uiLen, char* pcEndData)
{
    memcpy(pDst, *pSrc, uiLen);
    *pSrc += uiLen;

    return (*pSrc <= pcEndData) ? TLV_DECODE_RET_OK : TLV_DECODE_RET_FAIL;
}
int readInt(unsigned     int* uiData, char** pSrc, char* pcEndData)
{   
    int iRet = TLV_DECODE_RET_OK;

    iRet = readBlock(uiData, pSrc, sizeof(int), pcEndData);
    if (TLV_DECODE_RET_FAIL == iRet) {
        File_error("[%s]readBlock over lap!\n", __FUNCTION__);
        return TLV_DECODE_RET_FAIL;
    }

    *uiData = ntohl(*uiData);

    return TLV_DECODE_RET_OK;
}


/************************************************************
* FUNCTION                :tlvEncodeFile()
* Description             :use tlv pack uiData
* Arguments               :
* [pstFileData][IN]     :Point to uiData to be packaged
* [pcBuf][OUT]           :Storage of packaged uiData  
* [puiTlvTotolLen][OUT]:the size of being packaged uiData  
* [uiBufLen][IN]        ：the size of pcBuf
* return                  :success return TLV_ENCODE_RET_OK, 
*                          fail return TLV_ENCODE_RET_FAIL
************************************************************/
int tlvEncodeFile(FileData_Sp pstFileData, char* pcBuf, unsigned int* puiTlvTotolLen, unsigned int uiBufLen)
{
    assert(NULL != pstFileData);
    assert(NULL != pcBuf);
    assert(NULL != puiTlvTotolLen);

    char* pcWriteData = pcBuf;
    char* pcEndData = pcBuf + uiBufLen;

    /*write the root node*/
    writeInt(TLV_FILE_ROOT, &pcWriteData, pcEndData);
    writeInt(32 + pstFileData->m_uiDataTotolSize, &pcWriteData, pcEndData);

    /*construct the cmd node*/
    writeInt(TLV_FILE_CMD, &pcWriteData, pcEndData);
    writeInt(12, &pcWriteData, pcEndData);
    writeBlock(&pcWriteData, sizeof(pstFileData->m_cFileCmd), pstFileData->m_cFileCmd, pcEndData);

    /*construct the file name node*/
    writeInt(TLV_FILE_NAME, &pcWriteData, pcEndData);
    writeInt(12, &pcWriteData, pcEndData);
    writeBlock(&pcWriteData, sizeof(pstFileData->m_cFileName), pstFileData->m_cFileName, pcEndData);

    /*construct the file content node*/
    writeInt(TLV_FILE_CONTENT, &pcWriteData, pcEndData);
    writeInt(pstFileData->m_uiFileSize, &pcWriteData, pcEndData);
    writeBlock(&pcWriteData, pstFileData->m_uiFileSize, pstFileData->m_pcFileContent, pcEndData);

    /*construct the file size node*/
    writeInt(TLV_FILE_SIZE, &pcWriteData, pcEndData);
    writeInt(4, &pcWriteData, pcEndData);
    writeInt(pstFileData->m_uiFileSize, &pcWriteData, pcEndData);    

    *puiTlvTotolLen = 32 + pstFileData->m_uiDataTotolSize + 8;

    return TLV_ENCODE_RET_OK;
}


/************************************************************
* FUNCTION               :tlvDecodeFile()
* Description            :unpack the tlv uiData
* Arguments:
* [pstFileData][OUT]   ：Storage of unpackaged uiData 
* [pcBuf][IN]:Storage of to be unpackaged uiData  
* [puiTlvTotolLen][IN]:the size of being packaged uiData  
* return                 :success return TLV_DECODE_RET_OK, 
*                         fail return TLV_DECODE_RET_FAIL
************************************************************/
int tlvDecodeFile(char* pcBuf, unsigned int uiTlvTotolLen, FileData_Sp pstFileData)
{
    assert(NULL != pstFileData);
    assert(NULL != pcBuf);

    char* pcReadData = pcBuf;
    char* pcEndData = pcBuf + uiTlvTotolLen;
    FILE_TEST_EN enTlvType = TLV_FILE_NONE;
    unsigned int uiTlvLenSum = 0;
    unsigned int uiTlvLen = 0;

    readInt(&enTlvType, &pcReadData, pcEndData);
    if (TLV_FILE_ROOT != enTlvType) {
        File_error("[%s]read TLV_FILE_ROOT is fail!\n", __FUNCTION__);
        return TLV_DECODE_RET_FAIL;
    }
    
    readInt(&uiTlvLenSum, &pcReadData, pcEndData);

    while (uiTlvLenSum > 0) {
        readInt(&enTlvType, &pcReadData, pcEndData);
        readInt(&uiTlvLen, &pcReadData, pcEndData);
        switch (enTlvType) {
            case TLV_FILE_CMD:          
                readBlock(pstFileData->m_cFileCmd, &pcReadData,  uiTlvLen, pcEndData);
                uiTlvLenSum = uiTlvLenSum - (8 + uiTlvLen);
                break;
            case TLV_FILE_NAME:
                readBlock(pstFileData->m_cFileName, &pcReadData, uiTlvLen, pcEndData);
                uiTlvLenSum = uiTlvLenSum - (8 + uiTlvLen);
                break;
            case TLV_FILE_CONTENT:
                readBlock(pstFileData->m_pcFileContent, &pcReadData, uiTlvLen, pcEndData);
                uiTlvLenSum = uiTlvLenSum - (8 + uiTlvLen);
                break;
            case TLV_FILE_SIZE:
                readInt(&pstFileData->m_uiFileSize, &pcReadData, pcEndData);
                uiTlvLenSum = uiTlvLenSum - (8 + uiTlvLen);
                break;
            default:
                File_error("[%s]tlv decode is fail!\n", __FUNCTION__);
                break;      
        }
    }

    return TLV_DECODE_RET_OK;   
}

