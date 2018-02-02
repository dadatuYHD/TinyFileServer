#include <assert.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "file_server_datadeal.h"
#include "Message.pb-c.h"
#include "file_server_debug.h"
#include "file_server.h"
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>




#define BUFSIZE 1024

TEST_HDR_T g_st_test_hdr;    //全局数据头部
TEST_HDR_T *g_stp_test_hdr;   //全局数据头部指针


int datadeal_proto_pack(char *cp_unpack_buf, char **cp_pack_buf);
int datadeal_file_list_deal(const char * cp_filename, int i_connect_fd);

/************************************************************
FUNCTION:datadeal_get_hdr()
Description:该函数主要用来读取g_st_test_hdr全局变量的值
Arguments:无
return:返回g_st_test_hdr
************************************************************/
TEST_HDR_T datadeal_get_hdr(void)
{
    return g_st_test_hdr;
}

/************************************************************
FUNCTION:datadeal_get_phdr()
Description:该函数主要用来读取g_st_test_hdr全局变量的地址
Arguments:无
return:返回g_st_test_hdr address
************************************************************/
TEST_HDR_T * datadeal_get_phdr(void)
{
    return &g_st_test_hdr;
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

    if (en_flg == HDR_FIELD_VERSION)
    {
        g_st_test_hdr.en_version = pst_test_hdr->en_version;   
	}
	else if (en_flg == HDR_FIELD_HDR_LEN)
    {
        g_st_test_hdr.us_hdr_len = pst_test_hdr->us_hdr_len;   
	}
    else if (en_flg == HDR_FIELD_DATA_LEN)
    {
        g_st_test_hdr.ui_dat_len = pst_test_hdr->ui_dat_len;   
	}
	else if (en_flg == HDR_FIELD_MODULE)
    {
        g_st_test_hdr.en_module = pst_test_hdr->en_module;   
	}
	else if (en_flg == HDR_FIELD_CMD)
    {
        g_st_test_hdr.en_cmd = pst_test_hdr->en_cmd;   
	}
	else
	{
	    file_error("[%s]datadeal_set_hdr input flag is error!\n", __FUNCTION__);
        return FILEDATA_DEAL_RET_FAIL;   
	}

	return FILEDATA_DEAL_RET_OK;
}

/************************************************************
FUNCTION:datadeal_proto_pack()
Description:以protobuf协议编码数据
Arguments:
[cp_unpack_buf][IN]：指向存放原始数据的内存
[cp_pack_buf][OUT]：保存打包完成的数据
return:返回打包之后数据的大小
************************************************************/
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

/************************************************************
FUNCTION:datadeal_proto_unpack()
Description:以protobuf协议解码数据
Arguments:
[cp_pack_buf][IN]：待解包的数据
[cp_unpack_buf][OUT]:存放解包之后的数据
return:success return FILEDATA_DEAL_RET_OK and fail return FILEDATA_DEAL_RET_FAIL
************************************************************/
int datadeal_proto_unpack(char *cp_pack_buf, char **cp_unpack_buf)
{
    assert(NULL != cp_pack_buf);  
	assert(NULL != cp_unpack_buf);

	FILEDATA *st_unpack_data = NULL;

	st_unpack_data = file__data__unpack(NULL, g_st_test_hdr.ui_dat_len, cp_pack_buf);
    if (NULL == st_unpack_data) {
        file_error("[%s]file__data__unpack is error!\n", __FUNCTION__);
	    return FILEDATA_DEAL_RET_FAIL;    
	}

    st_unpack_data->p_data_buf = NULL;

	*cp_unpack_buf = (char *)malloc(g_st_test_hdr.ui_dat_len + 1);
	if (NULL == *cp_unpack_buf) {
        file_error("[%s]malloc is error!\n", __FUNCTION__);
	    return FILEDATA_DEAL_RET_FAIL;    
    }
	strcpy(*cp_unpack_buf, st_unpack_data->p_cmd_buf);

	file__data__free_unpacked(st_unpack_data,NULL);

	return FILEDATA_DEAL_RET_OK;
}

