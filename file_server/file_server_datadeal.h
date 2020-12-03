#ifndef _FILE_SERVER_DATADEAL_H_
#define _FILE_SERVER_DATADEAL_H_

#define FILEDATA_DEAL_RET_OK    0
#define FILEDATA_DEAL_RET_FAIL -1


/*Head version*/
typedef enum {
    VERSION_ONE = 1,
	VERSION_TWO,
	VERSION_THIRD,
	VERSION_MAX,
}VERSION_E;

/**********************************
 * Describe which field structure 
 * of the header is processed
 *********************************/
typedef enum {
    HDR_FIELD_VERSION = 1,
	HDR_FIELD_HDR_LEN,
	HDR_FIELD_DATA_LEN,
	HDR_FIELD_MODULE,
	HDR_FIELD_CMD,
}HDR_FIELD_FLG ;


/*Header uiData exchange format*/
typedef enum  {
	MODULE_TEST_PROTO = 0,
	MODULE_TEST_TLV,
	MODULE_TEST_JSON,
	MODULE_MAX,
}MODULE_E;

/*Head command definition*/
typedef enum {
    CMD_TEST_SET = 0,
    CMD_TEST_GET,
    CMD_TEST_LIST,
    CMD_TEST_CLIENT_EXIT,
    CMD_MAX,
}CMD_E;

/*Data header declaration*/
typedef struct TestHdr_S {
	VERSION_E      m_enVersion;     // The header version is tentatively set to one, 
	                               // and needs to be verified after receipt
	unsigned short m_usHdrLen;     // Header length needs to be verified after receipt
	unsigned int   m_uiDataLen;	   // Load uiData length
	MODULE_E       m_enModule;	   // Current business module
	CMD_E          m_enCmd;	       // Command number under the module
}TestHdr_S;

/*Head processing function declaration*/
TestHdr_S DataDeal_getHdr(void);
int DataDeal_setHdr(TestHdr_S* pstTestHdr, HDR_FIELD_FLG enFlg);
int DataDeal_getFile(int iConnectFd);
int DataDeal_setFile(int iConnectFd);
int datadeal_file_list(int iConnectFd);
TestHdr_S* DataDeal_getHdrAddress(void);

#endif
