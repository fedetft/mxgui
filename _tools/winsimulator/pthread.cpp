#include "pthread.h"


void pthread_mutex_t::init(void* ptr)
{
    hMutex = CreateMutex(NULL, FALSE, NULL);
}

pthread_mutex_t::pthread_mutex_t()
    : hMutex(NULL)
{
}

pthread_mutex_t::~pthread_mutex_t()
{
    if (NULL != hMutex)
    {
        CloseHandle(hMutex);
    }
}

void pthread_mutex_t::lock()
{
    WaitForSingleObject(hMutex, INFINITE);
}

void pthread_mutex_t::unlock()
{
    ReleaseMutex(hMutex);
}

void pthread_mutex_lock(pthread_mutex_t *pMtx)
{
    if (0 != pMtx)
    {
        pMtx->lock();
    }
}

void pthread_mutex_unlock(pthread_mutex_t *pMtx)
{
    if (0 != pMtx)
    {
        pMtx->unlock();
    }
}

void pthread_mutex_init(pthread_mutex_t *pMtx, void* ptr)
{
    pMtx->init(ptr);
}
