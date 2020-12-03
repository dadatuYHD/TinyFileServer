#ifndef _FILE_CLIENT_STATE_H_
#define _FILE_CLIENT_STATE_H_

/*the major state*/
typedef enum {
    FILESTATE_TCP_INIT = 0,
    FILESTATE_MODULE,
	FILESTATE_CMD,
	FILESTATE_DATA_DEAL,
	FILESTATE_EXIT,
	FILESTATE_MAX,
} FILE_STATE_E;


void fileStateMain(void);


#endif
