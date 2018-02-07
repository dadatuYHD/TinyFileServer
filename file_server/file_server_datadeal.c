#include <assert.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "Message.pb-c.h"
#include "file_server_datadeal.h"
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


int datadeal_proto_pack(FILEDATA *cp_unpack_buf, char **cp_pack_buf);
int datadeal_proto_unpack(char *cp_pack_buf, FILEDATA *cp_unpack_buf);
int datadeal_file_list_deal(const char * cp_filename, int i_connect_fd);
int datadeal_file_read(int i_fd, void * buf, ssize_t nbytes);
int datadeal_file_write(int i_fd, const void * buf, ssize_t nbytes);


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

/************************************************************
FUNCTION:datadeal_file_read()
Description:该函数主要用来读取文件
Arguments:
[i_fd][IN]：文件描述符
[buf][IN]：存储文件的buf
[nbytes][IN]：需要读取的文件大小
return:返回总共读取的bytes
************************************************************/
int datadeal_file_read(int i_fd, void * buf, ssize_t nbytes)
{
    ssize_t st_total_read_bytes = 0;
	ssize_t st_read_bytes = 0;

	while (st_total_read_bytes < nbytes) {
        st_read_bytes = read(i_fd, buf, nbytes - st_total_read_bytes); 
		if (-1 == st_read_bytes) {
            perror("read");
			return FILEDATA_DEAL_RET_FAIL;
		}

		st_total_read_bytes += st_read_bytes;
	}

	return st_total_read_bytes;
}

