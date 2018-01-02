#ifndef _FILE_SERVER_DATADEAL_H_
#define _FILE_SERVER_DATADEAL_H_

#define FILEDATA_DEAL_RET_OK    0
#define FILEDATA_DEAL_RET_FAIL -1


/*头部版本号*/
typedef enum {
    VERSION_ONE = 1,
	VERSION_TWO,
	VERSION_THIRD,
	VERSION_MAX,
}VERSION_E;

/*描述处理头部哪一个字段结构*/
typedef enum {
    HDR_FIELD_VERSION = 1,
	HDR_FIELD_HDR_LEN,
	HDR_FIELD_DATA_LEN,
	HDR_FIELD_MODULE,
	HDR_FIELD_CMD,
}HDR_FIELD_FLG ;


/*头部数据交换格式*/
typedef enum  {
	MODULE_TEST_PROTO = 0,
	MODULE_TEST_TLV,
	MODULE_TEST_JSON,
	MODULE_MAX,
}MODULE_E;

/*头部命令定义*/
typedef enum {
    CMD_TEST_SET = 0,
    CMD_TEST_GET,
    CMD_TEST_LIST,
    CMD_MAX,
}CMD_E;

/*数据头部声明*/
typedef struct test_hdr_s {
	VERSION_E      en_version;     //头部版本暂定为1  收到后需校验
	unsigned short us_hdr_len;     //头部长度	收到后需校验
	unsigned int   ui_dat_len;	   //负载数据长度
	MODULE_E       en_module;	   //当前业务模块
	CMD_E          en_cmd;	       //模块下的命令号
}TEST_HDR_T;

/*头部处理函数声名*/
TEST_HDR_T datadeal_get_hdr(void);
int datadeal_set_hdr(TEST_HDR_T *pst_test_hdr, HDR_FIELD_FLG en_flg);
int datadeal_file_get(int i_connect_fd);
int datadeal_file_set(int i_connect_fd);
int datadeal_file_list(int i_connect_fd);
int datadeal_proto_unpack(char *cp_pack_buf, char **cp_unpack_buf);
TEST_HDR_T ** datadeal_get_phdr(void);








#endif
