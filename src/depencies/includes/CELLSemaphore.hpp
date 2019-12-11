#ifndef _CELL_SEMAPHORE_HPP_
#define _CELL_SEMAPHORE_HPP_

#include <thread>
#include <mutex>
#include <condition_variable>

class CELLSemaphore
{
public:
	CELLSemaphore();
	~CELLSemaphore();
	//阻塞
	void Wait();
	//唤醒
	void Wakeup();
private:
	std::condition_variable _cv;
	std::mutex _mutex;
	int _waitNum = 0;
	int _wakeupNum = 0;
};

CELLSemaphore::CELLSemaphore()
{
}

CELLSemaphore::~CELLSemaphore()
{
}

inline void CELLSemaphore::Wait()
{
	std::unique_lock<std::mutex> lock(_mutex);
	if (++_waitNum > 0)
	{
		_cv.wait(lock, [this]()->bool {
			return _wakeupNum > 0;
		});
		--_wakeupNum;
	}

}

inline void CELLSemaphore::Wakeup()
{
	std::lock_guard<std::mutex> lock(_mutex);
	if (--_waitNum <= 0)
	{
		++_wakeupNum;
		_cv.notify_one();
	}
}

#endif // !_CELL_SEMAPHORE_HPP_
