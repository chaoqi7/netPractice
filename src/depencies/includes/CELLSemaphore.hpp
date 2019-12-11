#ifndef _CELL_SEMAPHORE_HPP_
#define _CELL_SEMAPHORE_HPP_

#include <thread>
#include <chrono>

class CELLSemaphore
{
public:
	CELLSemaphore();
	~CELLSemaphore();
	void Wait();
	void Wakeup();
private:
	bool _bWaitExit = true;
};

CELLSemaphore::CELLSemaphore()
{
}

CELLSemaphore::~CELLSemaphore()
{
}

inline void CELLSemaphore::Wait()
{
	while (_bWaitExit)
	{
		std::chrono::milliseconds t(1);
		std::this_thread::sleep_for(t);
	}
}

inline void CELLSemaphore::Wakeup()
{
	if (_bWaitExit)
	{
		_bWaitExit = false;
	}
	else {
		printf("CELLSemaphore::Wakeup error.\n");
	}
}

#endif // !_CELL_SEMAPHORE_HPP_
