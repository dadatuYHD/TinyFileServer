/********************************************************
*   Copyright (C) 2017 All rights reserved.
*
*   Filename:CalcInput.c
*   Author  :叶鸿达
*   Date    :2017-04-30
*   Describe:计算器输入处理相关函数
*
********************************************************/
#include "../include/CalcState.h"
#include "../include/CalcInput.h"
#include "../include/calc_complex_debug.h"
#include <stdbool.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#define BUFSIZE 128

int CalcInput_IntFromBuf(unsigned int * uipInputInt, char * pcInputBuf);
int CalcInput_NumCheck(unsigned int uipInputInt);
int CalcInput_StrNumConvertIntegerNum(unsigned int * uipInputInt, char * pcInputBuf);
int CalcInput_Operator(char * pcOperator);
int CalcInput_StringFromBuf(char * pcStuInfoName, char * pcStuInfoChineseSocre,
								       char * pcStuInfoMathSocre, char * pcStuInfoEnglishSocre, 
								       char * pcStuInfo);
int CalcInput_StuInfoCheck(Stu_E pStuInfo);
int CalcInput_StuInfoFromBuf(Stu_E * pStuInfo, char * pcStuInfo_a);

/************************************************************
FUNCTION:CalcInput_StuInfo()
Description:该函数主要用来对来自标准输入的学生信息进行读取并且保存
Arguments:
Arg1[pStuInfo][In]:存放一个学生信息的结构体指针
return:成功返回CALCINPUT_RET_OK,失败返回CALCINPUT_RET_FAIL
************************************************************/
int CalcInput_StuInfo(Stu_E * pStuInfo)
{
	char cStuInfo_a[BUFSIZE];
	int iRet = CALCINPUT_RET_OK;

	/*read student information from stdin*/
	memset(cStuInfo_a, 0, sizeof(cStuInfo_a));
	iRet = CalcInput_String(cStuInfo_a);
	if (iRet == CALCINPUT_RET_FAIL)
	{
		calc_error("CalcInput_String read string is fail!\n");
		return CALCINPUT_RET_FAIL;
	}

	/*学生信息从普通的字符串，提取到学生信息结构体中保存*/
	iRet = CalcInput_StuInfoFromBuf(pStuInfo, cStuInfo_a);
	if (iRet == CALCINPUT_RET_FAIL)
	{
	    calc_error("Error:Input student info & score fail,tryagain!\n");
		return CALCINPUT_RET_FAIL;
	}

	/*check the student information is valid*/
	iRet = CalcInput_StuInfoCheck(*pStuInfo);
	if (iRet == CALCINPUT_RET_FAIL)
	{
		calc_error("Error:Input student info & score fail,tryagain!\n");
		return CALCINPUT_RET_FAIL;
	}

	return CALCINPUT_RET_OK;
}

/************************************************************
FUNCTION:Calculating_Int()
Description:该函数主要用来对来自标准输入的运算整数字符串数的处理
Arguments:
Arg1[uipInputInt][In]:暂时存放字符串数字成功转换成整数的地址的指针
return:成功返回CALCINPUT_RET_OK,输入字符串包含非法字符返回CALCINPUT_RET_FAIL_INPUT_INVALID,输入字符串不再合理范围，返回CALCINPUT_RET_FAIL_INPUT_OUTOFRANGE
***********************************************************/
int CalcInput_Int(unsigned int * uipInputInt)
{
	unsigned int iRet = CALCINPUT_RET_OK;
	char cInputBuf_a[BUFSIZE];

	/*read number string from standerd input*/
	memset(cInputBuf_a, 0, sizeof(cInputBuf_a));
	iRet = CalcInput_String(cInputBuf_a);
	if (iRet == CALCINPUT_RET_FAIL)
	{
		calc_error("[%s]String Input is fail!\n", __FUNCTION__);
		return CALCINPUT_RET_FAIL;
	}

	/*check number string is valid and convert to the integer number*/
	iRet = CalcInput_IntFromBuf(uipInputInt, cInputBuf_a);
	if (iRet == CALCINPUT_RET_FAIL)
	{
		calc_error("[%s]Input number is invalid!\n", __FUNCTION__);
		return CALCINPUT_RET_INPUT_INVALID;
	}

	/*check Input integer number is In a reasonable range*/
	iRet = CalcInput_NumCheck(*uipInputInt);
	if (iRet == CALCINPUT_RET_FAIL)
	{
		calc_error("[%s]Input number out of range\n", __FUNCTION__);
		return CALCINPUT_RET_INPUT_OUTOFRANGE;
	}

	return CALCINPUT_RET_OK;
}

