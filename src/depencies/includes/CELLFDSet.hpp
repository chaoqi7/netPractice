#ifndef _CELL_FD_SET_HPP_
#define _CELL_FD_SET_HPP_

#include "CELL.hpp"

class CELLFDSet
{
public:
	CELLFDSet();
	~CELLFDSet();

	void add(SOCKET s);

	void del(SOCKET s);

	void zero();

	bool has(SOCKET s);

	fd_set *fdset();

	void copyfrom(CELLFDSet &src);

	size_t fdsize();

private:
	fd_set *_pfd_set = nullptr;
	size_t _nfdsize = 0;
};

CELLFDSet::CELLFDSet()
{
	int nSocketNum = 10240;
#ifdef _WIN32
	_nfdsize = sizeof(u_int) + sizeof(SOCKET) * nSocketNum;
#else
	_nfdsize = nSocketNum / (8 * sizeof(char));
#endif
	_pfd_set = (fd_set *)new char[_nfdsize];
	memset(_pfd_set, 0, _nfdsize);
}

CELLFDSet::~CELLFDSet()
{
	if (_pfd_set)
	{
		delete[] _pfd_set;
		_pfd_set = nullptr;
	}
}
inline void CELLFDSet::add(SOCKET s)
{
	FD_SET(s, _pfd_set);
}
inline void CELLFDSet::del(SOCKET s)
{
	FD_CLR(s, _pfd_set);
}
inline void CELLFDSet::zero()
{
#ifdef _WIN32
	FD_ZERO(_pfd_set);
#else
	memset(_pfd_set, 0, _nfdsize);
#endif
}
inline bool CELLFDSet::has(SOCKET s)
{
	return FD_ISSET(s, _pfd_set);
}
inline fd_set *CELLFDSet::fdset()
{
	return _pfd_set;
}
inline void CELLFDSet::copyfrom(CELLFDSet &src)
{
	memcpy(_pfd_set, src.fdset(), src.fdsize());
}
inline size_t CELLFDSet::fdsize()
{
	return _nfdsize;
}
#endif // _CELL_FD_SET_HPP_
