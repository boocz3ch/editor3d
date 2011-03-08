#ifndef _EXCEPTION_H_
#define _EXCEPTION_H_

#include <iostream>

class CException: public std::exception {
	std::string msg;
public:
	CException(const std::string &m): msg(m) {}
	~CException() throw() {}
	const char *what() const throw()
	{
		return msg.c_str();
	}
	void show() const
	{
		std::cerr << msg << std::endl;
	}
};


#endif
