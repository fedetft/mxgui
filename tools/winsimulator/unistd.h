#ifndef _UNISTD_H_
#define _UNISTD_H_

#include <Windows.h>
#define usleep(x) Sleep(x/1000)
#endif // _UNISTD_H_
