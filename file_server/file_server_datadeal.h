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


/*Header data exchange format*/
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
typedef struct test_hdr_s {
	VERSION_E      en_version;     // The header version is tentatively set to one, 
	                               // and needs to be verified after receipt
	unsigned short us_hdr_len;     // Header length needs to be verified after receipt
	unsigned int   ui_dat_len;	   // Load data length
	MODULE_E       en_module;	   // Current business module
	CMD_E          en_cmd;	       // Command number under the module
}TEST_HDR_T;

/*Head processing function declaration*/
TEST_HDR_T datadeal_get_hdr(void);
int datadeal_set_hdr(TEST_HDR_T *pst_test_hdr, HDR_FIELD_FLG en_flg);
int datadeal_file_get(int i_connect_fd);
int datadeal_file_set(int i_connect_fd);
int datadeal_file_list(int i_connect_fd);
TEST_HDR_T * datadeal_get_phdr(void);

#endif
