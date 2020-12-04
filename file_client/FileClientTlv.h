#ifndef _FILE_CLIENT_TLV_H_
#define _FILE_CLIENT_TLV_H_


#define TLV_ENCODE_RET_OK    0
#define TLV_ENCODE_RET_FAIL -1
#define TLV_DECODE_RET_OK    1
#define TLV_DECODE_RET_FAIL -2

typedef enum {  
    TLV_FILE_NONE = 0,  
    TLV_FILE_ROOT,         //root node
    TLV_FILE_CMD,          //file server control cmd
    TLV_FILE_NAME,         //file name
    TLV_FILE_CONTENT,      //file content uiData
    TLV_FILE_SIZE,         //file size
}FILE_TEST_EN;

typedef struct FileData_S {
	char   m_cFileCmd[12];
    char   m_cFileName[12];
	char*  m_pcFileContent;
	unsigned int m_uiFileSize;
	unsigned int m_uiDataTotolSize;
}FileData_S, *FileData_Sp;

int tlvEncodeFile(FileData_Sp pstFileData, char* pcBuf, unsigned int* puiTlvTotolLen, unsigned int uiBufLen);
int tlvDecodeFile(char* pcBuf, unsigned int uiTlvTotolLen,FileData_Sp pstFileData);

#endif

