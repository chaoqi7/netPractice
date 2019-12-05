
#ifndef _CELL_TIME_STAMP_HPP_
#define _CELL_TIME_STAMP_HPP_

#include <chrono>

using namespace std::chrono;

class CELLTimeStamp
{
public:
	CELLTimeStamp();
	~CELLTimeStamp();

	void update();
	//秒
	double getElapseTimeInSeconds();
	//毫秒
	double getElapseTimeInMilliseconds();
	//微秒
	long long getElapseTimeInMicroSeconds();
private:
	time_point<high_resolution_clock> _begin;
};

CELLTimeStamp::CELLTimeStamp()
{
	update();
}

CELLTimeStamp::~CELLTimeStamp()
{
}

inline void CELLTimeStamp::update()
{
	_begin = high_resolution_clock::now();
}

inline double CELLTimeStamp::getElapseTimeInSeconds()
{
	return getElapseTimeInMicroSeconds() * 0.000001;
}

inline double CELLTimeStamp::getElapseTimeInMilliseconds()
{
	return getElapseTimeInMicroSeconds() * 0.001;
}

inline long long CELLTimeStamp::getElapseTimeInMicroSeconds()
{
	return duration_cast<microseconds>(high_resolution_clock::now() - _begin).count();
}

#endif // _CELL_TIME_STAMP_HPP_