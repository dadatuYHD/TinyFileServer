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
    int m_iCurState;
    int (* m_pCurState[])(int iState);
} STATE_CONTROL_ST;

int File_stateTcpInit(int iState);
int File_stateSelModule(int iState);
int File_stateSelCmd(int iState);
int File_stateExit(int iState);
int File_stateDataDeal(int iState);
void File_printCmdHelp(void);


STATE_CONTROL_ST g_stFileState = {
    FILESTATE_TCP_INIT,
    {
        File_stateTcpInit,
        File_stateSelModule,
        File_stateSelCmd,
        File_stateDataDeal,
        File_stateExit,
    },
};

void fileStateMain(void)
{
    while (g_stFileState.m_iCurState != FILESTATE_MAX) {
        g_stFileState.m_iCurState = g_stFileState.m_pCurState[g_stFileState.m_iCurState](g_stFileState.m_iCurState);
    };     
}

int File_stateTcpInit(int iState)
{
    if (iState != FILESTATE_TCP_INIT) {
        File_error("[%s]File_stateTcpInit is fail!\n", __FUNCTION__);
        return FILESTATE_TCP_INIT;
    }

    int iRet = FILE_CLIENT_OK;

    iRet = Client_initSocket();
    if (iRet == FILE_CLIENT_ERROR)
    {
        File_error("[%s]init_socket is failed!\n", __FUNCTION__);
        return FILE_CLIENT_ERROR;   
    }

    iRet = Client_sendReq(SERVER_PORT, SERVER_IP);
    if (iRet == FILE_CLIENT_ERROR)
    {
        File_error("[%s]send_request is failed!\n", __FUNCTION__);
        return FILE_CLIENT_ERROR;   
    }

    File_running("connect server is success!\n");

    return FILESTATE_MODULE;
}


int File_stateSelModule(int iState)
{
    if (iState != FILESTATE_MODULE) {
        File_error("[%s]File_stateSelModule is fail!\n", __FUNCTION__);
        return FILESTATE_MODULE;
    }

    unsigned int ui_module = 0;
    int iRet = FILEINPUT_RET_OK;
    TestHdr_S stTestHdr;
    TestHdr_S* pstTestHdr = NULL;

    /*init the g_st_test_hdr structure*/
    pstTestHdr = DataDeal_getHdrAddress();
    memset(pstTestHdr, 0, sizeof(TestHdr_S));

    /*read the module number*/
    File_running("please select module by number:1.PROTOBUF 2.JSON 3.TLV\n");
    iRet = File_inputInt(&ui_module);
    if (iRet == FILEINPUT_RET_INPUT_INVALID)
    {
        File_error("[%s]Input number is invalid!\n", __FUNCTION__);
        return FILESTATE_MODULE;
    }
    else if (iRet == FILEINPUT_RET_INPUT_OUTOFRANGE)
    {
        File_error("[%s]Input number out of range\n", __FUNCTION__);
        return FILESTATE_MODULE;    
    }

    /*check Input integer number is In a reasonable range*/
    iRet = File_inputModuleNumCheck(ui_module);
    if (iRet == FILEINPUT_RET_FAIL)
    {
        File_error("[%s]Input number out of range\n", __FUNCTION__);
        return FILESTATE_MODULE;
    }

    if (ui_module == MODULE_PROTOBUF)
    {
        memset(&stTestHdr, 0, sizeof(stTestHdr));
        stTestHdr.m_enModule = MODULE_TEST_PROTO;
        iRet = DataDeal_setHdr(&stTestHdr, HDR_FIELD_MODULE);
        if (iRet == FILEDATA_DEAL_RET_FAIL)
        {
            File_error("[%s]DataDeal_setHdr set is fail!\n", __FUNCTION__);
            return FILESTATE_MODULE;
        }
    }
    if (ui_module == MODULE_JSON)
    {
        memset(&stTestHdr, 0, sizeof(stTestHdr));
        stTestHdr.m_enModule = MODULE_TEST_JSON;
        iRet = DataDeal_setHdr(&stTestHdr, HDR_FIELD_MODULE);
        if (iRet == FILEDATA_DEAL_RET_FAIL)
        {
            File_error("[%s]DataDeal_setHdr set is fail!\n", __FUNCTION__);
            return FILESTATE_MODULE;
        }    
    }
    if (ui_module == MODULE_TVL)
    {
        memset(&stTestHdr, 0, sizeof(stTestHdr));
        stTestHdr.m_enModule = MODULE_TEST_TLV;
        iRet = DataDeal_setHdr(&stTestHdr, HDR_FIELD_MODULE);
        if (iRet == FILEDATA_DEAL_RET_FAIL)
        {
            File_error("[%s]DataDeal_setHdr set is fail!\n", __FUNCTION__);
            return FILESTATE_MODULE;
        }  
    }

    return FILESTATE_CMD;
}

