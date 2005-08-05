#ifndef __SDP_ERROR_H
#define __SDP_ERROR_H

#include <exception>

namespace Sdp
{

class Error : public std::exception
{
public:
	virtual const char* what() const throw();
};

}//namespace Sdp

#endif//__SDP_ERROR_H
