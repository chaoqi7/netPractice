#ifndef _CELL_THREAD_HPP_
#define _CELL_THREAD_HPP_

#include <thread>
#include <mutex>
#include <functional>
#include "CELLSemaphore.hpp"

class CELLThread
{
private:
	//传入 CELLThread 是为了在判断子线程是否运行时是正确的对应的子线程
	typedef std::function<void(CELLThread*)> EventCallBack;
public:
	//子线程启动
	void Start(EventCallBack onCreate = nullptr, 
		EventCallBack onRun = nullptr, EventCallBack onDestory = nullptr);
	//子线程停止
	void Close();
	//子线程是否在运行
	bool IsRun();
	//在运行函数里面退出，必须调用此函数，避免对信号量的使用导致的死锁
	void Exit();
private:
	//子线程工作函数
	void OnWork();
private:
	//子线程创建的时候需要做的事情
	EventCallBack _onCreate;
	//子线程运行需要做的事情
	EventCallBack _onRun;
	//子线程销毁的时候需要做的事情
	EventCallBack _onDestory;
	//Close 时需要确保 OnRun 先退出的信号量
	CELLSemaphore _semaphore;
	//对是否在运行中变量的锁，确保多个线程同时调用的都线程安全
	std::mutex _mutex;
	//是否在运行中
	bool _bRun = false;
};

inline void CELLThread::Start(
	EventCallBack onCreate, EventCallBack onRun, EventCallBack onDestory)
{
	std::lock_guard<std::mutex> lock(_mutex);
	if (!_bRun)
	{
		//设置运行状态
		_bRun = true;

		//赋值各种回调函数
		if (onCreate) {
			_onCreate = onCreate;
		}

		if (onRun) {
			_onRun = onRun;
		}

		if (onDestory) {
			_onDestory = onDestory;
		}
		//启动工作线程
		std::thread t(std::mem_fn(&CELLThread::OnWork), this);
		t.detach();
	}
}


inline void CELLThread::Close()
{
	std::lock_guard<std::mutex> lock(_mutex);
	if (_bRun)
	{
		_bRun = false;
		//等待 OnWork 函数完全退出
		_semaphore.Wait();
	}
}

inline bool CELLThread::IsRun()
{
	return _bRun;
}

inline void CELLThread::Exit()
{
	std::lock_guard<std::mutex> lock(_mutex);
	if (_bRun)
	{
		_bRun = false;
		//不需要调用信号量的原因是因为确认已经在 OnRun 工作函数里面完全退出了
		//_semaphore.Wait();
	}
}

inline void CELLThread::OnWork()
{
	if (_onCreate) {
		_onCreate(this);
	}

	if (_onRun) {
		_onRun(this);
	}

	if (_onDestory) {
		_onDestory(this);
	}
	//通告 OnWork 完全退出
	_semaphore.Wakeup();
}



#endif // _CELL_THREAD_HPP_