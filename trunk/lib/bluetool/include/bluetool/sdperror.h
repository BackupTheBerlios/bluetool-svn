#ifndef __SDP_ERROR_H
#define __SDP_ERROR_H

#include <cstring>	//strerror()

#include <common/types.h>
#include <common/error.h>

namespace Sdp
{

class Error : public Dbg::Error
{
public:
	Error();

	Error( u16 code );

	Error( const char* what );

private:
	u16 _code;
};

}//namespace Sdp

#endif//__SDP_ERROR_H
