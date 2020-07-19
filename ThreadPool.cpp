/*******************************************************
 * @FileName: ThreadPool.cpp
 * @Author: Frodo Cheng
 * @CreatedTime: Jul 17th 2020
 * @Description:
 *		Base thread pool tool.
********************************************************/
#include "ThreadPool.h"

int TASKAPI THP_Intialize(THPHANDLE * ph, int sz)
{
	if (ph == nullptr)
	{
		return -1;
	}
	ThreadPool* p = ThreadPool::create();
	if (p == nullptr)
	{
		return -2;
	}
	int ret = p->init(sz);
	if (ret != 0)
	{
		p->destroy();
		return -3;
	}
	*ph = p;
	return 0;
}

int TASKAPI THP_Unitialize(THPHANDLE h)
{
	if (h == THP_NULL_HANDLE)
	{
		return -1;
	}
	ThreadPool* p = (ThreadPool*)h;
	int ret = p->fini();
	if (ret != 0)
	{
		//p->destroy();
		//return ret;
	}
	p->destroy();
	return 0;
}

int TASKAPI THP_PushTask(THPHANDLE h, Task* t)
{
	if (h == THP_NULL_HANDLE)
	{
		return -1;
	}
	ThreadPool* p = (ThreadPool*)h;
	return p->push(t);
}

int TASKAPI THP_PushTask(THPHANDLE h, CBFct_t cb, void* arg, CBArgFree_t argfree)
{
	if (h == THP_NULL_HANDLE)
	{
		return -1;
	}
	ThreadPool* p = (ThreadPool*)h;
	return p->push(new CallbackTask(cb, arg, argfree));
}

CallbackTask::CallbackTask(CallbackFct cb, void* arg, CallbackTaskArgFree cbfree) : m_cb(cb), m_arg(arg), m_cbFree(cbfree)
{
}

int CallbackTask::run()
{
	int ret = 0;
	if (m_cb)
	{
		ret = m_cb(m_arg);
	}
	if (m_cbFree != nullptr && m_arg != nullptr)
	{
		m_cbFree(m_arg);
	}

	return ret;
}

CallbackTask::~CallbackTask()
{
}

ThreadPool * alloc_pool()
{
	return new (std::nothrow) ThreadPool;
}

void free_pool(ThreadPool * pool)
{
	if (pool)
	{
		delete pool;
	}
}

ThreadPool * ThreadPool::create()
{
	ThreadPool * pool = new (std::nothrow) ThreadPool();
	if (pool == nullptr)
	{
		// OOM ERRROR....
		return nullptr;
	}
	return pool;
}

void ThreadPool::destroy()
{
	if (!m_inited.load() && m_exit.load())
	{
		// Some logic here
		delete this;
	}
	else
	{
		delete this;
	}
}


int ThreadPool::init(int sz)
{
	if (sz < 1)
	{
		return -1;
	}

	if (m_inited.load())
	{
		return -2;
	}
	/**
	 * be careful, must set init flag true ASAP.
	**/
	m_inited.store(true);

	m_exit.store(false);
	std::thread* thrd = nullptr;
	for (int i = 0; i < sz; i++)
	{
		thrd = new (std::nothrow) std::thread(task_cb_fct, this);
		if (thrd == nullptr)
		{
			return -3;
		}
		m_pool.push_back(thrd);
	}
	return 0;
}

int ThreadPool::fini()
{
	if (!m_inited.load())
	{
		return -1;
	}
	/**
	 * be careful, must set exit flag true ASAP, so all the work thread can exit ASAP.
	**/
	m_exit.store(true);

	for (auto & thrd : m_pool)
	{
		m_cond.notify_all();
		if (thrd != nullptr && thrd->joinable())
		{
			thrd->join();
			delete thrd;
			thrd = nullptr;
		}
	}

	/**
	 * be careful, must set inited flag false after all the thread handled the task.
	**/
	m_inited.store(false);
	return 0;
}

int ThreadPool::push(Task * p_task)
{
	if (p_task == nullptr)
	{
		return -1;
	}
	// if not inited or if exited, cannot push any task.
	if (!m_inited.load() || m_exit.load())
	{
		return -2;
	}
	std::unique_lock<std::mutex>	lock_guard(m_mtx);
	m_que.push_back(p_task);
	m_cond.notify_one();
	return 0;
}

int ThreadPool::pop(Task ** pp_task)
{
	if (pp_task == nullptr)
	{
		return -1;
	}
	if (!m_inited.load())
	{
		return -2;
	}
	std::unique_lock<std::mutex>	lock_guard(m_mtx);
	if (m_que.empty())
	{
		m_cond.wait(lock_guard);
	}
	if (m_que.empty())
	{
		*pp_task = nullptr;
		return -3;
	}
	*pp_task = m_que.front();
	m_que.pop_front();
	return 0;
}

bool ThreadPool::is_empty()
{
	std::unique_lock<std::mutex>	lock_guard(m_mtx);
	return m_que.empty();
}

void ThreadPool::task_cb_fct(void* arg)
{
	ThreadPool *pool = (ThreadPool*)arg;
	Task* task = nullptr;
	int ret = 0;
	while (true)
	{
		if (pool->m_exit.load() && pool->is_empty())
		{
			break;
		}
		ret = pool->pop(&task);
		if (ret == 0 && task != nullptr)
		{
			ret = task->run();
			if (ret != 0)
			{
				//
			}
			delete task;
			task = nullptr;
		}
	}
}