void File_printCmdHelp(void)
{
    File_running("help: 显示客户端所有命令和说明\n");
    File_running("list: 显示服务器可下载文件列表\n");
    File_running("get: 下载文件\n");
    File_running("set: 上传文件\n");
    File_running("exit: 退出客户端\n");
}

int File_stateSelCmd(int iState)
{
    if (iState != FILESTATE_CMD)
    {
        File_error("[%s]File_stateSelCmd is fail!\n", __FUNCTION__);
        return FILESTATE_CMD;
    }

    int iRet = FILEINPUT_RET_OK;
    unsigned int ui_cmd;
    TestHdr_S stTestHdr;

    File_running("please select cmd by number:1.set 2.get 3.list 4.help 5.exit \n");
    
    iRet = File_inputInt(&ui_cmd);
    if (iRet == FILEINPUT_RET_INPUT_INVALID)
    {
        File_error("[%s]Input number is invalid!\n", __FUNCTION__);
        return FILESTATE_CMD;
    }
    else if (iRet == FILEINPUT_RET_INPUT_OUTOFRANGE)
    {
        File_error("[%s]Input number out of range\n", __FUNCTION__);
        return FILESTATE_CMD;    
    }

    /*check Input integer number is In a reasonable range*/
    iRet = File_inputCmdMumCheck(ui_cmd);
    if (iRet == FILEINPUT_RET_FAIL)
    {
        File_error("[%s]Input number out of range\n", __FUNCTION__);
        return FILESTATE_CMD;
    }

    if (ui_cmd == CMD_SET)
    {
        memset(&stTestHdr, 0, sizeof(stTestHdr));
        stTestHdr.m_enCmd = CMD_TEST_SET;
        iRet = DataDeal_setHdr(&stTestHdr, HDR_FIELD_CMD);
        if (iRet == FILEDATA_DEAL_RET_FAIL)
        {
            File_error("[%s]DataDeal_setHdr set is fail!\n", __FUNCTION__);
            return FILESTATE_CMD;
        }   
    }
    if (ui_cmd == CMD_GET)
    {
        memset(&stTestHdr, 0, sizeof(stTestHdr));
        stTestHdr.m_enCmd = CMD_TEST_GET;
        iRet = DataDeal_setHdr(&stTestHdr, HDR_FIELD_CMD);
        if (iRet == FILEDATA_DEAL_RET_FAIL)
        {
            File_error("[%s]DataDeal_setHdr set is fail!\n", __FUNCTION__);
            return FILESTATE_CMD;
        }   
    }
    if (ui_cmd == CMD_LIST)
    {
        memset(&stTestHdr, 0, sizeof(stTestHdr));
        stTestHdr.m_enCmd = CMD_TEST_LIST;
        iRet = DataDeal_setHdr(&stTestHdr, HDR_FIELD_CMD);
        if (iRet == FILEDATA_DEAL_RET_FAIL)
        {
            File_error("[%s]DataDeal_setHdr set is fail!\n", __FUNCTION__);
            return FILESTATE_CMD;
        }   
    }
    if (ui_cmd == CMD_HELP)
    {
        File_printCmdHelp(); 
        return FILESTATE_CMD;
    }
    if (ui_cmd == CMD_EXIT)
    {
        memset(&stTestHdr, 0, sizeof(stTestHdr));
        stTestHdr.m_enCmd = CMD_TEST_CLIENT_EXIT;
        iRet = DataDeal_setHdr(&stTestHdr, HDR_FIELD_CMD);
        if (iRet == FILEDATA_DEAL_RET_FAIL) {
            File_error("[%s]DataDeal_setHdr set is fail!\n", __FUNCTION__);
            return FILESTATE_CMD;
        }     
    }
    
    return FILESTATE_DATA_DEAL;    
}

int File_stateDataDeal(int iState)
{
    if (iState != FILESTATE_DATA_DEAL)
    {
        File_error("[%s]file_state_tcp_deal is fail!\n", __FUNCTION__);
        return FILESTATE_CMD;
    }

    int iRet = FILE_CLIENT_OK;

    File_running("start interaction uiData!\n");

    iRet = Client_dataInteraction();
    if (iRet == FILE_CLIENT_ERROR)
    {
        File_error("[%s]data_interaction is failed!\n", __FUNCTION__);
        return FILE_CLIENT_ERROR;   
    } else if (FILESTATE_MAX == iRet) {
        return  FILESTATE_MAX;  
    } else {
            /*************/
    }

    return FILESTATE_MODULE;
}

int File_stateExit(int iState)
{
    if (iState != FILESTATE_EXIT)
    {
        File_error("[%s]File_stateExit is fail!\n", __FUNCTION__);
        return FILESTATE_CMD;
    }
 
    File_running("client is exit!\n");

    return FILESTATE_MAX;
}


