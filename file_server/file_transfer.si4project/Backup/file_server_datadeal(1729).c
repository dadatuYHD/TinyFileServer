#include <assert.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include "file_server_datadeal.h"
#include "file_debug.h"
#include "Message.pb-c.h"
#include "file_server.h"


#define BUFSIZE 1024

TEST_HDR_T gst_test_hdr;    //全局数据头部

int datadeal_proto_pack(char *cp_unpack_buf, char **cp_pack_buf);

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

int datadeal_proto_pack(char *cp_unpack_buf, char **cp_pack_buf)
{
    assert(NULL != cp_unpack_buf);
	assert(NULL != cp_pack_buf);

	FILEDATA st_unpack_data;
	size_t filedata_len = 0;

    /*init the st_unpack_data*/
    file__data__init(&st_unpack_data);

	/*padding data*/
	st_unpack_data.p_cmd_buf = (char *)malloc(sizeof(BUFSIZE));
	if (NULL == st_unpack_data.p_cmd_buf)
	{
        file_error("[%s]malloc is error!\n", __FUNCTION__);
	    return FILEDATA_DEAL_RET_FAIL;
	}
	strncpy(st_unpack_data.p_cmd_buf, cp_unpack_buf, sizeof(BUFSIZE));
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

int datadeal_proto_unpack(char *cp_pack_buf, char **cp_unpack_buf)
{
    assert(NULL != cp_pack_buf);  
	assert(NULL != cp_unpack_buf);

	
}


int datadeal_file_list(int i_connect_fd)
{
           
}

int datadeal_file_get(int i_connect_fd)
{
        
}

int datadeal_file_set(int i_connect_fd)
{
        
}








