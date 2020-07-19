/*******************************************************
 * @FileName: ThreadPool.h
 * @Author: Frodo Cheng
 * @CreatedTime: Jul 17th 2020
 * @Description:
 *		Base thread pool tool.
********************************************************/
#ifndef __THREAD_POOL_H__
#define __THREAD_POOL_H__

#include "Task.h"

#include <thread>
#include <mutex>
#include <condition_variable>
#include <deque>
#include <atomic>

class CallbackTask : public Task
{
public:
	typedef int(*CallbackFct)(void*);
	typedef void(*CallbackTaskArgFree)(void*);
	CallbackTask(CallbackFct cb, void* arg, CallbackTaskArgFree cbfree = nullptr);
	virtual int run() override;
	~CallbackTask();
private:
	CallbackFct m_cb{ nullptr };
	CallbackTaskArgFree m_cbFree{ nullptr };
	void * m_arg{ nullptr };
};

class ThreadPool
{
public:
	/* ���캯�� �ѱ�˽�л�������ֻ��ͨ������������ʵ�ֶ��ж�̬����Ļ�ȡ�� delete this �������Դ */
	/* ��Ԫ����ʵ�� ���� */
	friend ThreadPool* alloc_pool();
	friend void free_pool(ThreadPool* pool);
	/* static public ��Ա��ʽ ʵ�ִ��� */
	static ThreadPool* create();
	void destroy();

	int init(int sz);
	int fini();
	int push(Task* p_task);

protected:
	int pop(Task** pp_task);
	bool is_empty();

private:
	std::atomic<bool>			m_inited{false};
	std::atomic<bool>			m_exit{false};
	std::deque<std::thread*>	m_pool{};
	std::deque<Task*>			m_que{};
	std::mutex					m_mtx{};
	std::condition_variable		m_cond{};

private:
	static void task_cb_fct(void* arg);

	// you must use create static method to get a threadpool instance
	ThreadPool() = default;
	~ThreadPool() = default;

	ThreadPool(ThreadPool const&) = delete;
	ThreadPool& operator=(ThreadPool const&) = delete;
};


#endif // !__THREAD_POOL_H__

