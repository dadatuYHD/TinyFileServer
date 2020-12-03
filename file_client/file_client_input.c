/********************************************************
*   Copyright (C) 2017 All rights reserved.
*
*   Filename:file_client_input.c
*   Author  :yhd
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

int File_atoi(unsigned int* puiInputInt, char* pcInputBuf);
int File_intFromBuf(unsigned int* puiInputInt, char* pcInputBuf);





/***************************************************************************
* FUNCTION             :File_inputInt()
* Description          :Processing of arithmetic integer 
*                       string numbers from standard input
* Arguments            :
* Arg1[puiInputInt][In]:A pointer to temporarily store the 
*                       address of the string number 
*                       successfully converted into an integer
* return               :Successfully return FILEINPUT_RET_OK, 
*                       the input string contains illegal 
*                       characters, return FILEINPUT_RET_FAIL_INPUT_INVALID, 
*                       the input string is no longer in a reasonable range, 
*                       return FILEINPUT_RET_FAIL_INPUT_OUTOFRANGE，error 
*                       return FILEINPUT_RET_FAIL
***************************************************************************/
int File_inputInt(unsigned int* puiInputInt)
{
    unsigned int iRet = FILEINPUT_RET_OK;
    char cInputBuf[BUFSIZE];

    /*read number string from standerd input*/
    memset(cInputBuf, 0, sizeof(cInputBuf));
    iRet = File_inputString(cInputBuf);
    if (iRet == FILEINPUT_RET_FAIL)
    {
        File_error("[%s]String Input is fail!\n", __FUNCTION__);
        return FILEINPUT_RET_FAIL;
    }

    /*check number string is valid and convert to the integer number*/
    iRet = File_intFromBuf(puiInputInt, cInputBuf);
    if (iRet == FILEINPUT_RET_FAIL)
    {
        File_error("[%s]Input number is invalid!\n", __FUNCTION__);
        return FILEINPUT_RET_INPUT_INVALID;
    }

    return FILEINPUT_RET_OK;
}


/************************************************************
* FUNCTION            :File_inputString()
* Description         :This function is mainly used to read 
*                      strings from standard input
* Arguments           :
* Arg1[pcInputBuf][In]:Store a pointer from the standard 
*                      input string
* return              :Successfully return FILEINPUT_RET_OK, 
*                      error return FILEINPUT_RET_FAIL
***********************************************************/
int File_inputString(char* pcInputBuf)
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

/***************************************************************
* FUNCTION              :File_intFromBuf()
* Description           :This function converts string numbers 
*                        into integer numbers and checks whether 
*                        the input string is reasonable
* Arguments:
* Arg1[puiInputInt][Out]:Point to the pBuf that stores the string 
*                        number successfully converted into an 
*                        integer
* Arg2[pcInputBuf][In]  :Store the address from the standard 
*                        input string
* return                :Successfully return FILEINPUT_RET_OK, 
*                        error return FILEINPUT_RET_FAIL
***************************************************************/
int File_intFromBuf(unsigned int* puiInputInt, char* pcInputBuf)
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
    iRet = File_atoi(puiInputInt, pcInputBuf);
    if (iRet == FILEINPUT_RET_FAIL)
    {
        File_error("File_atoi convert is fail!\n");
        return FILEINPUT_RET_FAIL;
    }

    return FILEINPUT_RET_OK;
}


/****************************************************************************
* FUNCTION              :File_atoi()
* Description           :Convert string numbers to integer numbers
* return                :Successfully return FILEINPUT_RET_OK, 
*                        the input string is no longer in a reasonable range, 
*                        return FILEINPUT_RET_FAIL_INPUT_OUTOFRANGE，error 
*                        return FILEINPUT_RET_FAIL
****************************************************************************/
int File_atoi(unsigned int * puiInputInt, char * pcInputBuf)
{
    if (NULL == puiInputInt || NULL == pcInputBuf)
    {
        File_error("Input parameter is fail!\n");
        return FILEINPUT_RET_FAIL;
    }

    *puiInputInt  = 0;
    for (int i = 0; i < (strlen(pcInputBuf)); i++)
    {
        if (pcInputBuf[i] < '0' || pcInputBuf[i] > '9')
        {
            return FILEINPUT_RET_INPUT_SCORE_FAIL;  
        }
        else
        {
            *puiInputInt  = *puiInputInt  * 10 + (pcInputBuf[i] - '0'); 
        }   
    }

    return FILEINPUT_RET_OK;
}

/************************************************************
* FUNCTION             :file_input_numcheck()
* Description          :Check whether the converted number 
*                       is within a reasonable range
* Arguments            :
* Arg1[puiInputInt][In]:Store the numbers to be checked
* return               :Return FILEINPUT_RET_OK on success, 
*                       return FILEINPUT_RET_FAIL on failure
***********************************************************/
int File_inputModuleNumCheck(unsigned int uiInputInt)
{
    if (uiInputInt < 1 || uiInputInt > 3)
    {
        return FILEINPUT_RET_FAIL;
    }

    return FILEINPUT_RET_OK;
}

int File_inputCmdMumCheck(unsigned int uiInputInt)
{
    if (uiInputInt < 1 || uiInputInt > 5)
    {
        return FILEINPUT_RET_FAIL;
    }

    return FILEINPUT_RET_OK;
}


/************************************************************
* FUNCTION            :File_inputChar()
* Description         :read of operator characters from 
*                      standard input
* Arguments           :
* Arg1[pcOperator][In]:Point to a char space
* return              :Return FILEINPUT_RET_OK on success, 
*                      return FILEINPUT_RET_FAIL on failure
***********************************************************/
int File_inputChar(char* pcOperator)
{
    if (NULL == pcOperator)
    {
        File_error("input param null\n");
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