/************************************************************
FUNCTION:datadeal_file_list()
Description:处理客户端发送过来的L命令
Arguments:
[i_connect_fd][IN]:建立连接之后的文件描述符
return:success return FILEDATA_DEAL_RET_OK and fail return FILEDATA_DEAL_RET_FAIL
************************************************************/
int datadeal_file_list(int i_connect_fd)
{
    int i_ret = FILEDATA_DEAL_RET_OK;
	int i_recv_data_bytes = 0;
	//char c_pack_buf_a[g_st_test_hdr.ui_dat_len + 1];
	//char * pc_unpack_buf = NULL;
	int i_send_bytes = 0;
	char c_end_buf_a[BUFSIZE];
	
    if (g_st_test_hdr.en_module == MODULE_TEST_PROTO) {
#if 0
		/*recv cmd data from client*/
		memset(c_pack_buf_a, 0, sizeof(c_pack_buf_a));
        i_recv_data_bytes = server_recv_data(i_connect_fd, (char *)c_pack_buf_a, g_st_test_hdr.ui_dat_len); 
	    if (i_recv_data_bytes == FILE_SERVER_ERROR) {
            file_error("[%s]server_recv_data is error, close server!!\n", __FUNCTION__);
		    close(i_connect_fd);
		    return FILEDATA_DEAL_RET_FAIL;
	    } else if (i_recv_data_bytes == FILE_SERVER_RECV_PEER_DOWN) {
            file_running("[%s]client close the connect!\n", __FUNCTION__);    
	    } else {
            file_running("recv client data is success!\n");
			file_printf("server_recv_data recv protobuf pack data %d bytes\n", i_recv_data_bytes);
        }

        file_printf("c_pack_buf_a = %s\n", c_pack_buf_a);
		file_printf("c_pack_buf_a size = %d\n", sizeof(c_pack_buf_a));
        for (int i = 0; i < g_st_test_hdr.ui_dat_len; i++) {
            file_printf("%X\n", c_pack_buf_a[i]);
		}   
		
		/*unpack the cmd data*/
		i_ret = datadeal_proto_unpack(c_pack_buf_a, &pc_unpack_buf);
		if (i_ret == FILEDATA_DEAL_RET_FAIL) {
            file_error("[%s]datadeal_proto_unpack is error!\n", __FUNCTION__);
	        return FILEDATA_DEAL_RET_FAIL;      
		}

		/*judge the cmd data*/
		switch (pc_unpack_buf[0]) {
            case 'L':
				file_printf("success recv cmd list!\n");
		        i_ret = datadeal_file_list_deal(".", i_connect_fd);
                if (i_ret == FILEDATA_DEAL_RET_FAIL) {
                    file_error("[%s]datadeal_file_list_deal is error!\n", __FUNCTION__);
					return FILEDATA_DEAL_RET_FAIL;
				}
				break;
		    case 'S':
				break;
			case 'G':
				break;
			default:
			    break;
		}
		file_printf("success recv cmd list!\n");
#endif
        
        i_ret = datadeal_file_list_deal(".", i_connect_fd);
        if (i_ret == FILEDATA_DEAL_RET_FAIL) {
            file_error("[%s]datadeal_file_list_deal is error!\n", __FUNCTION__);
		    return FILEDATA_DEAL_RET_FAIL;
	    }

		memset(c_end_buf_a, 0, sizeof(c_end_buf_a));
		strncpy(c_end_buf_a, "end", 3);
		c_end_buf_a[strlen(c_end_buf_a)] = '\0';
        i_send_bytes = server_send_data(i_connect_fd, c_end_buf_a, sizeof(c_end_buf_a));
	    if (FILE_SERVER_ERROR == i_send_bytes) {
            file_error("[%s]server_send_data is fail\n", __FUNCTION__);
		    return FILEDATA_DEAL_RET_FAIL;   
	    }	
	} else if (g_st_test_hdr.en_module == MODULE_TEST_JSON) {
        
	} else if (g_st_test_hdr.en_module == MODULE_TEST_TLV) {

	} else {
        /*end of else*/
	}

	return FILEDATA_DEAL_RET_OK;  
}

/************************************************************
FUNCTION:datadeal_file_list_deal()
Description:递归描述目录下的文件情况
Arguments:
[cp_filename][IN]:目录名字
[i_connect_fd][IN]:建立连接之后的文件描述符
return:success return FILEDATA_DEAL_RET_OK and fail return FILEDATA_DEAL_RET_FAIL
************************************************************/
int datadeal_file_list_deal(const char * cp_filename, int i_connect_fd)
{
    DIR * p_mydir = NULL;
	struct dirent * stp_dir_item = NULL;
	int i_ret = FILEDATA_DEAL_RET_OK;
	int i_send_bytes = 0;
	struct stat st_file_info;
	char c_filename_buf_a[BUFSIZE];

    p_mydir = opendir(cp_filename);
	if (NULL == p_mydir) {
        perror("opendir"); 
	    return FILEDATA_DEAL_RET_FAIL;
	} 

    while (NULL != (stp_dir_item = readdir(p_mydir))) {
        if ((strcmp(stp_dir_item->d_name, ".") == 0) || (strcmp(stp_dir_item->d_name, "..") == 0)) {
            continue;
		}

        memset(&st_file_info, 0, sizeof(st_file_info));
		sprintf(c_filename_buf_a, "%s/%s", cp_filename, stp_dir_item->d_name);
        i_ret = stat(c_filename_buf_a, &st_file_info);
		if (-1 == i_ret) {
            perror("stat");
			return FILEDATA_DEAL_RET_FAIL; 
		}

		if (S_ISDIR(st_file_info.st_mode)) {
            file_printf("[DIR]stp_dir_item->d_name = %s\n", c_filename_buf_a);
            i_ret = datadeal_file_list_deal(c_filename_buf_a, i_connect_fd);
			if (FILEDATA_DEAL_RET_FAIL == i_ret) {
                file_error("[%s]datadeal_file_list_deal is error!\n", __FUNCTION__);
				return FILEDATA_DEAL_RET_FAIL;    
			}
	    } else {
		    i_send_bytes = server_send_data(i_connect_fd, c_filename_buf_a, sizeof(c_filename_buf_a));
		    if (FILE_SERVER_ERROR == i_send_bytes) {
                file_error("[%s]server_send_data is fail\n", __FUNCTION__);
			    return FILEDATA_DEAL_RET_FAIL;   
		    }
	        file_printf("client_send_data send filename data %d bytes\n", i_send_bytes);    
		}
	}	
	closedir(p_mydir);
}

int datadeal_file_get(int i_connect_fd)
{
        
}

int datadeal_file_set(int i_connect_fd)
{
        
}
