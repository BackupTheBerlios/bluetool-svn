#ifndef __SDP_ERROR_H
#define __SDP_ERROR_H

#include <exception>
#include <cstring>

#include "../common/types.h"

namespace Sdp
{

class Error : public std::exception
{
public:
	Error();

	Error( u16 code );

	Error( const char* what );

	virtual const char* what() const throw();
private:
	u16 _code;
	const char* _what;
};

}//namespace Sdp

#endif//__SDP_ERROR_H
