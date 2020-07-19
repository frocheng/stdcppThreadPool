#include <iostream>
#include <sstream>
#include <thread>
#include <string>
#include <cstdio>
#include <cstdlib>

#include "Task.h"

/**
 * @Descrition:
 *		template method, object to string.
 * @Param: t of type t, must overload operator<< const method.
 * @Return: std::string type value.
**/
template<typename T>
std::string objectToString(T const& t)
{
	std::ostringstream stream;
	stream << t;
	return stream.str();
}

class TestTask: public Task
{
public:
	TestTask(std::string const& s = ""): m_msg(s) {}
	virtual int run()
	{
		m_msg += objectToString(std::this_thread::get_id());
		std::cout << m_msg << std::endl;
		std::this_thread::sleep_for(std::chrono::seconds(1));
		return 0;
	}
private:
	std::string m_msg{};
};


int io_test(void* arg)
{
	if (arg == nullptr)
	{
		/* maybe this is an error case,
		 * return error code,
		 * but you cannot get this error code.
		 * only you can do is to log this error.
		 */
		 // log_error("...............");
		return -1;
	}
	int * pn = (int*)arg;
	std::string s = "io test ";
	s += std::to_string(*pn);
	std::cout << s << std::endl;
	return 0;
}

/*
 * why need free callback??
 * if the arg were a class instance which is new-ed,
 * "delete void*" op won't call dctor.
 * 
 * free to malloc
 * delete to new
 * delete[] to new[].
***/
void io_test_arg_free(void* arg)
{
	if (arg == nullptr)
	{
		return;
	}
	int * pn = (int*)arg;
	delete pn;
}

int work_test(void* arg)
{
	if (arg == nullptr)
	{
		return -1;
	}
	std::string s = "work test: ";
	s += std::to_string(*((int*)arg));
	std::cout << s << std::endl;
	return 0;
}

void work_test_arg_free(void* arg)
{
	if (arg == nullptr)
	{
		return;
	}
	free(arg);
}

int threadpool_test()
{
	THPHANDLE h_io = THP_NULL_HANDLE;
	THPHANDLE h_worker = THP_NULL_HANDLE;
	int ret = THP_Intialize(&h_io, 2);
	if (ret != 0)
	{
		THP_Unitialize(h_io);
		exit(-1);
	}
	ret = THP_Intialize(&h_worker, 4);
	if (ret != 0)
	{
		THP_Unitialize(h_worker);
		exit(-1);
	}

	for (int i = 0; i < 2; i++)
	{
		THP_PushTask(h_io, new TestTask("IO Thread: "));
	}
	
	int * pm = new int{10};
	THP_PushTask(h_io, io_test, pm, io_test_arg_free);

	for (int i = 0; i < 8; i++)
	{
		THP_PushTask(h_worker, new TestTask("Worker: "));
	}

	int * pn = (int *)malloc(sizeof(int));
	*pn = 50;
	THP_PushTask(h_worker, work_test, pn, work_test_arg_free);

	THP_Unitialize(h_io);

	THP_Unitialize(h_worker);

	return 0;
}

int main()
{
	threadpool_test();
	return 0;
}
