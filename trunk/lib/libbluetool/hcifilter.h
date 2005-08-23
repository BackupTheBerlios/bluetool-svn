#ifndef __HCI_FILTER_H
#define __HCI_FILTER_H

#include "../common/types.h"

namespace Hci
{
	class Filter;
}

#include "hcisocket.h"

namespace Hci
{

class Filter
{

public:

	Filter();

	Filter( const Filter& );

	~Filter();

	void clear();

	void set_type( int bit );

	void clear_type( int bit );

	bool test_type( int bit );

	void set_event( int bit );

	void clear_event( int bit );

	bool test_event( int bit );

	u16 opcode();

	void set_opcode( u16 opcode );

	void clear_opcode();

	bool test_opcode( u16 opcode );

private:
	
	struct Private;
	Private* pvt;

friend class Socket;
};

}//namespace Hci

#endif//__HCI_FILTER_H