/************************************************************
FUNCTION:Calculating_String()
Description:该函数主要用来从标准输入读取字符串
Arguments:
Arg1[pcInputBuf][In]:存放来自标准输入字符串的指针
return:成功返回CALCINPUT_RET_OK,错误返回CALCINPUT_RET_FAIL
***********************************************************/
int CalcInput_String(char * pcInputBuf)
{
	/*read string number to the cc1stNum buffer*/
	memset(pcInputBuf, 0, sizeof(pcInputBuf));
	if (NULL == fgets(pcInputBuf, BUFSIZE, stdin))
	{
		perror("fgets");
		return CALCINPUT_RET_FAIL;
	}

	pcInputBuf[strlen(pcInputBuf)-1] = '\0';

	return CALCINPUT_RET_OK;
}

/************************************************************
FUNCTION:Calculating_IntFromBuf()
Description:该函数将来字符串数字转换成整型数字,并且检查输入字符串是否合理
Arguments:
Arg1[uipInputInt][Out]:暂时存放字符串数字成功转换成整数的地址的指针
Arg2[pcInputBuf][In]:存放来自标准输入字符串的指针
return:成功返回CALCINPUT_RET_OK,错误返回CALCINPUT_RET_FAIL
***********************************************************/
int CalcInput_IntFromBuf(unsigned int * uipInputInt, char * pcInputBuf)
{
	int iRet = CALCINPUT_RET_OK;
	
	/*judge the input number is valid*/
	for (int i = 0; i < (strlen(pcInputBuf)); i++)
	{
		if (pcInputBuf[i] >= '0' && pcInputBuf[i] <= '9')
		{
			continue;
		}
		else
		{
			return CALCINPUT_RET_FAIL;
		}
	}

	/*string number convert to the integer number*/
	iRet = CalcInput_StrNumConvertIntegerNum(uipInputInt, pcInputBuf);
	if (iRet == CALCINPUT_RET_FAIL)
	{
		calc_error("CalcInput_StrNumConvertIntegerNum convert is fail!\n");
		return CALCINPUT_RET_FAIL;
	}

	return CALCINPUT_RET_OK;
}


/************************************************************
FUNCTION:CalcInput_StrNumConvertIntegerNum()
Description:该函数将来字符串数字转换成整型数字
Arguments:
Arg1[uipInputInt][Out]:暂时存放字符串数字成功转换成整数的地址的指针
Arg2[pcInputBuf][In]:存放来自标准输入字符串的指针
return:无返回值
***********************************************************/
int CalcInput_StrNumConvertIntegerNum(unsigned int * uipInputInt, char * pcInputBuf)
{
	if (NULL == uipInputInt || NULL == pcInputBuf)
	{
		calc_error("Input parameter is fail!\n");
		return CALCINPUT_RET_FAIL;
	}

	*uipInputInt  = 0;
	for (int i = 0; i < (strlen(pcInputBuf)); i++)
	{
		if (pcInputBuf[i] < '0' || pcInputBuf[i] > '9')
		{
		    return CALCINPUT_RET_INPUT_SCORE_FAIL;	
		}
		else
		{
		    *uipInputInt  = *uipInputInt  * 10 + (pcInputBuf[i] - '0');	
		}	
	}

	return CALCINPUT_RET_OK;
}

/************************************************************
FUNCTION:CalcInput_NumCheck()
Description:检查转换之后的数字是否在合理的范围内
Arguments:
Arg1[uipInputInt][In]:存放待检查的整形数字
return:成功返回CALCINPUT_RET_OK,失败返回CALCINPUT_RET_FAIL
***********************************************************/
int CalcInput_NumCheck(unsigned int uipInputInt)
{
	if (uipInputInt < 0 || uipInputInt > 100)
	{
		return CALCINPUT_RET_FAIL;
	}

	return CALCINPUT_RET_OK;
}

/************************************************************
FUNCTION:CalcInput_char()
Description:对来自标准输入的操作符字符的处理
Arguments:
Arg1[pcOperator][In]:存放操作的字符数组的指针
return:成功返回CALCINPUT_RET_OK,失败返回CALCINPUT_RET_FAIL
***********************************************************/
int CalcInput_Char(char * pcOperator)
{
	if (NULL == pcOperator)
	{
		calc_error("input param null\n");
		return CALCINPUT_RET_FAIL;
	}

	*pcOperator = getchar();

	char ch = '\0';
	/*discard current line Residual character*/
	while ((ch = getchar()) != '\n' && ch != EOF)
	{
		continue;
	}

	return CALCINPUT_RET_OK;
}


