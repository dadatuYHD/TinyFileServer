#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "file_client_input.h"
#include "file_client_debug.h"
#include "file_client_state.h"
#include "file_client.h"
#include "file_client_datadeal.h"

#define BUFSIZE     1024
#define SERVER_PORT 8888
#define SERVER_IP   "192.168.1.159"
#define MODULE_PROTOBUF    1
#define MODULE_JSON        2
#define MODULE_TVL         3
#define CMD_SET            1
#define CMD_GET            2
#define CMD_LIST           3
#define CMD_HELP           4
#define CMD_EXIT           5


typedef struct StateControl_t_ {
	int i_curstate;
	int (* p_curstate[])(int i_state);
} STATE_CONTROL_ST;

int file_state_tcp_init(int i_state);
int file_state_sel_module(int i_state);
int file_state_sel_cmd(int i_state);
int file_state_exit(int i_state);
int file_state_data_deal(int i_state);
void print_cmd_help(void);


STATE_CONTROL_ST gst_file_state = {
	FILESTATE_TCP_INIT,
	{
	    file_state_tcp_init,
        file_state_sel_module,
		file_state_sel_cmd,
		file_state_data_deal,
		file_state_exit,
	},
};

void file_state_main(void)
{
    while (gst_file_state.i_curstate != FILESTATE_MAX) {
		gst_file_state.i_curstate = gst_file_state.p_curstate[gst_file_state.i_curstate](gst_file_state.i_curstate);
	};     
}

int file_state_tcp_init(int i_state)
{
    if (i_state != FILESTATE_TCP_INIT) {
        file_error("[%s]file_state_tcp_init is fail!\n", __FUNCTION__);
		return FILESTATE_TCP_INIT;
	}

    int i_ret = FILE_CLIENT_OK;

	i_ret = client_init_socket();
	if (i_ret == FILE_CLIENT_ERROR)
	{
        file_error("[%s]init_socket is failed!\n", __FUNCTION__);
		return FILE_CLIENT_ERROR;   
	}

	i_ret = client_send_request(SERVER_PORT, SERVER_IP);
	if (i_ret == FILE_CLIENT_ERROR)
	{
        file_error("[%s]send_request is failed!\n", __FUNCTION__);
		return FILE_CLIENT_ERROR;   
	}

	file_running("connect server is success!\n");

	return FILESTATE_MODULE;
}


int file_state_sel_module(int i_state)
{
    if (i_state != FILESTATE_MODULE) {
        file_error("[%s]file_state_sel_module is fail!\n", __FUNCTION__);
		return FILESTATE_MODULE;
	}

	unsigned int ui_module = 0;
	int i_ret = FILEINPUT_RET_OK;
	TEST_HDR_T st_test_hdr;

    file_running("please select module by number:1.PROTOBUF 2.JSON 3.TLV\n");
	i_ret = file_input_int(&ui_module);
	if (i_ret == FILEINPUT_RET_INPUT_INVALID)
	{
        file_error("[%s]Input number is invalid!\n", __FUNCTION__);
		return FILESTATE_MODULE;
	}
	else if (i_ret == FILEINPUT_RET_INPUT_OUTOFRANGE)
	{
        file_error("[%s]Input number out of range\n", __FUNCTION__);
		return FILESTATE_MODULE;    
	}

	/*check Input integer number is In a reasonable range*/
	i_ret = file_input_module_numcheck(ui_module);
	if (i_ret == FILEINPUT_RET_FAIL)
	{
		file_error("[%s]Input number out of range\n", __FUNCTION__);
		return FILESTATE_MODULE;
	}

	if (ui_module == MODULE_PROTOBUF)
	{
	    memset(&st_test_hdr, 0, sizeof(st_test_hdr));
		st_test_hdr.en_module = MODULE_TEST_PROTO;
        i_ret = datadeal_set_hdr(&st_test_hdr, HDR_FIELD_MODULE);
		if (i_ret == FILEDATA_DEAL_RET_FAIL)
		{
            file_error("[%s]datadeal_set_hdr set is fail!\n", __FUNCTION__);
			return FILESTATE_MODULE;
		}
	}
	if (ui_module == MODULE_JSON)
    {
        memset(&st_test_hdr, 0, sizeof(st_test_hdr));
		st_test_hdr.en_module = MODULE_TEST_JSON;
        i_ret = datadeal_set_hdr(&st_test_hdr, HDR_FIELD_MODULE);
		if (i_ret == FILEDATA_DEAL_RET_FAIL)
		{
            file_error("[%s]datadeal_set_hdr set is fail!\n", __FUNCTION__);
			return FILESTATE_MODULE;
		}    
	}
	if (ui_module == MODULE_TVL)
	{
        memset(&st_test_hdr, 0, sizeof(st_test_hdr));
		st_test_hdr.en_module = MODULE_TEST_TLV;
        i_ret = datadeal_set_hdr(&st_test_hdr, HDR_FIELD_MODULE);
		if (i_ret == FILEDATA_DEAL_RET_FAIL)
		{
            file_error("[%s]datadeal_set_hdr set is fail!\n", __FUNCTION__);
			return FILESTATE_MODULE;
		}  
	}

	return FILESTATE_CMD;
}

