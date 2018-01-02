#ifndef _FILE_DEBUG_H_
#define _FILE_DEBUG_H_

#define FILE_DEBUG
#define FILE_TRACE
#define FILE_ERROR

#ifdef FILE_DEBUG
#define file_printf(fmt, arg...) printf("[file] "fmt,##arg)
#else
#define file_printf(a,...) do{}while(0)
#endif

#ifdef FILE_TRACE
#define file_trace(fmt, arg...) printf("[file][%s][%d][%s]\n",__FUNCTION__,__LINE__, __FILE__)
#else
#define file_trace(a,...) do{}while(0)
#endif

#ifdef FILE_ERROR
#define file_error(fmt, arg...) printf("[file][ERROR]"fmt,##arg)
#else
#define file_error(a,...) do{}while(0)
#endif

#define file_running(fmt, arg...) printf(">> "fmt,##arg)


#endif

