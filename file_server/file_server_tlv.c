#include <stdio.h>   
#include <string.h>  
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <assert.h>
#include "file_server_tlv.h"
#include "file_server_debug.h"

int write_block(char ** p_dst, unsigned int ui_write_len, void * p_src, char *cp_end_data)
{
    memcpy(*p_dst, p_src, ui_write_len);
	*p_dst += ui_write_len;

	return (*p_dst <= cp_end_data) ? TLV_ENCODE_RET_OK: TLV_ENCODE_RET_FAIL;
}
int write_int(unsigned int data, char **p_dst, char *cp_end_data)
{
    unsigned int i_net_data = htonl(data);
	
	int i_ret = TLV_ENCODE_RET_OK;
	
	i_ret = write_block(p_dst, sizeof(int), &i_net_data, cp_end_data);
	if (TLV_ENCODE_RET_FAIL == i_ret) {
		file_error("[%s]write_block over lap!\n", __FUNCTION__);
        return TLV_ENCODE_RET_FAIL;
	}

	return TLV_ENCODE_RET_OK;
}

int read_block(void * p_dst, char ** p_src, unsigned int ui_len, char *cp_end_data)
{
    memcpy(p_dst, *p_src, ui_len);
	*p_src += ui_len;

	return (*p_src <= cp_end_data) ? TLV_DECODE_RET_OK : TLV_DECODE_RET_FAIL;
}
int read_int(unsigned     int *data, char **p_src, char * cp_end_data)
{ 	
    int i_ret = TLV_DECODE_RET_OK;

	i_ret = read_block(data, p_src, sizeof(int), cp_end_data);
	if (TLV_DECODE_RET_FAIL == i_ret) {
		file_error("[%s]read_block over lap!\n", __FUNCTION__);
        return TLV_DECODE_RET_FAIL;
	}

	*data = ntohl(*data);

	return TLV_DECODE_RET_OK;
}


/************************************************************
FUNCTION:tlv_encode_file()
Description:use tlv pack data
Arguments:
[stp_file_data][IN]：Point to data to be packaged
[cp_buf][OUT]:Storage of packaged data  
[uip_tlv_totol_len][OUT]:the size of being packaged data  
[ui_buf_len][IN]：the size of cp_buf
return:success return TLV_ENCODE_RET_OK, fail return TLV_ENCODE_RET_FAIL
************************************************************/
int tlv_encode_file(FILE_DATA_STP stp_file_data, char *cp_buf, unsigned int *uip_tlv_totol_len, unsigned int ui_buf_len)
{
    assert(NULL != stp_file_data);
	assert(NULL != cp_buf);
	assert(NULL != uip_tlv_totol_len);

	char * cp_write_data = cp_buf;
	char * cp_end_data = cp_buf + ui_buf_len;

    /*write the root node*/
	write_int(TLV_FILE_ROOT, &cp_write_data, cp_end_data);
	write_int(32 + stp_file_data->ui_data_totol_size, &cp_write_data, cp_end_data);

    /*construct the cmd node*/
    write_int(TLV_FILE_CMD, &cp_write_data, cp_end_data);
	write_int(12, &cp_write_data, cp_end_data);
	write_block(&cp_write_data, sizeof(stp_file_data->c_file_cmd), stp_file_data->c_file_cmd, cp_end_data);

    /*construct the file name node*/
	write_int(TLV_FILE_NAME, &cp_write_data, cp_end_data);
	write_int(12, &cp_write_data, cp_end_data);
	write_block(&cp_write_data, sizeof(stp_file_data->c_file_name), stp_file_data->c_file_name, cp_end_data);

    /*construct the file content node*/
	write_int(TLV_FILE_CONTENT, &cp_write_data, cp_end_data);
	write_int(stp_file_data->ui_file_size, &cp_write_data, cp_end_data);
	write_block(&cp_write_data, stp_file_data->ui_file_size, stp_file_data->cp_file_content, cp_end_data);

    /*construct the file size node*/
	write_int(TLV_FILE_SIZE, &cp_write_data, cp_end_data);
	write_int(4, &cp_write_data, cp_end_data);
	write_int(stp_file_data->ui_file_size, &cp_write_data, cp_end_data);	

	*uip_tlv_totol_len = 32 + stp_file_data->ui_data_totol_size + 8;

	return TLV_ENCODE_RET_OK;
}


/************************************************************
FUNCTION:tlv_decode_file()
Description:unpack the tlv data
Arguments:
[stp_file_data][OUT]：Storage of unpackaged data 
[cp_buf][IN]:Storage of to be unpackaged data  
[uip_tlv_totol_len][IN]:the size of being packaged data  
return:success return TLV_DECODE_RET_OK, fail return TLV_DECODE_RET_FAIL
************************************************************/
int tlv_decode_file(char *cp_buf, unsigned int ui_tlv_totol_len, FILE_DATA_STP stp_file_data)
{
    assert(NULL != stp_file_data);
	assert(NULL != cp_buf);

	char * cp_read_data = cp_buf;
	char * cp_end_data = cp_buf + ui_tlv_totol_len;
	FILE_TEST_EN en_tlv_type = TLV_FILE_NONE;
	unsigned int ui_tlv_len_sum = 0;
	unsigned int ui_tlv_len = 0;

	read_int(&en_tlv_type, &cp_read_data, cp_end_data);
	if (TLV_FILE_ROOT != en_tlv_type) {
		file_error("[%s]read TLV_FILE_ROOT is fail!\n", __FUNCTION__);
        return TLV_DECODE_RET_FAIL;
	}
	
	read_int(&ui_tlv_len_sum, &cp_read_data, cp_end_data);

	while (ui_tlv_len_sum > 0) {
        read_int(&en_tlv_type, &cp_read_data, cp_end_data);
		read_int(&ui_tlv_len, &cp_read_data, cp_end_data);
		switch (en_tlv_type) {
            case TLV_FILE_CMD:			
		        read_block(stp_file_data->c_file_cmd, &cp_read_data,  ui_tlv_len, cp_end_data);
			    ui_tlv_len_sum = ui_tlv_len_sum - (8 + ui_tlv_len);
				break;
			case TLV_FILE_NAME:
		        read_block(stp_file_data->c_file_name, &cp_read_data, ui_tlv_len, cp_end_data);
			    ui_tlv_len_sum = ui_tlv_len_sum - (8 + ui_tlv_len);
				break;
			case TLV_FILE_CONTENT:
		        read_block(stp_file_data->cp_file_content, &cp_read_data, ui_tlv_len, cp_end_data);
			    ui_tlv_len_sum = ui_tlv_len_sum - (8 + ui_tlv_len);
				break;
			case TLV_FILE_SIZE:
				read_int(&stp_file_data->ui_file_size, &cp_read_data, cp_end_data);
			    ui_tlv_len_sum = ui_tlv_len_sum - (8 + ui_tlv_len);
				break;
			default:
				file_error("[%s]tlv decode is fail!\n", __FUNCTION__);
				break;		
		}
	}

	return TLV_DECODE_RET_OK;	
}

