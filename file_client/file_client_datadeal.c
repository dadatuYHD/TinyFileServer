#include <assert.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include "file_client_datadeal.h"
#include "file_client_debug.h"
#include "Message.pb-c.h"
#include "file_client.h"
#include "file_client_state.h"


#define BUFSIZE 1024

TEST_HDR_T g_st_test_hdr;    //全局数据头部

int datadeal_proto_pack(char *cp_unpack_buf, char **cp_pack_buf);



/************************************************************
FUNCTION:datadeal_get_hdr()
Description:该函数主要用来文件间传递g_st_test_hdr全局变量
Arguments:无
return:返回g_st_test_hdr
************************************************************/
TEST_HDR_T datadeal_get_hdr(void)
{
    return g_st_test_hdr;
}


/************************************************************
FUNCTION:datadeal_set_hdr()
Description:该函数主要用来文件间设置全局变量g_st_test_hdr
Arguments:
[pst_test_hdr][IN]：指向一个TEST_HDR_T结构，存放有待设置的数据
[en_flg][IN]：设置标志位
return:返回g_st_test_hdr
************************************************************/
int datadeal_set_hdr(TEST_HDR_T *pst_test_hdr, HDR_FIELD_FLG en_flg)
{
    assert(NULL != pst_test_hdr);

    if (en_flg == HDR_FIELD_VERSION) {
        g_st_test_hdr.en_version = pst_test_hdr->en_version;   
	} else if (en_flg == HDR_FIELD_HDR_LEN) {
        g_st_test_hdr.us_hdr_len = pst_test_hdr->us_hdr_len;   
	} else if (en_flg == HDR_FIELD_DATA_LEN) {
        g_st_test_hdr.ui_dat_len = pst_test_hdr->ui_dat_len;   
	} else if (en_flg == HDR_FIELD_MODULE) {
        g_st_test_hdr.en_module = pst_test_hdr->en_module;   
	} else if (en_flg == HDR_FIELD_CMD) {
        g_st_test_hdr.en_cmd = pst_test_hdr->en_cmd;   
	} else {
	    file_error("[%s]datadeal_set_hdr input flag is error!\n", __FUNCTION__);
        return FILEDATA_DEAL_RET_FAIL;   
	}

	return FILEDATA_DEAL_RET_OK;
}

int datadeal_file_set(int i_connect_fd)
{
    if (g_st_test_hdr.en_module == MODULE_TEST_PROTO) {

	}
	if (g_st_test_hdr.en_module == MODULE_TEST_JSON) {

	}
	if (g_st_test_hdr.en_module == MODULE_TEST_TLV) {

	}    
}

/************************************************************
FUNCTION:datadeal_file_get()
Description:该函数主要用来获取服务器保存的文件
Arguments:
[i_connect_fd][IN]：已经建立好连接的文件描述符
return:返回g_st_test_hdr
************************************************************/
int datadeal_file_get(int i_connect_fd)
{
    char c_unpack_buf_a[BUFSIZE];
	char *c_pack_buf_a = NULL;
	int i_pack_len = 0;
	int i_ret = FILE_CLIENT_OK;
	int i_send_bytes = 0;
	char c_filename_buf_a[BUFSIZE];

    if (g_st_test_hdr.en_module == MODULE_TEST_PROTO) {
        memset(c_unpack_buf_a, 0, sizeof(c_unpack_buf_a));
        strncpy(c_unpack_buf_a, "G", 1);
		c_unpack_buf_a[1] = '\0';

		
	}
	if (g_st_test_hdr.en_module == MODULE_TEST_JSON) {

	}
	if (g_st_test_hdr.en_module == MODULE_TEST_TLV) {

	}    
}

