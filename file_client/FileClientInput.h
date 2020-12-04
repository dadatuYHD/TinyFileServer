/********************************************************
*   Copyright (C) 2017 All rights reserved.
*
*   Filename:CalcInput.h
*   Author  :yhd
*   Date    :2017-04-30
*   Describe:
*
********************************************************/
#ifndef _CALCINPUT_H
#define _CALCINPUT_H

#define FILEINPUT_RET_OK	            (0)
#define FILEINPUT_RET_FAIL	            (-1)
#define FILEINPUT_RET_INPUT_OUTOFRANGE	(-2)
#define FILEINPUT_RET_INPUT_INVALID		(-3)
#define FILEINPUT_CONTINUE              (1)
#define FILEINPUT_EXIT                  (-4)
#define FILEINPUT_RET_INPUT_SCORE_FAIL  (-5)

int File_inputInt(unsigned int* puiInputInt);

int File_inputChar(char* pcOperator);

int File_inputString(char* pcInputBuf);

int File_inputModuleNumCheck(unsigned int uiInputInt);
int File_inputCmdMumCheck(unsigned int uiInputInt);




#endif