/************************************************************
FUNCTION:datadeal_file_write()
Description:该函数主要用来写入数据到文件描述符指定的文件
Arguments:
[i_fd][IN]：文件描述符
[buf][IN]：待读取文件数据的buf
[nbytes][IN]：需要写入的文件大小
return:返回总共读取的bytes
************************************************************/
int datadeal_file_write(int i_fd, const void * buf, ssize_t nbytes)
{
    ssize_t st_total_write_bytes = 0;
	ssize_t st_write_bytes = 0;

	while (st_total_write_bytes < nbytes) {
        st_write_bytes = write(i_fd, buf, nbytes - st_total_write_bytes); 
		if (-1 == st_write_bytes) {
            perror("read");
			return FILEDATA_DEAL_RET_FAIL;
		}

		st_total_write_bytes += st_write_bytes;
	}

	return st_total_write_bytes;
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
	char c_pack_buf_a[g_st_test_hdr.ui_dat_len + 1];
	FILEDATA st_unpack_buf;
	int i_send_bytes = 0;
	char c_end_buf_a[BUFSIZE];
	
    if (g_st_test_hdr.en_module == MODULE_TEST_PROTO) {
#if 1
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
		memset(&st_unpack_buf, 0, sizeof(st_unpack_buf));
		i_ret = datadeal_proto_unpack(c_pack_buf_a, &st_unpack_buf);
		if (i_ret == FILEDATA_DEAL_RET_FAIL) {
            file_error("[%s]datadeal_proto_unpack is error!\n", __FUNCTION__);
	        return FILEDATA_DEAL_RET_FAIL;      
		}

		/*judge the cmd data*/
		if (NULL != st_unpack_buf.p_cmd_buf) {
            if (st_unpack_buf.p_cmd_buf[0] != 'L') {
                file_printf("recv cmd list, \"L\" cmd check fail!\n");
                return FILEDATA_DEAL_RET_FAIL;
            } else {
                file_printf("recv cmd list, \"L\" cmd check success!\n");
                i_ret = datadeal_file_list_deal(".", i_connect_fd);
                if (i_ret == FILEDATA_DEAL_RET_FAIL) {
                    file_error("[%s]datadeal_file_list_deal is error!\n", __FUNCTION__);
                    return FILEDATA_DEAL_RET_FAIL;
                }
            }
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
#if 0        
        i_ret = datadeal_file_list_deal(".", i_connect_fd);
        if (i_ret == FILEDATA_DEAL_RET_FAIL) {
            file_error("[%s]datadeal_file_list_deal is error!\n", __FUNCTION__);
		    return FILEDATA_DEAL_RET_FAIL;
	    }
#endif
        /*send file describe end mark*/
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

/************************************************************
FUNCTION:datadeal_file_set()
Description:处理客户端发送过来的S命令
Arguments:
[i_connect_fd][IN]:建立连接之后的文件描述符
return:success return FILEDATA_DEAL_RET_OK and fail return FILEDATA_DEAL_RET_FAIL
************************************************************/
int datadeal_file_get(int i_connect_fd)
{
    int i_ret = FILEDATA_DEAL_RET_OK;
	int i_recv_data_bytes = 0;
	char c_pack_buf_a[g_st_test_hdr.ui_dat_len + 1];
	FILEDATA st_unpack_buf;
	int i_send_bytes = 0;        
}

/************************************************************
FUNCTION:datadeal_file_set()
Description:处理客户端发送过来的S命令
Arguments:
[i_connect_fd][IN]:建立连接之后的文件描述符
return:success return FILEDATA_DEAL_RET_OK and fail return FILEDATA_DEAL_RET_FAIL
************************************************************/
int datadeal_file_set(int i_connect_fd)
{
    int i_ret = FILEDATA_DEAL_RET_OK;
	int i_recv_data_bytes = 0;
	char c_pack_buf_a[g_st_test_hdr.ui_dat_len + 1];
	FILEDATA st_unpack_buf;
	int i_send_bytes = 0;
	int i_file_fd = 0;
	ssize_t sst_write_bytes = 0;
	
    if (g_st_test_hdr.en_module == MODULE_TEST_PROTO) {
		/*recv pack data from client*/
		memset(c_pack_buf_a, 0, sizeof(c_pack_buf_a));
        i_recv_data_bytes = server_recv_data(i_connect_fd, (char *)c_pack_buf_a, g_st_test_hdr.ui_dat_len); 
	    if (i_recv_data_bytes == FILE_SERVER_ERROR) {
            file_error("[%s]server_recv_data is error, close server!!\n", __FUNCTION__);
		    close(i_connect_fd);
		    return FILEDATA_DEAL_RET_FAIL;
	    } else if (i_recv_data_bytes == FILE_SERVER_RECV_PEER_DOWN) {
            file_running("[%s]client close the connect!\n", __FUNCTION__);    
	    } else {
			/*check having read file size */
            if (i_recv_data_bytes != g_st_test_hdr.ui_dat_len) {
			    file_running("file size is checking fail!\n");	
			    return FILEDATA_DEAL_RET_FAIL; 
		    }
            file_running("recv client data is success!\n");
			file_printf("server_recv_data recv protobuf pack data %d bytes\n", i_recv_data_bytes);
        }

        file_printf("c_pack_buf_a = %s\n", c_pack_buf_a);
		file_printf("c_pack_buf_a size = %d\n", sizeof(c_pack_buf_a));
        for (int i = 0; i < g_st_test_hdr.ui_dat_len; i++) {
            file_printf("%X\n", c_pack_buf_a[i]);
		}   
		
		/*unpack the pack data*/
		memset(&st_unpack_buf, 0, sizeof(st_unpack_buf));
		i_ret = datadeal_proto_unpack(c_pack_buf_a, &st_unpack_buf);
		if (i_ret == FILEDATA_DEAL_RET_FAIL) {
            file_error("[%s]datadeal_proto_unpack is error!\n", __FUNCTION__);
	        return FILEDATA_DEAL_RET_FAIL;      
		}

		/*check the cmd data*/
		if (NULL != st_unpack_buf.p_cmd_buf) {
            if (st_unpack_buf.p_cmd_buf[0] != 'S') {
                file_printf("recv set file ,  \"S\" cmd check fail!\n");
                return FILEDATA_DEAL_RET_FAIL;
            } else {
                file_printf("recv set file ,  \"S\" cmd check success!\n");
            }
		}

		
		/*open the file*/
		i_file_fd = open(st_unpack_buf.p_filename_buf, O_WRONLY|O_CREAT|O_EXCL);
		if (-1 == i_file_fd) {
            perror("open");
			file_error("[%s]open is fail!\n", __FUNCTION__);
		    return FILEDATA_DEAL_RET_FAIL; 
		}

		/*write data to filename*/
		if (1 == st_unpack_buf.has_i_file_size) {
            sst_write_bytes = datadeal_file_write(i_file_fd, st_unpack_buf.p_filedata_buf, st_unpack_buf.i_file_size);
            if (-1 == sst_write_bytes) {
                file_error("[%s]datadeal_file_write is fail!\n", __FUNCTION__);
        		return FILEDATA_DEAL_RET_FAIL;     
        	}	
		} else {
            sst_write_bytes = datadeal_file_write(i_file_fd, st_unpack_buf.p_filedata_buf, BUFSIZE);
            if (-1 == sst_write_bytes) {
                file_error("[%s]datadeal_file_write is fail!\n", __FUNCTION__);
        		return FILEDATA_DEAL_RET_FAIL;     
        	}	    
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
		close(i_file_fd);
	} else if (g_st_test_hdr.en_module == MODULE_TEST_JSON) {
		
	} else if (g_st_test_hdr.en_module == MODULE_TEST_TLV) {

	} else {
        /*end of else*/
	}

	return FILEDATA_DEAL_RET_OK;                  
}
