#ifndef CTHREAD_H_INCLUDED
#define CTHREAD_H_INCLUDED

#include "CCriticalSection.h"
#include <pthread.h>

class CThread
{
private:
    pthread_t hThread;
    int idThread;
    void *lpParam;
	CCriticalSection mtxFree;

	void _zero();
	void _terminate(void* returnvalue=0);
	CThread(const CThread&);
public:
	void* getlpParam() const{return lpParam;}
	bool begin(void* (*routine)(void*),void* lpParam=NULL)
	{
        hThread=0;
        this->lpParam=lpParam;
        return 0==pthread_create(&hThread,NULL,routine,this) && 0!=hThread;
    }
	void* exit(void* returnvalue=0) {return returnvalue;} // don't call exit() in the main thread, call terminate() instead
	CThread() {_zero();}
	CThread(void* (*routine)(void*),void* lpParam=NULL)
	{
        if (!begin(routine,lpParam))
            assert(false);
    }
	~CThread() {}
	pthread_t handle() const{return hThread;}
	int threadid() const{return idThread;}
	bool waitforme()
	{
        pthread_join(hThread,NULL);return true;
    }
};

#endif // CTHREAD_H_INCLUDED
