/********************************************************
*   Copyright (C) 2017 All rights reserved.
*
*   Filename:file_client_input.c
*   Author  :叶鸿达
*   Date    :2017-11-24
*   Describe:文件服务器输入相关函数
*
********************************************************/
#include "file_client_input.h"
#include "file_client_debug.h"
#include <stdbool.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define BUFSIZE 128

int file_input_StrNumConvertIntegerNum(unsigned int * uipInputInt, 
                                                           char * pcInputBuf);
int file_input_intfrombuf(unsigned int * uipInputInt, char * pcInputBuf);





/************************************************************
FUNCTION:file_input_int()
Description:该函数主要用来对来自标准输入的运算整数字符串数的处理
Arguments:
Arg1[uipInputInt][In]:暂时存放字符串数字成功转换成整数的地址的指针
return:成功返回FILEINPUT_RET_OK,输入字符串包含非法字符返回FILEINPUT_RET_FAIL_INPUT_INVALID,输入字符串不再合理范围，返回FILEINPUT_RET_FAIL_INPUT_OUTOFRANGE
***********************************************************/
int file_input_int(unsigned int * uipInputInt)
{
    unsigned int iRet = FILEINPUT_RET_OK;
    char cInputBuf_a[BUFSIZE];

    /*read number string from standerd input*/
    memset(cInputBuf_a, 0, sizeof(cInputBuf_a));
    iRet = file_input_string(cInputBuf_a);
    if (iRet == FILEINPUT_RET_FAIL)
    {
        file_error("[%s]String Input is fail!\n", __FUNCTION__);
        return FILEINPUT_RET_FAIL;
    }

    /*check number string is valid and convert to the integer number*/
    iRet = file_input_intfrombuf(uipInputInt, cInputBuf_a);
    if (iRet == FILEINPUT_RET_FAIL)
    {
        file_error("[%s]Input number is invalid!\n", __FUNCTION__);
        return FILEINPUT_RET_INPUT_INVALID;
    }

    return FILEINPUT_RET_OK;
}


/************************************************************
FUNCTION:file_input_string()
Description:该函数主要用来从标准输入读取字符串
Arguments:
Arg1[pcInputBuf][In]:存放来自标准输入字符串的指针
return:成功返回FILEINPUT_RET_OK,错误返回FILEINPUT_RET_FAIL
***********************************************************/
int file_input_string(char * pcInputBuf)
{
    /*read string number to the cc1stNum buffer*/
    memset(pcInputBuf, 0, sizeof(pcInputBuf));
    if (NULL == fgets(pcInputBuf, BUFSIZE, stdin))
    {
        perror("fgets");
        return FILEINPUT_RET_FAIL;
    }

    pcInputBuf[strlen(pcInputBuf)-1] = '\0';

    return FILEINPUT_RET_OK;
}

/************************************************************
FUNCTION:file_input_intfrombuf()
Description:该函数将来字符串数字转换成整型数字,并且检查输入字符串是否合理
Arguments:
Arg1[uipInputInt][Out]:暂时存放字符串数字成功转换成整数的地址的指针
Arg2[pcInputBuf][In]:存放来自标准输入字符串的指针
return:成功返回FILEINPUT_RET_OK,错误返回FILEINPUT_RET_FAIL
***********************************************************/
int file_input_intfrombuf(unsigned int * uipInputInt, char * pcInputBuf)
{
    int iRet = FILEINPUT_RET_OK;
    
    /*judge the input number is valid*/
    for (int i = 0; i < (strlen(pcInputBuf)); i++)
    {
        if (pcInputBuf[i] >= '0' && pcInputBuf[i] <= '9')
        {
            continue;
        }
        else
        {
            return FILEINPUT_RET_FAIL;
        }
    }

    /*string number convert to the integer number*/
    iRet = file_input_StrNumConvertIntegerNum(uipInputInt, pcInputBuf);
    if (iRet == FILEINPUT_RET_FAIL)
    {
        file_error("file_input_StrNumConvertIntegerNum convert is fail!\n");
        return FILEINPUT_RET_FAIL;
    }

    return FILEINPUT_RET_OK;
}


/************************************************************
FUNCTION:file_input_StrNumConvertIntegerNum()
Description:该函数将来字符串数字转换成整型数字
Arguments:
Arg1[uipInputInt][Out]:暂时存放字符串数字成功转换成整数的地址的指针
Arg2[pcInputBuf][In]:存放来自标准输入字符串的指针
return:无返回值
***********************************************************/
int file_input_StrNumConvertIntegerNum(unsigned int * uipInputInt, char * pcInputBuf)
{
    if (NULL == uipInputInt || NULL == pcInputBuf)
    {
        file_error("Input parameter is fail!\n");
        return FILEINPUT_RET_FAIL;
    }

    *uipInputInt  = 0;
    for (int i = 0; i < (strlen(pcInputBuf)); i++)
    {
        if (pcInputBuf[i] < '0' || pcInputBuf[i] > '9')
        {
            return FILEINPUT_RET_INPUT_SCORE_FAIL;  
        }
        else
        {
            *uipInputInt  = *uipInputInt  * 10 + (pcInputBuf[i] - '0'); 
        }   
    }

    return FILEINPUT_RET_OK;
}

/************************************************************
FUNCTION:file_input_numcheck()
Description:检查转换之后的数字是否在合理的范围内
Arguments:
Arg1[uipInputInt][In]:存放待检查的整形数字
return:成功返回FILEINPUT_RET_OK,失败返回FILEINPUT_RET_FAIL
***********************************************************/
int file_input_module_numcheck(unsigned int uipInputInt)
{
    if (uipInputInt < 1 || uipInputInt > 3)
    {
        return FILEINPUT_RET_FAIL;
    }

    return FILEINPUT_RET_OK;
}

int file_input_cmd_numcheck(unsigned int uipInputInt)
{
    if (uipInputInt < 1 || uipInputInt > 5)
    {
        return FILEINPUT_RET_FAIL;
    }

    return FILEINPUT_RET_OK;
}


/************************************************************
FUNCTION:file_input_char()
Description:对来自标准输入的操作符字符的处理
Arguments:
Arg1[pcOperator][In]:存放操作的字符数组的指针
return:成功返回FILEINPUT_RET_OK,失败返回FILEINPUT_RET_FAIL
***********************************************************/
int file_input_char(char * pcOperator)
{
    if (NULL == pcOperator)
    {
        file_error("input param null\n");
        return FILEINPUT_RET_FAIL;
    }

    *pcOperator = getchar();

    char ch = '\0';
    /*discard current line Residual character*/
    while ((ch = getchar()) != '\n' && ch != EOF)
    {
        continue;
    }

    return FILEINPUT_RET_OK;
}

