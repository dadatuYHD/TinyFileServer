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
#include "file_client_input.h"


#define BUFSIZE 1024

TEST_HDR_T g_st_test_hdr;    //全局数据头部

int datadeal_proto_pack(FILEDATA *cp_unpack_buf, char **cp_pack_buf);
int datadeal_proto_unpack(char *cp_pack_buf, FILEDATA *cp_unpack_buf);

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

/************************************************************
FUNCTION:datadeal_file_set()
Description:该函数主要用来获取服务器保存的文件
Arguments:
[i_connect_fd][IN]：已经建立好连接的文件描述符
return:返回g_st_test_hdr
************************************************************/
int datadeal_file_set(int i_connect_fd)
{
	FILEDATA st_unpack_buf;
	char *cp_pack_buf = NULL;
	int i_pack_len = 0;
	int i_ret = FILE_CLIENT_OK;
	int i_send_bytes = 0;
	char c_filename_buf_a[BUFSIZE];

    if (g_st_test_hdr.en_module == MODULE_TEST_PROTO) {
		memset(&st_unpack_buf, 0, BUFSIZE);
		st_unpack_buf.p_cmd_buf = (char *)malloc(BUFSIZE);
		st_unpack_buf.p_filename_buf = (char *)malloc(BUFSIZE);
		st_unpack_buf.p_filedata_buf = NULL;
        strncpy(st_unpack_buf.p_cmd_buf, "S", 1);
		st_unpack_buf.p_cmd_buf[1] = '\0';	

		file_running("Please input file name:\n");
		i_ret = file_input_string(st_unpack_buf.p_filename_buf);
		if (i_ret == FILEINPUT_RET_FAIL) {
            file_error("[%s]file_input_string is fail!\n", __FUNCTION__);
			return FILEDATA_DEAL_RET_FAIL;    
		}

		i_pack_len = datadeal_proto_pack(&st_unpack_buf, &cp_pack_buf);
		if (i_pack_len == FILEDATA_DEAL_RET_FAIL) {
            file_error("[%s]datadeal_proto_pack is fail!\n", __FUNCTION__);
			return FILEDATA_DEAL_RET_FAIL;
		}

        if (NULL != st_unpack_buf.p_cmd_buf) {
            free(st_unpack_buf.p_cmd_buf);
		} 
		if (NULL != st_unpack_buf.p_filename_buf) {
            free(st_unpack_buf.p_filename_buf);   
		}
		if (NULL != st_unpack_buf.p_filedata_buf) {
            free(st_unpack_buf.p_filedata_buf);   
		}

		/*pading the g_st_test_hdr structure*/
		g_st_test_hdr.en_version = VERSION_ONE;
		g_st_test_hdr.ui_dat_len = i_pack_len;
		g_st_test_hdr.us_hdr_len = sizeof(g_st_test_hdr);

		/*sned the g_st_test_hdr*/
        i_send_bytes = client_send_data(i_connect_fd, (char *)&g_st_test_hdr, sizeof(g_st_test_hdr));
		if (i_ret == FILE_CLIENT_ERROR) {
            file_error("[%s]client_send_data is fail\n", __FUNCTION__);
			return FILEDATA_DEAL_RET_FAIL;
		}
		file_printf("client_send_data send hdr data %d bytes\n", i_send_bytes);

		/*sned the pack data of cmd filename and file content */
        i_send_bytes = client_send_data(i_connect_fd, cp_pack_buf, i_pack_len);
		if (i_send_bytes == FILE_CLIENT_ERROR) {
            file_error("[%s]client_send_data is fail\n", __FUNCTION__);
			return FILEDATA_DEAL_RET_FAIL;
		}
		file_printf("client_send_data send protobuf pack data %d bytes\n", i_send_bytes);
		file_printf("c_pack_buf_a = %s\n", cp_pack_buf);
		file_printf("c_pack_buf_a size = %d\n", g_st_test_hdr.ui_dat_len);
        for (int i = 0; i < g_st_test_hdr.ui_dat_len; i++) {
            file_printf("%hhX\n", cp_pack_buf[i]);
		}
		
        if (NULL != cp_pack_buf) {
            free(cp_pack_buf);
		}	
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
    if (g_st_test_hdr.en_module == MODULE_TEST_PROTO) {
		
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
#if 1
    FILEDATA st_unpack_buf;
	char *cp_pack_buf = NULL;
	int i_pack_len = 0;
#endif
	int i_ret = FILE_CLIENT_OK;
	int i_send_bytes = 0;
	char c_filename_buf_a[BUFSIZE];

    if (g_st_test_hdr.en_module == MODULE_TEST_PROTO) {
#if 1
        memset(&st_unpack_buf, 0, BUFSIZE);
        st_unpack_buf.p_cmd_buf = (char *)malloc(BUFSIZE);
		st_unpack_buf.p_filename_buf = NULL;
		st_unpack_buf.p_filedata_buf = NULL;
        strncpy(st_unpack_buf.p_cmd_buf, "L", 1);
		st_unpack_buf.p_cmd_buf[1] = '\0';	
		
		i_pack_len = datadeal_proto_pack(&st_unpack_buf, &cp_pack_buf);
		if (i_pack_len == FILEDATA_DEAL_RET_FAIL) {
            file_error("[%s]datadeal_proto_pack is fail!\n", __FUNCTION__);
			return FILEDATA_DEAL_RET_FAIL;
		}

        if (NULL != st_unpack_buf.p_cmd_buf) {
            free(st_unpack_buf.p_cmd_buf);
		} 
		if (NULL != st_unpack_buf.p_filename_buf) {
            free(st_unpack_buf.p_filename_buf);   
		}
		if (NULL != st_unpack_buf.p_filedata_buf) {
            free(st_unpack_buf.p_filedata_buf);   
		}
#endif
        /*pading the g_st_test_hdr structure*/
		g_st_test_hdr.en_version = VERSION_ONE;
		g_st_test_hdr.ui_dat_len = i_pack_len;
		g_st_test_hdr.us_hdr_len = sizeof(g_st_test_hdr);

		/*sned the g_st_test_hdr*/
        i_send_bytes = client_send_data(i_connect_fd, (char *)&g_st_test_hdr, sizeof(g_st_test_hdr));
		if (i_ret == FILE_CLIENT_ERROR) {
            file_error("[%s]client_send_data is fail\n", __FUNCTION__);
			return FILEDATA_DEAL_RET_FAIL;
		}
		file_printf("client_send_data send hdr data %d bytes\n", i_send_bytes);
#if 1
		/*sned the cmd pack data*/
        i_send_bytes = client_send_data(i_connect_fd, cp_pack_buf, i_pack_len);
		if (i_send_bytes == FILE_CLIENT_ERROR) {
            file_error("[%s]client_send_data is fail\n", __FUNCTION__);
			return FILEDATA_DEAL_RET_FAIL;
		}
		file_printf("client_send_data send protobuf pack data %d bytes\n", i_send_bytes);
		file_printf("c_pack_buf_a = %s\n", cp_pack_buf);
		file_printf("c_pack_buf_a size = %d\n", g_st_test_hdr.ui_dat_len);
        for (int i = 0; i < g_st_test_hdr.ui_dat_len; i++) {
            file_printf("%hhX\n", cp_pack_buf[i]);
		}
#endif
        if (NULL != cp_pack_buf) {
            free(cp_pack_buf);
		}

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

/************************************************************
FUNCTION:datadeal_proto_pack()
Description:以protobuf协议编码数据
Arguments:
[stp_unpack_buf][IN]：指向存放原始数据的内存
[cp_pack_buf][OUT]：保存打包完成的数据
return:返回打包之后数据的大小
************************************************************/
int datadeal_proto_pack(FILEDATA *stp_unpack_buf, char **cp_pack_buf)
{
    assert(NULL != stp_unpack_buf);
	assert(NULL != cp_pack_buf);

	FILEDATA st_unpack_data;
	size_t filedata_len = 0;

    /*init the st_unpack_data*/
    file__data__init(&st_unpack_data);

	/*padding data*/
	if (NULL != stp_unpack_buf->p_cmd_buf)
	{    
	    st_unpack_data.p_cmd_buf = (char *)malloc(BUFSIZE);
		if (NULL == st_unpack_data.p_cmd_buf) {
            file_error("[%s]malloc is error!\n", __FUNCTION__);
			return FILEDATA_DEAL_RET_FAIL;  
		} 
		strncpy(st_unpack_data.p_cmd_buf, stp_unpack_buf->p_cmd_buf, BUFSIZE);  
	} else {
        st_unpack_data.p_cmd_buf = NULL;
	}
	
	if (NULL != stp_unpack_buf->p_filename_buf) {
	    st_unpack_data.p_filename_buf = (char *)malloc(BUFSIZE);
		if (NULL == st_unpack_data.p_filename_buf) {
            file_error("[%s]malloc is error!\n", __FUNCTION__);
			return FILEDATA_DEAL_RET_FAIL;  
		} 
        strncpy(st_unpack_data.p_filename_buf, stp_unpack_buf->p_filename_buf, BUFSIZE);    
	} else {
        st_unpack_data.p_filename_buf = NULL;
	}

	if (NULL != stp_unpack_buf->p_filedata_buf) {
	    st_unpack_data.p_filedata_buf = (char *)malloc(BUFSIZE);
		if (NULL == st_unpack_data.p_filedata_buf) {
            file_error("[%s]malloc is error!\n", __FUNCTION__);
			return FILEDATA_DEAL_RET_FAIL;  
		} 
        strncpy(st_unpack_data.p_filedata_buf, stp_unpack_buf->p_filedata_buf, BUFSIZE);    
	} else {
        st_unpack_data.p_filedata_buf = NULL;
	}

    /*get size of pack data*/
	filedata_len = file__data__get_packed_size(&st_unpack_data);

	*cp_pack_buf = (char *)malloc(filedata_len);
	if (NULL == *cp_pack_buf)
	{
        file_error("[%s]malloc is error!\n", __FUNCTION__);
	    return FILEDATA_DEAL_RET_FAIL;
	}

	file__data__pack(&st_unpack_data, *cp_pack_buf);

    if (NULL != st_unpack_data.p_cmd_buf) {
        free(st_unpack_data.p_cmd_buf);    
	}
	if (NULL != st_unpack_data.p_filename_buf) {
        free(st_unpack_data.p_filename_buf);     
	}
	if (NULL != st_unpack_data.p_filedata_buf) {
        free(st_unpack_data.p_filedata_buf);     
	}
	
	return filedata_len;
}

/************************************************************
FUNCTION:datadeal_proto_unpack()
Description:以protobuf协议解码数据
Arguments:
[cp_pack_buf][IN]：待解包的数据
[stp_unpack_buf][OUT]:存放解包之后的数据
return:success return FILEDATA_DEAL_RET_OK and fail return FILEDATA_DEAL_RET_FAIL
************************************************************/
int datadeal_proto_unpack(char *cp_pack_buf, FILEDATA *stp_unpack_buf)
{
    assert(NULL != cp_pack_buf);  
	assert(NULL != stp_unpack_buf);

	FILEDATA *st_unpack_data = NULL;

	st_unpack_data = file__data__unpack(NULL, g_st_test_hdr.ui_dat_len, cp_pack_buf);
    if (NULL == st_unpack_data) {
        file_error("[%s]file__data__unpack is error!\n", __FUNCTION__);
	    return FILEDATA_DEAL_RET_FAIL;    
	}
  
    if (NULL != st_unpack_data->p_cmd_buf) {
        stp_unpack_buf->p_cmd_buf = (char *)malloc(g_st_test_hdr.ui_dat_len + 1);
        if (NULL == stp_unpack_buf) {
            file_error("[%s]malloc is error!\n", __FUNCTION__);
            return FILEDATA_DEAL_RET_FAIL;    
        }
        strcpy(stp_unpack_buf->p_cmd_buf, st_unpack_data->p_cmd_buf);    
	}

	if (NULL != st_unpack_data->p_filename_buf) {
        stp_unpack_buf->p_filename_buf = (char *)malloc(g_st_test_hdr.ui_dat_len + 1);
        if (NULL == stp_unpack_buf) {
            file_error("[%s]malloc is error!\n", __FUNCTION__);
            return FILEDATA_DEAL_RET_FAIL;    
        }
        strcpy(stp_unpack_buf->p_filename_buf, st_unpack_data->p_filename_buf);    
	}

	if (NULL != st_unpack_data->p_filedata_buf) {
        stp_unpack_buf->p_filedata_buf = (char *)malloc(g_st_test_hdr.ui_dat_len + 1);
        if (NULL == stp_unpack_buf) {
            file_error("[%s]malloc is error!\n", __FUNCTION__);
            return FILEDATA_DEAL_RET_FAIL;    
        }
        strcpy(stp_unpack_buf->p_filedata_buf, st_unpack_data->p_filedata_buf);    
	}

	file__data__free_unpacked(st_unpack_data,NULL);

	return FILEDATA_DEAL_RET_OK;
}











