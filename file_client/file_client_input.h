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

#define FILEINPUT_RET_OK	            (0)
#define FILEINPUT_RET_FAIL	            (-1)
#define FILEINPUT_RET_INPUT_OUTOFRANGE	(-2)
#define FILEINPUT_RET_INPUT_INVALID		(-3)
#define FILEINPUT_CONTINUE              (1)
#define FILEINPUT_EXIT                  (-4)
#define FILEINPUT_RET_INPUT_SCORE_FAIL  (-5)

int file_input_int(unsigned int * uipInputInt);

int file_input_char(char * pcOperator);

int file_input_string(char * pcInputBuf);

int file_input_module_numcheck(unsigned int uipInputInt);
int file_input_cmd_numcheck(unsigned int uipInputInt);




#endif