/************************************************************
FUNCTION:CalcInput_OperatorCheck()
Description:检查操作字符是否合法
Arguments:
Arg1[pcOperator][In]:存放操作字符的数组指针
return:成功返回CALCINPUT_RET_OK,失败返回CALCINPUT_RET_FAIL
***********************************************************/
int CalcInput_OperatorCheck(char * pcOperator)
{
	/*judge the input Operator is valid*/
	if (*pcOperator != '+' && *pcOperator != '-'
		 && *pcOperator != '*' && *pcOperator != '/')
	{
		calc_error("[%s]Operator Input is invalid!\n", __FUNCTION__);
		return CALCINPUT_RET_FAIL;
	}

	return CALCINPUT_RET_OK;
}

/************************************************************
FUNCTION:CalcInput_StringFromBuf()
Description:从来自标准输入的学生字符串信息，分类提取到相应的buf中
Arguments:
Arg1[pcStuInfoName][Out]:指向一个缓冲区，用来存放学生姓名
Arg1[pcStuInfoChineseSocre][Out]:指向一个缓冲区，用来存放学生语文成绩
Arg1[pcStuInfoMathSocre][Out]:指向一个缓冲区，用来存放学生数学成绩
Arg1[pcStuInfoEnglishSocre][Out]:指向一个缓冲区，用来存放学生英语成绩
Arg1[pcStuInfo][In]:指向一个缓冲区，用来提取学生的相应信息
return:成功返回CALCINPUT_RET_OK,失败返回CALCINPUT_RET_FAIL
***********************************************************/
int CalcInput_StringFromBuf(char * pcStuInfoName, char * pcStuInfoChineseSocre,
								       char * pcStuInfoMathSocre, char * pcStuInfoEnglishSocre, 
								       char * pcStuInfo)
{
	unsigned int uiStrCount = 0;
	
	if (pcStuInfoName == NULL || pcStuInfoChineseSocre == NULL || pcStuInfoMathSocre == NULL
			|| pcStuInfoEnglishSocre == NULL || pcStuInfo == NULL)
	{
		calc_error("Input parameter is fail!\n");
		return CALCINPUT_RET_FAIL;
	}

	while (uiStrCount < strlen(pcStuInfo))
	{
		while (1)
		{
			if (*pcStuInfo != ' '  && *pcStuInfo != '\0')
			{
				*pcStuInfoName = *pcStuInfo;
				pcStuInfoName++;
				pcStuInfo++;
				uiStrCount++;
			}
			else
			{
			    switch (*pcStuInfo)
			    {
					case ' ':
					    pcStuInfo++;
					    uiStrCount++;
					    break;	
					case '\0':
						return CALCINPUT_RET_FAIL;
					default:
					    break;			
				}
				break;
			}	
		}
		
		while (1)
		{
			if (*pcStuInfo != ' '  && *pcStuInfo != '\0')
			{
				*pcStuInfoChineseSocre = *pcStuInfo;
				pcStuInfoChineseSocre++;
				pcStuInfo++;
				uiStrCount++;
			}
			else
			{
			    switch (*pcStuInfo)
			    {
					case ' ':
					    pcStuInfo++;
					    uiStrCount++;
					    break;	
					case '\0':
						return CALCINPUT_RET_FAIL;
					default:
					    break;			
				}
				break;
			}	
		}
	
		while (1)
		{
			if (*pcStuInfo != ' '  && *pcStuInfo != '\0')
			{
				*pcStuInfoMathSocre = *pcStuInfo;
				pcStuInfoMathSocre++;
				pcStuInfo++;
				uiStrCount++;
			}
			else
			{
			    switch (*pcStuInfo)
			    {
					case ' ':
					    pcStuInfo++;
					    uiStrCount++;
					    break;	
					case '\0':
						return CALCINPUT_RET_FAIL;
					default:
					    break;			
				}
				break;
			}	
		}

		while (1)
		{
			if (*pcStuInfo != ' '  && *pcStuInfo != '\0')
			{
				*pcStuInfoEnglishSocre = *pcStuInfo;
				pcStuInfoEnglishSocre++;
				pcStuInfo++;
				uiStrCount++;
			}
			else
			{
			    switch (*pcStuInfo)
			    {
					case ' ':
						return CALCINPUT_RET_FAIL;	
					case '\0':
						break;
					default:
					    break;			
				}
				break;
			}	
		}
	}

	return CALCINPUT_RET_OK;
}
									   
