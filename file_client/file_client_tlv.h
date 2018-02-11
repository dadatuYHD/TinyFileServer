#ifndef _FILE_CLIENT_TLV_H_
#define _FILE_CLIENT_TLV_H_


#define TLV_ENCODE_RET_OK    0
#define TLV_ENCODE_RET_FAIL -1
#define TLV_DECODE_RET_OK    1
#define TLV_DECODE_RET_FAIL -2

typedef enum {  
    TLV_FILE_NONE = 0,  
    TLV_FILE_ROOT,         //root node
    TLV_FILE_CMD,          //file server control cmd
    TLV_FILE_NAME,         //file name
    TLV_FILE_CONTENT,      //file content data
    TLV_FILE_SIZE,         //file size
}FILE_TEST_EN;

typedef struct file_data_st {
	char   c_file_cmd[12];
    char   c_file_name[12];
	char * cp_file_content;
	unsigned int ui_file_size;
	unsigned int ui_data_totol_size;
}FILE_DATA_ST, *FILE_DATA_STP;

int tlv_encode_file(FILE_DATA_STP stp_file_data, char *cp_buf, unsigned int *uip_tlv_totol_len, unsigned int ui_buf_len);
int tlv_decode_file(char *cp_buf, unsigned int ui_tlv_totol_len,FILE_DATA_STP stp_file_data);

#endif

