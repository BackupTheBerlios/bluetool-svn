#ifndef __HCI_EXCEPTION_H
#define __HCI_EXCEPTION_H

#include <cstdlib>
#include <common/types.h>
#include <common/error.h>

namespace Hci
{

class Error : public Dbg::Error
{

public:

	Error( u16 code );

	Error( const char* error = NULL );
};

}//namespace Hci

#endif//__HCI_EXCEPTION_H