void print_cmd_help(void)
{
    file_running("help: 显示客户端所有命令和说明\n");
	file_running("list: 显示服务器可下载文件列表\n");
	file_running("get <file>: 下载文件\n");
	file_running("set <file>: 上传文件\n");
	file_running("exit: 退出客户端\n");
}

int file_state_sel_cmd(int i_state)
{
    if (i_state != FILESTATE_CMD)
    {
        file_error("[%s]file_state_sel_cmd is fail!\n", __FUNCTION__);
		return FILESTATE_CMD;
	}

	int i_ret = FILEINPUT_RET_OK;
	unsigned int ui_cmd;
	TEST_HDR_T st_test_hdr;

	file_running("please select cmd by number:1.set 2.get 3.list 4.help 5.exit \n");
	
	i_ret = file_input_int(&ui_cmd);
	if (i_ret == FILEINPUT_RET_INPUT_INVALID)
	{
        file_error("[%s]Input number is invalid!\n", __FUNCTION__);
		return FILESTATE_CMD;
	}
	else if (i_ret == FILEINPUT_RET_INPUT_OUTOFRANGE)
	{
        file_error("[%s]Input number out of range\n", __FUNCTION__);
		return FILESTATE_CMD;    
	}

	/*check Input integer number is In a reasonable range*/
	i_ret = file_input_cmd_numcheck(ui_cmd);
	if (i_ret == FILEINPUT_RET_FAIL)
	{
		file_error("[%s]Input number out of range\n", __FUNCTION__);
		return FILESTATE_CMD;
	}

	if (ui_cmd == CMD_SET)
	{
        memset(&st_test_hdr, 0, sizeof(st_test_hdr));
		st_test_hdr.en_cmd = CMD_TEST_SET;
        i_ret = datadeal_set_hdr(&st_test_hdr, HDR_FIELD_CMD);
		if (i_ret == FILEDATA_DEAL_RET_FAIL)
		{
            file_error("[%s]datadeal_set_hdr set is fail!\n", __FUNCTION__);
			return FILESTATE_CMD;
		}   
	}
	if (ui_cmd == CMD_GET)
	{
        memset(&st_test_hdr, 0, sizeof(st_test_hdr));
		st_test_hdr.en_cmd = CMD_TEST_GET;
        i_ret = datadeal_set_hdr(&st_test_hdr, HDR_FIELD_CMD);
		if (i_ret == FILEDATA_DEAL_RET_FAIL)
		{
            file_error("[%s]datadeal_set_hdr set is fail!\n", __FUNCTION__);
			return FILESTATE_CMD;
		}   
	}
	if (ui_cmd == CMD_LIST)
	{
        memset(&st_test_hdr, 0, sizeof(st_test_hdr));
		st_test_hdr.en_cmd = CMD_TEST_LIST;
        i_ret = datadeal_set_hdr(&st_test_hdr, HDR_FIELD_CMD);
		if (i_ret == FILEDATA_DEAL_RET_FAIL)
		{
            file_error("[%s]datadeal_set_hdr set is fail!\n", __FUNCTION__);
			return FILESTATE_CMD;
		}   
	}
	if (ui_cmd == CMD_HELP)
	{
        print_cmd_help(); 
		return FILESTATE_CMD;
	}
	if (ui_cmd == CMD_EXIT)
	{
        memset(&st_test_hdr, 0, sizeof(st_test_hdr));
		st_test_hdr.en_cmd = CMD_TEST_CLIENT_EXIT;
        i_ret = datadeal_set_hdr(&st_test_hdr, HDR_FIELD_CMD);
		if (i_ret == FILEDATA_DEAL_RET_FAIL) {
            file_error("[%s]datadeal_set_hdr set is fail!\n", __FUNCTION__);
			return FILESTATE_CMD;
		}     
	}
	
    return FILESTATE_DATA_DEAL;    
}

int file_state_data_deal(int i_state)
{
    if (i_state != FILESTATE_DATA_DEAL)
    {
        file_error("[%s]file_state_tcp_deal is fail!\n", __FUNCTION__);
		return FILESTATE_CMD;
	}

    int iRet = FILE_CLIENT_OK;

    file_running("start interaction data!\n");

    iRet = client_data_interaction();
	if (iRet == FILE_CLIENT_ERROR)
	{
        file_error("[%s]data_interaction is failed!\n", __FUNCTION__);
		return FILE_CLIENT_ERROR;   
	} else if (FILESTATE_MAX == iRet) {
        return  FILESTATE_MAX;  
	} else {
            /*************/
	}

	return FILESTATE_MODULE;
}

int file_state_exit(int i_state)
{
    if (i_state != FILESTATE_EXIT)
    {
        file_error("[%s]file_state_exit is fail!\n", __FUNCTION__);
		return FILESTATE_CMD;
	}
 
    file_running("client is exit!\n");

    return FILESTATE_MAX;
}


