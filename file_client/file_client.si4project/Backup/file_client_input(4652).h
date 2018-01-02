/********************************************************
*   Copyright (C) 2017 All rights reserved.
*
*   Filename:CalcInput.h
*   Author  :叶鸿达
*   Date    :2017-04-30
*   Describe:
*
********************************************************/
#ifndef _CALCINPUT_H
#define _CALCINPUT_H

#define CALCINPUT_RET_OK	            (0)
#define CALCINPUT_RET_FAIL	            (-1)
#define CALCINPUT_RET_INPUT_OUTOFRANGE	(-2)
#define CALCINPUT_RET_INPUT_INVALID		(-3)
#define CALCINPUT_CONTINUE              (1)
#define CALCINPUT_EXIT                  (-4)
#define CALCINPUT_RET_INPUT_SCORE_FAIL  (-5)

int CalcInput_Int(unsigned int * uipInputInt);

int CalcInput_Char(char * pcOperator);

int CalcInput_String(char * pcInputBuf);

int CalcInput_OperatorCheck(char * pcOperator);

int CalcInput_StuInfo(Stu_E * pStuInfo);

#endif
