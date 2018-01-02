#include <assert.h>
#include <stdio.h>
#include <assert.h>
#include "file_client_datadeal.h"
#include "file_debug.h"
#include "Message.pb-c.h"


#define BUFSIZE 1024

TEST_HDR_T gst_test_hdr;    //全局数据头部

int datadeal_proto_pack(char *p_unpack_buf, char *p_pack_buf);


/************************************************************
FUNCTION:datadeal_get_hdr()
Description:该函数主要用来文件间传递gst_test_hdr全局变量
Arguments:无
return:返回gst_test_hdr
************************************************************/
TEST_HDR_T datadeal_get_hdr(void)
{
    return gst_test_hdr;
}


/************************************************************
FUNCTION:datadeal_set_hdr()
Description:该函数主要用来文件间设置全局变量gst_test_hdr
Arguments:
[pst_test_hdr][IN]：指向一个TEST_HDR_T结构，存放有待设置的数据
[en_flg][IN]：设置标志位
return:返回gst_test_hdr
************************************************************/
int datadeal_set_hdr(TEST_HDR_T *pst_test_hdr, HDR_FIELD_FLG en_flg)
{
    assert(NULL != pst_test_hdr);

    if (en_flg == HDR_FIELD_VERSION)
    {
        gst_test_hdr.en_version = pst_test_hdr->en_version;   
	}
	else if (en_flg == HDR_FIELD_HDR_LEN)
    {
        gst_test_hdr.us_hdr_len = pst_test_hdr->us_hdr_len;   
	}
    else if (en_flg == HDR_FIELD_DATA_LEN)
    {
        gst_test_hdr.ui_dat_len = pst_test_hdr->ui_dat_len;   
	}
	else if (en_flg == HDR_FIELD_MODULE)
    {
        gst_test_hdr.en_module = pst_test_hdr->en_module;   
	}
	else if (en_flg == HDR_FIELD_CMD)
    {
        gst_test_hdr.en_cmd = pst_test_hdr->en_cmd;   
	}
	else
	{
	    file_error("[%s]datadeal_set_hdr input flag is error!\n", __FUNCTION__);
        return FILEDATA_DEAL_RET_FAIL;   
	}

	return FILEDATA_DEAL_RET_OK;
}

int datadeal_file_set(int i_connect_fd)
{
    
}

int datadeal_file_get(int i_connect_fd)
{

}


/************************************************************
FUNCTION:datadeal_file_list()
Description:该函数主要用来获取服务器保存的文件名字
Arguments:
[i_connect_fd][IN]：已经建立好连接的文件描述符
[en_flg][IN]：设置标志位
return:返回gst_test_hdr
************************************************************/

int datadeal_file_list(int i_connect_fd)
{
    char c_unpack_buf_a[BUFSIZE];
	int i_ret = FILEDATA_DEAL_RET_OK;


    if (gst_test_hdr.en_module == MODULE_TEST_PROTO)
    {
        strncpy(c_unpack_buf_a, 'L', 1);

	}
	if (gst_test_hdr.en_module == MODULE_TEST_JSON)
	{

	}
	if (gst_test_hdr.en_module == MODULE_TEST_TLV)
	{

	}
	
    i_ret = send(i_connect_fd, c_cmd_buf_a, sizeof(c_cmd_buf_a), 0);
	
}

int datadeal_proto_pack(char *p_unpack_buf, char *p_pack_buf)
{
    assert(NULL != p_unpack_buf);
	assert(NULL != p_pack_buf);

	FILEDATA st_unpack_data;
}










