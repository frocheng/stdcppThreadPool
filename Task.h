/*******************************************************
 * @FileName: Task.h
 * @Author: Frodo Cheng
 * @CreatedTime: Jul 17th 2020
 * @Description:
 *		Wrapped thread pool task.
********************************************************/
#ifndef __TASK__H___	// macro name too short, so make it a little complicated.
#define __TASK__H___

#if defined _WIN32 || defined _WIN64
#  ifndef WIN32
#    define WIN32
#  endif
#endif

// For corssplatform
#ifdef WIN32
#define TASKAPI		__stdcall
#else
#define TASKAPI
#endif

#include <iostream>

typedef void* THPHANDLE;

#define	THP_NULL_HANDLE	(nullptr)

class Task
{
public:
	virtual int run() = 0;
	virtual ~Task() = default;
};

typedef int(*CBFct_t)(void*);

typedef void(*CBArgFree_t)(void*);

int TASKAPI THP_Intialize(THPHANDLE * ph, int sz);

int TASKAPI THP_Unitialize(THPHANDLE h);

int TASKAPI THP_PushTask(THPHANDLE h, Task* t);

int TASKAPI THP_PushTask(THPHANDLE h, CBFct_t cb, void* arg = nullptr, CBArgFree_t argfree = nullptr);

#endif	// !__TASK__H___