/************************************************************
FUNCTION:datadeal_file_list()
Description:该函数主要用来获取服务器保存的文件名字
Arguments:
[i_connect_fd][IN]：已经建立好连接的文件描述符
return:返回g_st_test_hdr
************************************************************/
int datadeal_file_list(int i_connect_fd)
{
#if 0
    char c_unpack_buf_a[BUFSIZE];
	char *c_pack_buf_a = NULL;
	int i_pack_len = 0;
#endif
	int i_ret = FILE_CLIENT_OK;
	int i_send_bytes = 0;
	char c_filename_buf_a[BUFSIZE];

    if (g_st_test_hdr.en_module == MODULE_TEST_PROTO) {
#if 0
        memset(c_unpack_buf_a, 0, sizeof(c_unpack_buf_a));
        strncpy(c_unpack_buf_a, "L", 1);
		c_unpack_buf_a[1] = '\0';
		
		i_pack_len = datadeal_proto_pack(c_unpack_buf_a, &c_pack_buf_a);
		if (i_pack_len == FILEDATA_DEAL_RET_FAIL) {
            file_error("[%s]datadeal_proto_pack is fail!\n", __FUNCTION__);
			return FILEDATA_DEAL_RET_FAIL;
		}
#endif
        /*pading the g_st_test_hdr structure*/
		g_st_test_hdr.en_version = VERSION_ONE;
		g_st_test_hdr.ui_dat_len = 0;
		g_st_test_hdr.us_hdr_len = sizeof(g_st_test_hdr);

		/*sned the g_st_test_hdr*/
        i_send_bytes = client_send_data(i_connect_fd, (char *)&g_st_test_hdr, sizeof(g_st_test_hdr));
		if (i_ret == FILE_CLIENT_ERROR) {
            file_error("[%s]client_send_data is fail\n", __FUNCTION__);
			return FILEDATA_DEAL_RET_FAIL;
		}
		file_printf("client_send_data send hdr data %d bytes\n", i_send_bytes);
#if 0
		/*sned the cmd pack data*/
        i_send_bytes = client_send_data(i_connect_fd, c_pack_buf_a, i_pack_len);
		if (i_send_bytes == FILE_CLIENT_ERROR) {
            file_error("[%s]client_send_data is fail\n", __FUNCTION__);
			return FILEDATA_DEAL_RET_FAIL;
		}
		file_printf("client_send_data send protobuf pack data %d bytes\n", i_send_bytes);
		file_printf("c_pack_buf_a = %s\n", c_pack_buf_a);
		file_printf("c_pack_buf_a size = %d\n", g_st_test_hdr.ui_dat_len);
        for (int i = 0; i < g_st_test_hdr.ui_dat_len; i++) {
            file_printf("%hhX\n", c_pack_buf_a[i]);
		}
#endif
        /*recv file description from server*/
        file_running("server file as follow:\n");
		while (1) {    
			memset(c_filename_buf_a, 0, sizeof(c_filename_buf_a));
			i_ret= client_recv_data(i_connect_fd, c_filename_buf_a, sizeof(c_filename_buf_a));
            if (i_ret == FILE_CLIENT_ERROR) {
                file_error("[%s]client_recv_data is fail\n", __FUNCTION__); 
				return FILEDATA_DEAL_RET_OK;
			}
            
            if (strncmp(c_filename_buf_a, "end", 3) == 0) {
                file_printf("recv filename data complete!\n");
				break;
			}
			
            file_running("%s\n", c_filename_buf_a);		
		}
	}
	if (g_st_test_hdr.en_module == MODULE_TEST_JSON) {

	}
	if (g_st_test_hdr.en_module == MODULE_TEST_TLV) {

	}	
}

int datadeal_file_exit(int i_connect_fd)
{
    int i_ret = FILE_CLIENT_OK;
	int i_send_bytes = 0;

    /*pading the g_st_test_hdr structure*/
	g_st_test_hdr.en_version = VERSION_ONE;
	g_st_test_hdr.ui_dat_len = 0;
	g_st_test_hdr.us_hdr_len = sizeof(g_st_test_hdr);

	/*sned the g_st_test_hdr*/
    i_send_bytes = client_send_data(i_connect_fd, (char *)&g_st_test_hdr, sizeof(g_st_test_hdr));
	if (i_ret == FILE_CLIENT_ERROR) {
        file_error("[%s]client_send_data is fail\n", __FUNCTION__);
		return FILEDATA_DEAL_RET_FAIL;
	}
	file_printf("client_send_data send hdr data %d bytes\n", i_send_bytes); 

	return FILESTATE_MAX;
}


int datadeal_proto_pack(char *cp_unpack_buf, char **cp_pack_buf)
{
    assert(NULL != cp_unpack_buf);
	assert(NULL != cp_pack_buf);

	FILEDATA st_unpack_data;
	size_t filedata_len = 0;

    /*init the st_unpack_data*/
    file__data__init(&st_unpack_data);

	/*padding data*/
	st_unpack_data.p_cmd_buf = (char *)malloc(BUFSIZE);
	if (NULL == st_unpack_data.p_cmd_buf)
	{
        file_error("[%s]malloc is error!\n", __FUNCTION__);
	    return FILEDATA_DEAL_RET_FAIL;
	}
	strncpy(st_unpack_data.p_cmd_buf, cp_unpack_buf, BUFSIZE);
	st_unpack_data.p_data_buf = NULL;

	filedata_len = file__data__get_packed_size(&st_unpack_data);

	*cp_pack_buf = (char *)malloc(filedata_len);
	if (NULL == *cp_pack_buf)
	{
        file_error("[%s]malloc is error!\n", __FUNCTION__);
	    return FILEDATA_DEAL_RET_FAIL;
	}

	file__data__pack(&st_unpack_data, *cp_pack_buf);

	free(st_unpack_data.p_cmd_buf);

	return filedata_len;
}










