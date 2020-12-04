#ifndef _FILE_DEBUG_H_
#define _FILE_DEBUG_H_

#define FILE_DEBUG
#define FILE_TRACE
#define FILE_ERROR

#ifdef FILE_DEBUG
#define File_printf(fmt, arg...) printf("[file] "fmt,##arg)
#else
#define File_printf(a,...) do{}while(0)
#endif

#ifdef FILE_TRACE
#define File_trace(fmt, arg...) printf("[file][%s][%d][%s]\n",__FUNCTION__,__LINE__, __FILE__)
#else
#define File_trace(a,...) do{}while(0)
#endif

#ifdef FILE_ERROR
#define File_error(fmt, arg...) printf("[file][ERROR]"fmt,##arg)
#else
#define File_error(a,...) do{}while(0)
#endif

#define File_running(fmt, arg...) printf(">> "fmt,##arg)


#endif

