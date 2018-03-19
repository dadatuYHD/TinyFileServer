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
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <cjson/cJSON.h>
#include "file_client_tlv.h"


#define BUFSIZE 1024

TEST_HDR_T g_st_test_hdr;    //全局数据头部

int datadeal_proto_pack(FILEDATA *cp_unpack_buf, char **cp_pack_buf);
int datadeal_proto_unpack(char *cp_pack_buf, FILEDATA *stp_unpack_buf, int i_size);
int datadeal_file_write(int i_fd, const void * buf, ssize_t nbytes);
int datadeal_file_read(int i_fd, void * buf, ssize_t nbytes);

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
FUNCTION:datadeal_file_set()
Description:该函数主要用来获取服务器保存的文件
Arguments:
[i_connect_fd][IN]：已经建立好连接的文件描述符
return:返回g_st_test_hdr
************************************************************/
int datadeal_file_set(int i_connect_fd)
{
	int i_ret = FILE_CLIENT_OK;
	int i_send_bytes = 0;
	int i_file_fd = 0;
	struct stat st_file_info;
	ssize_t sst_read_bytes = 0;
		
    if (g_st_test_hdr.en_module == MODULE_TEST_PROTO) {
		FILEDATA st_unpack_buf;
		char *cp_pack_buf = NULL;
		int i_pack_len = 0;	

		memset(&st_unpack_buf, 0, BUFSIZE);
		st_unpack_buf.p_cmd_buf = (char *)malloc(BUFSIZE);
	    if (NULL == st_unpack_buf.p_cmd_buf) {
            perror("malloc");
			file_error("[%s]malloc is fail!\n", __FUNCTION__);
			return FILEDATA_DEAL_RET_FAIL; 
		}
		st_unpack_buf.p_filename_buf = (char *)malloc(BUFSIZE);
	    if (NULL == st_unpack_buf.p_filename_buf) {
            perror("malloc");
			file_error("[%s]malloc is fail!\n", __FUNCTION__);
			return FILEDATA_DEAL_RET_FAIL; 
		}
		st_unpack_buf.p_filedata_buf = NULL;
        strncpy(st_unpack_buf.p_cmd_buf, "S", 1);
		st_unpack_buf.p_cmd_buf[1] = '\0';	

        /*read the filename*/
		file_running("Please input file name:\n");
		i_ret = file_input_string(st_unpack_buf.p_filename_buf);
		if (i_ret == FILEINPUT_RET_FAIL) {
            file_error("[%s]file_input_string is fail!\n", __FUNCTION__);
			return FILEDATA_DEAL_RET_FAIL;    
		}

		/*open the file*/
		i_file_fd = open(st_unpack_buf.p_filename_buf, O_RDONLY);
		if (-1 == i_file_fd) {
            perror("open");
			file_error("[%s]open is fail!\n", __FUNCTION__);
		    return FILEDATA_DEAL_RET_FAIL; 
		}

		/*Get the size of file*/
		memset(&st_file_info, 0, sizeof(st_file_info));
		i_ret = stat(st_unpack_buf.p_filename_buf, &st_file_info);
		if (-1 == i_ret) {
            perror("stat");
			file_error("[%s]stat is fail!\n", __FUNCTION__);
			return FILEDATA_DEAL_RET_FAIL; 
		}
		st_unpack_buf.i_file_size = st_file_info.st_size;
		st_unpack_buf.has_i_file_size = 1;
		

        /*read the file to st_unpack_buf.p_filedata_buf*/
		st_unpack_buf.p_filedata_buf = (char *)malloc(st_file_info.st_size);
		if (NULL == st_unpack_buf.p_filedata_buf) {
            perror("malloc");
			file_error("[%s]malloc is fail!\n", __FUNCTION__);
			return FILEDATA_DEAL_RET_FAIL; 
		}

		sst_read_bytes = datadeal_file_read(i_file_fd, st_unpack_buf.p_filedata_buf, st_file_info.st_size);
        if (-1 == sst_read_bytes) {
            file_error("[%s]datadeal_file_read is fail!\n", __FUNCTION__);
			return FILEDATA_DEAL_RET_FAIL;     
		}

        /*read file size check*/
        if (sst_read_bytes != st_file_info.st_size) {
			file_running("file read is fail!\n");	
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
		close(i_file_fd);

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

	    cJSON *stp_cjson_root = cJSON_CreateObject();
        char *cp_cjson_data_out = NULL;
	    int i_cjson_data_size = 0;
		char c_filename_buf_a[BUFSIZE];
		char *cp_file_data = NULL;

		/*read the filename*/
		file_running("Please input file name:\n");
		memset(&c_filename_buf_a, 0, BUFSIZE);
		i_ret = file_input_string(c_filename_buf_a);
		if (i_ret == FILEINPUT_RET_FAIL) {
            file_error("[%s]file_input_string is fail!\n", __FUNCTION__);
			return FILEDATA_DEAL_RET_FAIL;    
		}

		/*open the file*/
		i_file_fd = open(c_filename_buf_a, O_RDONLY);
		if (-1 == i_file_fd) {
            perror("open");
			file_error("[%s]open is fail!\n", __FUNCTION__);
		    return FILEDATA_DEAL_RET_FAIL; 
		}

		/*Get the size of file*/
		memset(&st_file_info, 0, sizeof(st_file_info));
		i_ret = stat(c_filename_buf_a, &st_file_info);
		if (-1 == i_ret) {
            perror("stat");
			file_error("[%s]stat is fail!\n", __FUNCTION__);
			return FILEDATA_DEAL_RET_FAIL; 
		}

        /*read the file to cp_file_data*/
		cp_file_data = (char *)malloc(st_file_info.st_size);
		if (NULL == cp_file_data) {
            perror("malloc");
			file_error("[%s]malloc is fail!\n", __FUNCTION__);
			return FILEDATA_DEAL_RET_FAIL; 
		}

		sst_read_bytes = datadeal_file_read(i_file_fd, cp_file_data, st_file_info.st_size);
        if (-1 == sst_read_bytes) {
            file_error("[%s]datadeal_file_read is fail!\n", __FUNCTION__);
			return FILEDATA_DEAL_RET_FAIL;     
		}

        /*read file size check*/
        if (sst_read_bytes != st_file_info.st_size) {
			file_running("file read is fail!\n");	
			return FILEDATA_DEAL_RET_FAIL; 
		}

        /*add the "S" cmd to the json structure*/
		cJSON_AddStringToObject(stp_cjson_root, "string_cmd", "S");
		/*add the filename to the json structure*/
		cJSON_AddStringToObject(stp_cjson_root, "string_file_name", c_filename_buf_a);
		/*add the file content data to the json structure*/
		cJSON_AddStringToObject(stp_cjson_root, "string_file_data", cp_file_data);
		/*add the file size to the json structure*/
		cJSON_AddNumberToObject(stp_cjson_root, "number_file_size", st_file_info.st_size);
		
		/*output the json data to char*/
		cp_cjson_data_out = cJSON_PrintUnformatted(stp_cjson_root);
		
		/*judge the data size of bytes*/
		i_cjson_data_size = strlen(cp_cjson_data_out);

	    /*pading the g_st_test_hdr structure*/
		g_st_test_hdr.en_version = VERSION_ONE;
		g_st_test_hdr.ui_dat_len = i_cjson_data_size + 1;
		g_st_test_hdr.us_hdr_len = sizeof(g_st_test_hdr);

		/*sned the g_st_test_hdr*/
        i_send_bytes = client_send_data(i_connect_fd, (char *)&g_st_test_hdr, sizeof(g_st_test_hdr));
		if (i_ret == FILE_CLIENT_ERROR) {
            file_error("[%s]client_send_data is fail\n", __FUNCTION__);
			return FILEDATA_DEAL_RET_FAIL;
		}
		file_printf("client_send_data send hdr data %d bytes\n", i_send_bytes);

		/*sned the cjson pack data*/
        i_send_bytes = client_send_data(i_connect_fd, cp_cjson_data_out, i_cjson_data_size + 1);
		if (i_send_bytes == FILE_CLIENT_ERROR) {
            file_error("[%s]client_send_data is fail\n", __FUNCTION__);
			return FILEDATA_DEAL_RET_FAIL;
		}

		file_printf("client_send_data send cjson pack data %d bytes\n", i_send_bytes);
		file_printf("cp_cjson_data_out = %s\n", cp_cjson_data_out);
		file_printf("cp_cjson_data_out size = %d\n", g_st_test_hdr.ui_dat_len);
        for (int i = 0; i < g_st_test_hdr.ui_dat_len; i++) {
            file_printf("%hhX\n", cp_cjson_data_out[i]);
		}

		if (NULL != stp_cjson_root) {
            cJSON_Delete(stp_cjson_root);
			stp_cjson_root = NULL;
		}
		if (NULL != cp_cjson_data_out) {
            free(cp_cjson_data_out);
			cp_cjson_data_out = NULL;
		}
	    if (NULL != cp_file_data) {
            free(cp_file_data);
			cp_file_data = NULL;
		}  
	}
	if (g_st_test_hdr.en_module == MODULE_TEST_TLV) {
		
		FILE_DATA_ST st_file_data;
		char c_pack_buf_a[BUFSIZE];
	    unsigned int ui_tlv_totol_len = 0; 
		int i_ret = TLV_ENCODE_RET_OK;

		/*read the filename*/
		file_running("Please input file name:\n");
		memset(&st_file_data, 0, sizeof(FILE_DATA_ST));
		i_ret = file_input_string(st_file_data.c_file_name);
		if (i_ret == FILEINPUT_RET_FAIL) {
            file_error("[%s]file_input_string is fail!\n", __FUNCTION__);
			return FILEDATA_DEAL_RET_FAIL;    
		}

		/*open the file*/
		i_file_fd = open(st_file_data.c_file_name, O_RDONLY);
		if (-1 == i_file_fd) {
            perror("open");
			file_error("[%s]open is fail!\n", __FUNCTION__);
		    return FILEDATA_DEAL_RET_FAIL; 
		}

		/*Get the size of file*/
		memset(&st_file_info, 0, sizeof(st_file_info));
		i_ret = stat(st_file_data.c_file_name, &st_file_info);
		if (-1 == i_ret) {
            perror("stat");
			file_error("[%s]stat is fail!\n", __FUNCTION__);
			return FILEDATA_DEAL_RET_FAIL; 
		}

        /*read the file to cp_file_data*/
		st_file_data.cp_file_content = (char *)malloc(st_file_info.st_size + 1);
		if (NULL == st_file_data.cp_file_content) {
            perror("malloc");
			file_error("[%s]malloc is fail!\n", __FUNCTION__);
			return FILEDATA_DEAL_RET_FAIL; 
		}

		sst_read_bytes = datadeal_file_read(i_file_fd, st_file_data.cp_file_content, st_file_info.st_size);
        if (-1 == sst_read_bytes) {
            file_error("[%s]datadeal_file_read is fail!\n", __FUNCTION__);
			return FILEDATA_DEAL_RET_FAIL;     
		}

        /*read file size check*/
        if (sst_read_bytes != st_file_info.st_size) {
			file_running("file read is fail!\n");	
			return FILEDATA_DEAL_RET_FAIL; 
		}

		/*pading the FILE_DATA_ST structure*/
	    strcpy(st_file_data.c_file_cmd, "S");
	    st_file_data.c_file_cmd[1] = '\0';
		st_file_data.ui_file_size = st_file_info.st_size;
		st_file_data.ui_data_totol_size = 12 + 12 + 4 + st_file_data.ui_file_size; 

		/*tlv encode the st_file_data*/
		i_ret = tlv_encode_file(&st_file_data, c_pack_buf_a, &ui_tlv_totol_len, BUFSIZE);
		if (TLV_ENCODE_RET_FAIL == i_ret) {
            file_error("[%s]tlv_encode_file is fail\n", __FUNCTION__); 
			return FILEDATA_DEAL_RET_FAIL;
		}

	    /*pading the g_st_test_hdr structure*/
		g_st_test_hdr.en_version = VERSION_ONE;
		g_st_test_hdr.ui_dat_len = ui_tlv_totol_len;
		g_st_test_hdr.us_hdr_len = sizeof(g_st_test_hdr);

		/*sned the g_st_test_hdr*/
        i_send_bytes = client_send_data(i_connect_fd, (char *)&g_st_test_hdr, sizeof(g_st_test_hdr));
		if (i_ret == FILE_CLIENT_ERROR) {
            file_error("[%s]client_send_data is fail\n", __FUNCTION__);
			return FILEDATA_DEAL_RET_FAIL;
		}
		file_printf("client_send_data send hdr data %d bytes\n", i_send_bytes);

		/*sned the tlv pack data*/
        i_send_bytes = client_send_data(i_connect_fd, c_pack_buf_a, ui_tlv_totol_len);
		if (i_send_bytes == FILE_CLIENT_ERROR) {
            file_error("[%s]client_send_data is fail\n", __FUNCTION__);
			return FILEDATA_DEAL_RET_FAIL;
		}

		file_printf("client_send_data send tlv pack data %d bytes\n", i_send_bytes);
		file_printf("c_pack_buf_a = %s\n", c_pack_buf_a);
		file_printf("ui_tlv_totol_len size = %d\n", g_st_test_hdr.ui_dat_len);
        for (int i = 0; i < g_st_test_hdr.ui_dat_len; i++) {
            file_printf("%hhX\n", c_pack_buf_a[i]);
		}

		if (NULL != st_file_data.cp_file_content) {
            free(st_file_data.cp_file_content);
		    st_file_data.cp_file_content = NULL;
		}
		close(i_file_fd);
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
    int i_ret = FILE_CLIENT_OK;
    int i_send_bytes = 0;
    int i_recv_data_bytes = 0;
    int i_file_fd = 0;
    TEST_HDR_T st_test_hdr;  
    ssize_t sst_write_bytes = 0;

    if (g_st_test_hdr.en_module == MODULE_TEST_PROTO) {
		
        FILEDATA st_unpack_buf;
        char *cp_pack_buf = NULL;
        int i_pack_len = 0;

	    memset(&st_unpack_buf, 0, BUFSIZE);
        st_unpack_buf.p_cmd_buf = (char *)malloc(BUFSIZE);
		if (NULL == st_unpack_buf.p_cmd_buf) {
            perror("malloc");
			file_error("[%s]malloc is fail!\n", __FUNCTION__);
			return FILEDATA_DEAL_RET_FAIL; 
		}
		st_unpack_buf.p_filename_buf = (char *)malloc(BUFSIZE);
	    if (NULL == st_unpack_buf.p_filename_buf) {
            perror("malloc");
			file_error("[%s]malloc is fail!\n", __FUNCTION__);
			return FILEDATA_DEAL_RET_FAIL; 
		}
		st_unpack_buf.p_filedata_buf = NULL;
		st_unpack_buf.has_i_file_size = 0;
        strncpy(st_unpack_buf.p_cmd_buf, "G", 1);
		st_unpack_buf.p_cmd_buf[1] = '\0';	

		/*read the filename*/
		file_running("Please input file name:\n");
		i_ret = file_input_string(st_unpack_buf.p_filename_buf);
		if (i_ret == FILEINPUT_RET_FAIL) {
            file_error("[%s]file_input_string is fail!\n", __FUNCTION__);
			return FILEDATA_DEAL_RET_FAIL;    
		}

		/*pack the origin data*/
		i_pack_len = datadeal_proto_pack(&st_unpack_buf, &cp_pack_buf);
		if (i_pack_len == FILEDATA_DEAL_RET_FAIL) {
            file_error("[%s]datadeal_proto_pack is fail!\n", __FUNCTION__);
			return FILEDATA_DEAL_RET_FAIL;
		}

		if (NULL != st_unpack_buf.p_cmd_buf) {
            free(st_unpack_buf.p_cmd_buf);
			st_unpack_buf.p_cmd_buf = NULL;
		} 
		if (NULL != st_unpack_buf.p_filename_buf) {
            free(st_unpack_buf.p_filename_buf);  
			st_unpack_buf.p_filename_buf = NULL;
		}
		if (NULL != st_unpack_buf.p_filedata_buf) {
            free(st_unpack_buf.p_filedata_buf);  
			st_unpack_buf.p_filedata_buf = NULL;
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

		/*sned the pack data*/
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
			cp_pack_buf = NULL;
		}

        /*recv file content size of hdr*/
		memset(&st_test_hdr, 0, sizeof(st_test_hdr));
		i_recv_data_bytes = client_recv_data(i_connect_fd, &st_test_hdr, sizeof(st_test_hdr)); 
	    if (i_recv_data_bytes == FILE_CLIENT_ERROR) {
            file_error("[%s]client_recv_data is error, close server!!\n", __FUNCTION__);
		    close(i_connect_fd);
		    return FILEDATA_DEAL_RET_FAIL;
	    } else if (i_recv_data_bytes == FILE_CLIENT_RECV_PEER_DOWN) {
            file_running("[%s]SERVER close the connect!\n", __FUNCTION__);    
	    } else {
            file_running("recv server hdr is success!\n");
			file_printf("client_recv_data recv protobuf pack data %d bytes\n", i_recv_data_bytes);
        } 

		/*recv pack file content data from sever*/
		cp_pack_buf = (char *)malloc(st_test_hdr.ui_dat_len);
		if (NULL == cp_pack_buf) {
            perror("malloc");
			file_error("[%s]malloc is fail!\n", __FUNCTION__);
			return FILEDATA_DEAL_RET_FAIL; 		
		}
        i_recv_data_bytes = client_recv_data(i_connect_fd, (char *)cp_pack_buf, st_test_hdr.ui_dat_len); 
	    if (i_recv_data_bytes == FILE_CLIENT_ERROR) {
            file_error("[%s]client_recv_data is error, close server!!\n", __FUNCTION__);
		    close(i_connect_fd);
		    return FILEDATA_DEAL_RET_FAIL;
	    } else if (i_recv_data_bytes == FILE_CLIENT_RECV_PEER_DOWN) {
            file_running("[%s]SERVER close the connect!\n", __FUNCTION__);    
	    } else {
			/*check having read file size */
            if (i_recv_data_bytes != st_test_hdr.ui_dat_len) {
			    file_running("file size is checking fail!\n");	
			    return FILEDATA_DEAL_RET_FAIL; 
		    }
            file_running("recv server data is success!\n");
			file_printf("client_recv_data recv protobuf pack data %d bytes\n", i_recv_data_bytes);
        } 
		
		/*unpack the pack data*/
		memset(&st_unpack_buf, 0, sizeof(st_unpack_buf));
		i_ret = datadeal_proto_unpack(cp_pack_buf, &st_unpack_buf, st_test_hdr.ui_dat_len);
		if (i_ret == FILEDATA_DEAL_RET_FAIL) {
            file_error("[%s]datadeal_proto_unpack is error!\n", __FUNCTION__);
	        return FILEDATA_DEAL_RET_FAIL;      
		}

		if (NULL != cp_pack_buf) {
            free(cp_pack_buf);
			cp_pack_buf = NULL;
		}
	
		/*open the file*/
		while (1) {
          	i_file_fd = open(st_unpack_buf.p_filename_buf, O_WRONLY|O_CREAT|O_EXCL);
          	if (-1 == i_file_fd) {
          		if (errno == EEXIST) {
					i_ret = remove(st_unpack_buf.p_filename_buf);
					if (-1 == i_ret) {
                        perror("remove");
						file_error("[%s]remove is error!\n", __FUNCTION__);
						return 0;
					}
                    continue;
          	    } else {
                    perror("open");
            		file_error("[%s]open is fail!\n", __FUNCTION__);
            	    return FILEDATA_DEAL_RET_FAIL;     
				}
          	} else {
                break;
			}   
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
		file_running("file write is success!\n");

		if (NULL != st_unpack_buf.p_cmd_buf) {
            free(st_unpack_buf.p_cmd_buf);
			st_unpack_buf.p_cmd_buf = NULL;
		}
		if (NULL != st_unpack_buf.p_filename_buf) {
            free(st_unpack_buf.p_filename_buf); 
			st_unpack_buf.p_cmd_buf = NULL;
		}
		if (NULL != st_unpack_buf.p_filedata_buf) {
            free(st_unpack_buf.p_filedata_buf); 
			st_unpack_buf.p_cmd_buf = NULL;
		}
		close(i_file_fd);
		
	}
	if (g_st_test_hdr.en_module == MODULE_TEST_JSON) {

	    cJSON *stp_cjson_root = cJSON_CreateObject();
        char *cp_cjson_data_out = NULL;
	    int i_cjson_data_size = 0;
		char c_filename_buf_a[BUFSIZE];
		char *cp_file_data = NULL; 

		/*read the filename*/
		file_running("Please input file name:\n");
		memset(&c_filename_buf_a, 0, BUFSIZE);
		i_ret = file_input_string(c_filename_buf_a);
		if (i_ret == FILEINPUT_RET_FAIL) {
            file_error("[%s]file_input_string is fail!\n", __FUNCTION__);
			return FILEDATA_DEAL_RET_FAIL;    
		}

		/*add the "G" cmd to the json structure*/
		cJSON_AddStringToObject(stp_cjson_root, "string_cmd", "G");
		/*add the filename to the json structure*/
		cJSON_AddStringToObject(stp_cjson_root, "string_file_name", c_filename_buf_a);
		
		/*output the json data to char*/
		cp_cjson_data_out = cJSON_PrintUnformatted(stp_cjson_root);
		
		/*judge the data size of bytes*/
		i_cjson_data_size = strlen(cp_cjson_data_out);

		/*pading the g_st_test_hdr structure*/
		g_st_test_hdr.en_version = VERSION_ONE;
		g_st_test_hdr.ui_dat_len = i_cjson_data_size + 1;
		g_st_test_hdr.us_hdr_len = sizeof(g_st_test_hdr);

		/*sned the g_st_test_hdr*/
        i_send_bytes = client_send_data(i_connect_fd, (char *)&g_st_test_hdr, sizeof(g_st_test_hdr));
		if (i_ret == FILE_CLIENT_ERROR) {
            file_error("[%s]client_send_data is fail\n", __FUNCTION__);
			return FILEDATA_DEAL_RET_FAIL;
		}
		file_printf("client_send_data send hdr data %d bytes\n", i_send_bytes);

		/*sned the cjson pack data*/
        i_send_bytes = client_send_data(i_connect_fd, cp_cjson_data_out, i_cjson_data_size + 1);
		if (i_send_bytes == FILE_CLIENT_ERROR) {
            file_error("[%s]client_send_data is fail\n", __FUNCTION__);
			return FILEDATA_DEAL_RET_FAIL;
		}
		file_printf("client_send_data send cjson pack data %d bytes\n", i_send_bytes);
		file_printf("cp_cjson_data_out = %s\n", cp_cjson_data_out);
		file_printf("cp_cjson_data_out size = %d\n", g_st_test_hdr.ui_dat_len);
        for (int i = 0; i < g_st_test_hdr.ui_dat_len; i++) {
            file_printf("%hhX\n", cp_cjson_data_out[i]);
		}

		cJSON_Delete(stp_cjson_root);
		if (NULL != cp_cjson_data_out) {
            free(cp_cjson_data_out);
			cp_cjson_data_out = NULL;
		}

		/*recv file content size of hdr*/
		memset(&st_test_hdr, 0, sizeof(st_test_hdr));
		i_recv_data_bytes = client_recv_data(i_connect_fd, &st_test_hdr, sizeof(st_test_hdr)); 
	    if (i_recv_data_bytes == FILE_CLIENT_ERROR) {
            file_error("[%s]client_recv_data is error, close server!!\n", __FUNCTION__);
		    close(i_connect_fd);
		    return FILEDATA_DEAL_RET_FAIL;
	    } else if (i_recv_data_bytes == FILE_CLIENT_RECV_PEER_DOWN) {
            file_running("[%s]SERVER close the connect!\n", __FUNCTION__);    
	    } else {
            file_running("recv server hdr is success!\n");
			file_printf("client_recv_data recv hdr data %d bytes\n", i_recv_data_bytes);
        } 

        cJSON * stp_json = NULL; 
		cJSON * stp_json_str_filename = NULL; 
		cJSON * stp_json_str_filedata = NULL; 
		cJSON * stp_json_num_filesize = NULL; 

		/*recv pack file content data from sever*/
		cp_cjson_data_out = (char *)malloc(st_test_hdr.ui_dat_len);
		if (NULL == cp_cjson_data_out) {
            perror("malloc");
			file_error("[%s]malloc is fail!\n", __FUNCTION__);
			return FILEDATA_DEAL_RET_FAIL; 		
		}
        i_recv_data_bytes = client_recv_data(i_connect_fd, (char *)cp_cjson_data_out, st_test_hdr.ui_dat_len); 
	    if (i_recv_data_bytes == FILE_CLIENT_ERROR) {
            file_error("[%s]client_recv_data is error, close server!!\n", __FUNCTION__);
		    return FILEDATA_DEAL_RET_FAIL;
	    } else if (i_recv_data_bytes == FILE_CLIENT_RECV_PEER_DOWN) {
            file_running("[%s]SERVER close the connect!\n", __FUNCTION__);    
	    } else {
			/*check having read file size */
            if (i_recv_data_bytes != st_test_hdr.ui_dat_len) {
			    file_running("file size is checking fail!\n");	
			    return FILEDATA_DEAL_RET_FAIL; 
		    }
            file_running("recv server file content of cjsont pack data is success!\n");
			file_printf("client_recv_data recv cjosn pack data %d bytes\n", i_recv_data_bytes);
        }

		/*Parse the json data*/
		stp_json = cJSON_Parse(cp_cjson_data_out);
		if (NULL == stp_json) {
            file_error("[%s]cJSON_Parse is error, close server!!\n", __FUNCTION__);  
			 return FILEDATA_DEAL_RET_FAIL;  
		}

		/*get the item that the key name is string_file_data*/
		stp_json_str_filedata = cJSON_GetObjectItem(stp_json, "string_file_data");
		if (cJSON_IsString(stp_json_str_filedata)) {
            if (!strncmp(stp_json_str_filedata->string, "string_file_data", 16)) {
                file_running("[%s]json string_file_data check is success!\n", __FUNCTION__);    
			} else {
                file_running("[%s]json string_file_data check is fail!\n", __FUNCTION__); 
				return FILEDATA_DEAL_RET_FAIL; 
			} 
		}

		/*get the item that the key name is number_file_size*/
		stp_json_num_filesize = cJSON_GetObjectItem(stp_json, "number_file_size");
		if (cJSON_IsNumber(stp_json_num_filesize)) {
            if (!strncmp(stp_json_num_filesize->string, "number_file_size", 16)) {
                file_running("[%s]json number_file_size check is success!\n", __FUNCTION__);    
			} else {
                file_running("[%s]json number_file_size check is fail!\n", __FUNCTION__); 
				return FILEDATA_DEAL_RET_FAIL; 
			} 
		}

		/*open the file*/
		while (1) {
          	i_file_fd = open(c_filename_buf_a, O_WRONLY|O_CREAT|O_EXCL);
          	if (-1 == i_file_fd) {
          		if (errno == EEXIST) {
					i_ret = remove(c_filename_buf_a);
					if (-1 == i_ret) {
                        perror("remove");
						file_error("[%s]remove is error!\n", __FUNCTION__);
						return 0;
					}
                    continue;
          	    } else {
                    perror("open");
            		file_error("[%s]open is fail!\n", __FUNCTION__);
            	    return FILEDATA_DEAL_RET_FAIL;     
				}
          	} else {
                break;
			}   
		}

		/*write data to filename*/		
        sst_write_bytes = datadeal_file_write(i_file_fd, stp_json_str_filedata->valuestring, stp_json_num_filesize->valueint);
        if (-1 == sst_write_bytes) {
            file_error("[%s]datadeal_file_write is fail!\n", __FUNCTION__);
    		return FILEDATA_DEAL_RET_FAIL;     
    	}	
		file_running("file write is success!\n");

		close(i_file_fd);
		if (NULL != cp_cjson_data_out) {
            free(cp_cjson_data_out);
			cp_cjson_data_out = NULL;
		}
		if (NULL != stp_json) {
            cJSON_Delete(stp_json);
			stp_json = NULL;
		}
	}
	if (g_st_test_hdr.en_module == MODULE_TEST_TLV) {
		
        FILE_DATA_ST st_file_data;
		char c_pack_buf_a[BUFSIZE];
	    unsigned int ui_tlv_totol_len = 0; 
		int i_ret = TLV_ENCODE_RET_OK;

		/*read the filename*/
		file_running("Please input file name:\n");
		memset(&st_file_data, 0, sizeof(FILE_DATA_ST));
		i_ret = file_input_string(st_file_data.c_file_name);
		if (i_ret == FILEINPUT_RET_FAIL) {
            file_error("[%s]file_input_string is fail!\n", __FUNCTION__);
			return FILEDATA_DEAL_RET_FAIL;    
		}

		/*pading the FILE_DATA_ST structure*/
	    strcpy(st_file_data.c_file_cmd, "G");
	    st_file_data.c_file_cmd[1] = '\0';
		st_file_data.ui_data_totol_size = 12 + 12 + 4 + st_file_data.ui_file_size; 

		/*tlv encode the st_file_data*/
		memset(c_pack_buf_a, 0, sizeof(c_pack_buf_a));
		i_ret = tlv_encode_file(&st_file_data, c_pack_buf_a, &ui_tlv_totol_len, BUFSIZE);
		if (TLV_ENCODE_RET_FAIL == i_ret) {
            file_error("[%s]tlv_encode_file is fail\n", __FUNCTION__); 
			return FILEDATA_DEAL_RET_FAIL;
		}

	    /*pading the g_st_test_hdr structure*/
		g_st_test_hdr.en_version = VERSION_ONE;
		g_st_test_hdr.ui_dat_len = ui_tlv_totol_len;
		g_st_test_hdr.us_hdr_len = sizeof(g_st_test_hdr);

		/*sned the g_st_test_hdr*/
        i_send_bytes = client_send_data(i_connect_fd, (char *)&g_st_test_hdr, sizeof(g_st_test_hdr));
		if (i_ret == FILE_CLIENT_ERROR) {
            file_error("[%s]client_send_data is fail\n", __FUNCTION__);
			return FILEDATA_DEAL_RET_FAIL;
		}
		file_printf("client_send_data send hdr data %d bytes\n", i_send_bytes);

		/*sned the tlv pack data*/
        i_send_bytes = client_send_data(i_connect_fd, c_pack_buf_a, ui_tlv_totol_len);
		if (i_send_bytes == FILE_CLIENT_ERROR) {
            file_error("[%s]client_send_data is fail\n", __FUNCTION__);
			return FILEDATA_DEAL_RET_FAIL;
		}

		file_printf("client_send_data send tlv pack data %d bytes\n", i_send_bytes);
		file_printf("c_pack_buf_a = %s\n", c_pack_buf_a);
		file_printf("ui_tlv_totol_len size = %d\n", g_st_test_hdr.ui_dat_len);
        for (int i = 0; i < g_st_test_hdr.ui_dat_len; i++) {
            file_printf("%hhX\n", c_pack_buf_a[i]);
		}

		/*recv file content size of hdr*/
		memset(&st_test_hdr, 0, sizeof(st_test_hdr));
		i_recv_data_bytes = client_recv_data(i_connect_fd, &st_test_hdr, sizeof(st_test_hdr)); 
	    if (i_recv_data_bytes == FILE_CLIENT_ERROR) {
            file_error("[%s]client_recv_data is error, close server!!\n", __FUNCTION__);
		    close(i_connect_fd);
		    return FILEDATA_DEAL_RET_FAIL;
	    } else if (i_recv_data_bytes == FILE_CLIENT_RECV_PEER_DOWN) {
            file_running("[%s]SERVER close the connect!\n", __FUNCTION__);    
	    } else {
            file_running("recv server hdr is success!\n");
			file_printf("client_recv_data recv hdr data %d bytes\n", i_recv_data_bytes);
        } 

		/*recv pack file content data from sever*/
		memset(c_pack_buf_a, 0, sizeof(c_pack_buf_a));
        i_recv_data_bytes = client_recv_data(i_connect_fd, (char *)c_pack_buf_a, st_test_hdr.ui_dat_len); 
	    if (i_recv_data_bytes == FILE_CLIENT_ERROR) {
            file_error("[%s]client_recv_data is error, close server!!\n", __FUNCTION__);
		    return FILEDATA_DEAL_RET_FAIL;
	    } else if (i_recv_data_bytes == FILE_CLIENT_RECV_PEER_DOWN) {
            file_running("[%s]SERVER close the connect!\n", __FUNCTION__);    
	    } else {
			/*check having read file size */
            if (i_recv_data_bytes != st_test_hdr.ui_dat_len) {
			    file_running("file size is checking fail!\n");	
			    return FILEDATA_DEAL_RET_FAIL; 
		    }
            file_running("recv server file content of tlv pack data is success!\n");
			file_printf("client_recv_data recv tlv pack data %d bytes\n", i_recv_data_bytes);
        }

        /*decode the tlv pack data*/
		memset(&st_file_data, 0, sizeof(FILE_DATA_ST));
		st_file_data.cp_file_content = (char *)malloc(st_test_hdr.ui_dat_len);
		if (NULL == st_file_data.cp_file_content) {
            perror("malloc");
			file_error("[%s]malloc is fail!\n", __FUNCTION__);
			return FILEDATA_DEAL_RET_FAIL; 
		}
		i_ret = tlv_decode_file(c_pack_buf_a, st_test_hdr.ui_dat_len, &st_file_data);
		if (TLV_DECODE_RET_FAIL == i_ret) {
            file_error("[%s]tlv_decode_file is fail!\n", __FUNCTION__);  
			return FILEDATA_DEAL_RET_FAIL;
		}

		/*open the file*/
		while (1) {
          	i_file_fd = open(st_file_data.c_file_name, O_WRONLY|O_CREAT|O_EXCL);
          	if (-1 == i_file_fd) {
          		if (errno == EEXIST) {
					i_ret = remove(st_file_data.c_file_name);
					if (-1 == i_ret) {
                        perror("remove");
						file_error("[%s]remove is error!\n", __FUNCTION__);
						return 0;
					}
                    continue;
          	    } else {
                    perror("open");
            		file_error("[%s]open is fail!\n", __FUNCTION__);
            	    return FILEDATA_DEAL_RET_FAIL;     
				}
          	} else {
                break;
			}   
		}

		/*write data to filename*/	
        sst_write_bytes = datadeal_file_write(i_file_fd, st_file_data.cp_file_content, st_file_data.ui_file_size);
        if (-1 == sst_write_bytes) {
            file_error("[%s]datadeal_file_write is fail!\n", __FUNCTION__);
    		return FILEDATA_DEAL_RET_FAIL;     
    	}
		file_running("file write is success!\n");

		close(i_file_fd);
		if (NULL != st_file_data.cp_file_content) {
            free(st_file_data.cp_file_content);
			st_file_data.cp_file_content = NULL;
		}
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
    int i_ret = FILE_CLIENT_OK;
    int i_send_bytes = 0;
	char c_filename_buf_a[BUFSIZE];

    if (g_st_test_hdr.en_module == MODULE_TEST_PROTO) {
        FILEDATA st_unpack_buf;
        char *cp_pack_buf = NULL;
		int i_pack_len = 0;
	
        memset(&st_unpack_buf, 0, BUFSIZE);
        st_unpack_buf.p_cmd_buf = (char *)malloc(BUFSIZE);
		st_unpack_buf.p_filename_buf = NULL;
		st_unpack_buf.p_filedata_buf = NULL;
		st_unpack_buf.has_i_file_size = 0;
        strncpy(st_unpack_buf.p_cmd_buf, "L", 1);
		st_unpack_buf.p_cmd_buf[1] = '\0';	

		/*pack the origin data*/
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
		
        cJSON *stp_cjson_root = cJSON_CreateObject();
        char *cp_cjson_data_out = NULL;
	    int i_cjson_data_size = 0;

        /*pack the "L" cmd to the json structure*/
		cJSON_AddStringToObject(stp_cjson_root, "string_cmd", "L");
		
		/*output the json data to char*/
		cp_cjson_data_out = cJSON_PrintUnformatted(stp_cjson_root);
		
		/*judge the data size of bytes*/
		i_cjson_data_size = strlen(cp_cjson_data_out);

	    /*pading the g_st_test_hdr structure*/
		g_st_test_hdr.en_version = VERSION_ONE;
		g_st_test_hdr.ui_dat_len = i_cjson_data_size + 1;
		g_st_test_hdr.us_hdr_len = sizeof(g_st_test_hdr);

		/*sned the g_st_test_hdr*/
        i_send_bytes = client_send_data(i_connect_fd, (char *)&g_st_test_hdr, sizeof(g_st_test_hdr));
		if (i_ret == FILE_CLIENT_ERROR) {
            file_error("[%s]client_send_data is fail\n", __FUNCTION__);
			return FILEDATA_DEAL_RET_FAIL;
		}
		file_printf("client_send_data send hdr data %d bytes\n", i_send_bytes);

		/*sned the cmd cjson pack data*/
        i_send_bytes = client_send_data(i_connect_fd, cp_cjson_data_out, i_cjson_data_size + 1);
		if (i_send_bytes == FILE_CLIENT_ERROR) {
            file_error("[%s]client_send_data is fail\n", __FUNCTION__);
			return FILEDATA_DEAL_RET_FAIL;
		}

		file_printf("client_send_data send cjson pack data %d bytes\n", i_send_bytes);
		file_printf("cp_cjson_data_out = %s\n", cp_cjson_data_out);
		file_printf("cp_cjson_data_out size = %d\n", g_st_test_hdr.ui_dat_len);
        for (int i = 0; i < g_st_test_hdr.ui_dat_len; i++) {
            file_printf("%hhX\n", cp_cjson_data_out[i]);
		}

		cJSON_Delete(stp_cjson_root);
	    free(cp_cjson_data_out);

		/*recv file description from server*/
        file_running("server file as follow:\n");
		while (1) {    
			memset(c_filename_buf_a, 0, sizeof(c_filename_buf_a));
			i_ret= client_recv_data(i_connect_fd, c_filename_buf_a, sizeof(c_filename_buf_a));
            if (i_ret == FILE_CLIENT_ERROR) {
                file_error("[%s]client_recv_data is fail\n", __FUNCTION__); 
				return FILEDATA_DEAL_RET_FAIL;
			}
            
            if (strncmp(c_filename_buf_a, "end", 3) == 0) {
                file_printf("recv filename data complete!\n");
				break;
			}
			
            file_running("%s\n", c_filename_buf_a);		
		}
	}
	if (g_st_test_hdr.en_module == MODULE_TEST_TLV) {
		
	    FILE_DATA_ST st_file_data;
		char c_pack_buf_a[BUFSIZE];
	    unsigned int ui_tlv_totol_len = 0; 
		int i_ret = TLV_ENCODE_RET_OK;

        /*pading the FILE_DATA_ST structure*/
        memset(&st_file_data, 0, sizeof(FILE_DATA_ST));
	    strcpy(st_file_data.c_file_cmd, "L");
	    st_file_data.c_file_cmd[1] = '\0';
		st_file_data.ui_data_totol_size = 12 + 12 + 4 + st_file_data.ui_file_size; 

		/*tlv encode the st_file_data*/
		i_ret = tlv_encode_file(&st_file_data, c_pack_buf_a, &ui_tlv_totol_len, BUFSIZE);
		if (TLV_ENCODE_RET_FAIL == i_ret) {
            file_error("[%s]tlv_encode_file is fail\n", __FUNCTION__); 
			return FILEDATA_DEAL_RET_FAIL;
		}

	    /*pading the g_st_test_hdr structure*/
		g_st_test_hdr.en_version = VERSION_ONE;
		g_st_test_hdr.ui_dat_len = ui_tlv_totol_len;
		g_st_test_hdr.us_hdr_len = sizeof(g_st_test_hdr);

		/*sned the g_st_test_hdr*/
        i_send_bytes = client_send_data(i_connect_fd, (char *)&g_st_test_hdr, sizeof(g_st_test_hdr));
		if (i_ret == FILE_CLIENT_ERROR) {
            file_error("[%s]client_send_data is fail\n", __FUNCTION__);
			return FILEDATA_DEAL_RET_FAIL;
		}
		file_printf("client_send_data send hdr data %d bytes\n", i_send_bytes);

		/*sned the cmd tlv pack data*/
        i_send_bytes = client_send_data(i_connect_fd, c_pack_buf_a, ui_tlv_totol_len);
		if (i_send_bytes == FILE_CLIENT_ERROR) {
            file_error("[%s]client_send_data is fail\n", __FUNCTION__);
			return FILEDATA_DEAL_RET_FAIL;
		}

		file_printf("client_send_data send tlv pack data %d bytes\n", i_send_bytes);
		file_printf("c_pack_buf_a = %s\n", c_pack_buf_a);
		file_printf("ui_tlv_totol_len size = %d\n", g_st_test_hdr.ui_dat_len);
        for (int i = 0; i < g_st_test_hdr.ui_dat_len; i++) {
            file_printf("%hhX\n", c_pack_buf_a[i]);
		}

		/*recv file description from server*/
        file_running("server file as follow:\n");
		while (1) {    
			memset(c_filename_buf_a, 0, sizeof(c_filename_buf_a));
			i_ret= client_recv_data(i_connect_fd, c_filename_buf_a, sizeof(c_filename_buf_a));
            if (i_ret == FILE_CLIENT_ERROR) {
                file_error("[%s]client_recv_data is fail\n", __FUNCTION__); 
				return FILEDATA_DEAL_RET_FAIL;
			}
            
            if (strncmp(c_filename_buf_a, "end", 3) == 0) {
                file_printf("recv filename data complete!\n");
				break;
			}
			
            file_running("%s\n", c_filename_buf_a);		
		}
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
FUNCTION:datadeal_proto_pack
Description:pack data by use protobuf
Arguments:
[stp_unpack_buf][IN]：store origin data
[cp_pack_buf][OUT]：store data be packed
return:success return the size of pack data, fail return FILEDATA_DEAL_RET_FAIL
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
Description:protobuf协议解码
Arguments:
[cp_pack_buf][IN]：待解包的数据
[stp_unpack_buf][OUT]:存放解包之后的数据
return:success return FILEDATA_DEAL_RET_OK and fail return FILEDATA_DEAL_RET_FAIL
************************************************************/
int datadeal_proto_unpack(char *cp_pack_buf, FILEDATA *stp_unpack_buf, int i_size)
{
    assert(NULL != cp_pack_buf);  
	assert(NULL != stp_unpack_buf);

	FILEDATA *st_unpack_data = NULL;

	st_unpack_data = file__data__unpack(NULL, i_size, cp_pack_buf);
    if (NULL == st_unpack_data) {
        file_error("[%s]file__data__unpack is error!\n", __FUNCTION__);
	    return FILEDATA_DEAL_RET_FAIL;    
	}
  
    if (NULL != st_unpack_data->p_cmd_buf) {
        stp_unpack_buf->p_cmd_buf = (char *)malloc(i_size + 1);
        if (NULL == stp_unpack_buf) {
            file_error("[%s]malloc is error!\n", __FUNCTION__);
            return FILEDATA_DEAL_RET_FAIL;    
        }
        strcpy(stp_unpack_buf->p_cmd_buf, st_unpack_data->p_cmd_buf);    
	}

	if (NULL != st_unpack_data->p_filename_buf) {
        stp_unpack_buf->p_filename_buf = (char *)malloc(i_size + 1);
        if (NULL == stp_unpack_buf) {
            file_error("[%s]malloc is error!\n", __FUNCTION__);
            return FILEDATA_DEAL_RET_FAIL;    
        }
        strcpy(stp_unpack_buf->p_filename_buf, st_unpack_data->p_filename_buf);    
	}

	if (NULL != st_unpack_data->p_filedata_buf) {
        stp_unpack_buf->p_filedata_buf = (char *)malloc(i_size + 1);
        if (NULL == stp_unpack_buf) {
            file_error("[%s]malloc is error!\n", __FUNCTION__);
            return FILEDATA_DEAL_RET_FAIL;    
        }
        strcpy(stp_unpack_buf->p_filedata_buf, st_unpack_data->p_filedata_buf);    
	}

	file__data__free_unpacked(st_unpack_data,NULL);

	return FILEDATA_DEAL_RET_OK;
}

