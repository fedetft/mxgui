//pthread stub for Windows MXGUI emulator
//To be replaced with pthread-win32

#ifndef _PTHREAD_WIN_H_
#define _PTHREAD_WIN_H_

#include <Windows.h>

class pthread_mutex_t
{
private:
    HANDLE hMutex;
public:
    void init(void* ptr);
    pthread_mutex_t();
    ~pthread_mutex_t();
    void lock();
    void unlock();
};

extern "C"
{
    void pthread_mutex_lock(pthread_mutex_t *pMtx);
    void pthread_mutex_unlock(pthread_mutex_t *pMtx);
    void pthread_mutex_init(pthread_mutex_t *pMtx, void* ptr);
}

#endif //_PTHREAD_WIN_H_