/************************************************************
FUNCTION:CalcInput_StuInfoCheck()
Description:检查学生信息是否合法
Arguments:无
return:成功返回CALCINPUT_RET_OK,失败返回CALCINPUT_RET_FAIL
***********************************************************/
int CalcInput_StuInfoCheck(Stu_E pStuInfo)
{
	for (int i = 0; pStuInfo.gcStuName[i] != '\0'; i++)
	{
		if ((pStuInfo.gcStuName[i] >= 'a' && pStuInfo.gcStuName[i] <= 'z') 
			|| (pStuInfo.gcStuName[i] >= 'A' && pStuInfo.gcStuName[i] <= 'Z'))
		{
		    continue;
		}
		else
		{
			return CALCINPUT_RET_FAIL;
		}
	}

	if ((pStuInfo.uiChineseScore < 0 || pStuInfo.uiChineseScore > 100) 
		|| (pStuInfo.uiMathScore < 0 || pStuInfo.uiMathScore > 100)
		|| (pStuInfo.uiEnglishScore < 0 || pStuInfo.uiEnglishScore > 100))
	{
	    return CALCINPUT_RET_FAIL;
	}
	
	return CALCINPUT_RET_OK;
}

/************************************************************
FUNCTION:CalcInput_StuInfoFromBuf()
Description:把stdin标准输入读取的字符串信息提取到学生信息结构体当中
Arguments:无
return:成功返回CALCINPUT_RET_OK,失败返回CALCINPUT_RET_FAIL
***********************************************************/
int CalcInput_StuInfoFromBuf(Stu_E * pStuInfo, char * pcStuInfo_a)
{
    char cStuInfoName_a[BUFSIZE];
	char cStuInfoChineseSocre_a[BUFSIZE];
	char cStuInfoMathSocre_a[BUFSIZE];
	char cStuInfoEnglishSocre_a[BUFSIZE];
	int iRet = CALCINPUT_RET_OK;

	/*dump student information from cStuInfo_a buf*/
	memset(cStuInfoName_a, 0, sizeof(cStuInfoName_a));
	memset(cStuInfoChineseSocre_a, 0, sizeof(cStuInfoChineseSocre_a));
	memset(cStuInfoMathSocre_a, 0, sizeof(cStuInfoMathSocre_a));
	memset(cStuInfoEnglishSocre_a, 0, sizeof(cStuInfoEnglishSocre_a));
	iRet = CalcInput_StringFromBuf(cStuInfoName_a, cStuInfoChineseSocre_a,
								   cStuInfoMathSocre_a,cStuInfoEnglishSocre_a, pcStuInfo_a);
	if (iRet == CALCINPUT_RET_FAIL)
	{
		calc_error("CalcInput_StringFromBuf dump is fail!\n");
		return CALCINPUT_RET_FAIL;
	}

	/*填充学生信息结构体，把学生字符串表示的分数转换成整数表示，并保存到学生结构体当中*/
	strncpy(pStuInfo->gcStuName, cStuInfoName_a, sizeof(cStuInfoName_a));
#if 1
	iRet = CalcInput_StrNumConvertIntegerNum(&(pStuInfo->uiChineseScore), cStuInfoChineseSocre_a);
	if (iRet == CALCINPUT_RET_INPUT_SCORE_FAIL)
	{
		calc_error("Error:Input student info & score fail,tryagain!\n");
		return CALCINPUT_RET_FAIL;
	}
	iRet = CalcInput_StrNumConvertIntegerNum(&(pStuInfo->uiMathScore), cStuInfoMathSocre_a);
	if (iRet == CALCINPUT_RET_INPUT_SCORE_FAIL)
	{
		calc_error("Error:Input student info & score fail,tryagain!\n");
		return CALCINPUT_RET_FAIL;
	}
	iRet = CalcInput_StrNumConvertIntegerNum(&(pStuInfo->uiEnglishScore), cStuInfoEnglishSocre_a);
	if (iRet == CALCINPUT_RET_INPUT_SCORE_FAIL)
	{
		calc_error("Error:Input student info & score fail,tryagain!\n");
		return CALCINPUT_RET_FAIL;
	}
#endif
	/*calc_printf("pStuInfo->gcStuName = %s, pStuInfo->uiChineseScore = %d, pStuInfo->uiMathScore = %d, pStuInfo->uiEnglishScore = %d\n", 
						pStuInfo->gcStuName, pStuInfo->uiChineseScore, 
						pStuInfo->uiMathScore, pStuInfo->uiEnglishScore);*/	
}

