#include <stdlib.h>
#include <windows.h>

struct CThreadPool;

typedef struct CThreadParam
{
	struct CThreadPool* ThreadPool; 
	void* State;
} CThreadParam;

typedef struct CThreadPool
{
	unsigned long NumThreads;
	HANDLE* ThreadHandle;
	volatile long ActiveThreads;
	CThreadParam* ThreadParam;
} CThreadPool;

static DWORD ThreadFunction(void* param)
{
	CThreadParam* threadParam = param;
	CThreadPool* threadPool = threadParam->ThreadPool;
	InterlockedExchangeAdd(&threadPool->ActiveThreads, 1);
	Sleep(2000);
	return 0;
}

void ThreadPool_Create(CThreadPool* threadPool)
{
	SYSTEM_INFO sysInfo;
	GetSystemInfo(&sysInfo);
	threadPool->ActiveThreads = 0;
	threadPool->NumThreads = sysInfo.dwNumberOfProcessors;
	threadPool->ThreadHandle = calloc(threadPool->NumThreads, sizeof(HANDLE));
	threadPool->ThreadParam = calloc(threadPool->NumThreads, sizeof(CThreadParam));
}

unsigned long ThreadPool_Run(CThreadPool* threadPool, DWORD(*threadProc)(void*), DWORD stateSize, void* statePtr)
{
	unsigned long runningThreads = 0;
	for (DWORD i = 0; i < threadPool->NumThreads; i++)
	{
		threadPool->ThreadParam[i].ThreadPool = threadPool;
		threadPool->ThreadParam[i].State = (statePtr) ? ((char*)statePtr) + i * stateSize : NULL;
		threadPool->ThreadHandle[i] = CreateThread(NULL, 0, threadProc, &threadPool->ThreadParam[i], 0, NULL);
		if (!threadPool->ThreadHandle[i])
		{
			runningThreads = 0;
			break;
		}
		++runningThreads;
	}
	long expected = (long)threadPool->NumThreads;
	while (InterlockedCompareExchange(&threadPool->ActiveThreads, expected, expected) != expected)
	{
		Sleep(0);
	}
	return runningThreads;
}

void ThreadPool_Wait(CThreadPool* threadPool)
{
	WaitForMultipleObjects(threadPool->NumThreads, threadPool->ThreadHandle, TRUE, INFINITE);
}

void ThreadPool_Destroy(CThreadPool* threadPool)
{
	free(threadPool->ThreadHandle);
	free(threadPool->ThreadParam);
}

/*int AppMain(int argc, char** argv)
{
	CThreadPool threadPool;
	ThreadPool_Create(&threadPool);
	if (ThreadPool_Run(&threadPool, &ThreadFunction, 0, NULL))
	{
		ThreadPool_Wait(&threadPool);
	}
	ThreadPool_Destroy(&threadPool);
	return 0;
}*/
