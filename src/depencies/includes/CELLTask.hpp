#ifndef _CELL_TASK_H_
#define _CELL_TASK_H_

#include <list>
#include <thread>
#include <mutex>
#include <functional>
//1:1 一个接收线程，对应一个发送线程
//生产者-消费者模式调度
class CellTaskServer
{
	typedef std::function<void()> CellTask;
public:
	CellTaskServer();
	~CellTaskServer();
	void AddTask(CellTask task);
	void Start(int id);
	void Close();
private:
	void OnRun();
private:
	//正在处理的任务列表
	std::list<CellTask> _tasks;
	//待处理任务列表
	std::list<CellTask> _tasksBuf;
	std::mutex _mutex;
	//仅仅为了测试
	int _id = -1;
	bool _bRun = false;
	bool _bWaitExit = true;
};

CellTaskServer::CellTaskServer()
{
}

CellTaskServer::~CellTaskServer()
{
}
inline void CellTaskServer::AddTask(CellTask task)
{
	std::lock_guard<std::mutex> lock(_mutex);
	_tasksBuf.push_back(task);
}
inline void CellTaskServer::Start(int id)
{
	_id = id;
	_bRun = true;
	std::thread t(std::mem_fn(&CellTaskServer::OnRun), this);
	t.detach();
}
inline void CellTaskServer::Close()
{
	printf("CellTaskServer %d::Close start\n", _id);
	if (_bRun)
	{
		_bRun = false;
		while (_bWaitExit)
		{
			std::chrono::milliseconds t(1);
			std::this_thread::sleep_for(t);
		}
	}
	printf("CellTaskServer %d::Close end\n", _id);
}
inline void CellTaskServer::OnRun()
{
	printf("CellTaskServer %d::OnRun start\n", _id);
	while (_bRun)
	{
		if (!_tasksBuf.empty())
		{
			//把待处理列表的任务加入正在处理的任务列表
			std::lock_guard<std::mutex> lock(_mutex);
			for (auto pTask : _tasksBuf)
			{
				_tasks.push_back(pTask);
			}
			_tasksBuf.clear();
		}

		//如果正在处理列表没有任务
		if (_tasks.empty())
		{
			std::chrono::milliseconds t(1);
			std::this_thread::sleep_for(t);
			continue;
		}

		//遍历需要处理的任务列表
		for (auto pTask : _tasks)
		{
			//做任务
			pTask();
		}
		//清空需要处理的任务列表
		_tasks.clear();
	}
	_bWaitExit = false;
	printf("CellTaskServer %d::OnRun end\n", _id);
}
#endif // _CELL_TASK_H_
