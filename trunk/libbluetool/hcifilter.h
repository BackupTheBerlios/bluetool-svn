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

	Filter()
	{
		clear();
	}

	void clear()
	{
		_filter.type_mask = 0;
		_filter.event_mask = 0;
		_filter.opcode = 0;
	}

	void set_type( int bit )
	{
		bit_set(_filter.type_mask, 32, bit);
	}
	void clear_type( int bit )
	{
		bit_clear(_filter.type_mask, 32, bit);
	}
	bool test_type( int bit )
	{
		return bit_test(_filter.type_mask, 32, bit);
	}

	void set_event( int bit )
	{
		bit_set(_filter.event_mask, 64, bit);
	}
	void clear_event( int bit )
	{
		bit_clear(_filter.event_mask, 64, bit);
	}
	bool test_event( int bit )
	{
		return bit_test(_filter.event_mask, 64, bit);
	}

	u16 opcode()
	{
		return _filter.opcode;
	}
	void set_opcode( u16 opcode )
	{
		_filter.opcode = opcode;
	}
	void clear_opcode()
	{
		_filter.opcode = 0;
	}
	bool test_opcode( u16 opcode )
	{
		return opcode == _filter.opcode;
	}

private:

	struct
	{
		u32	type_mask;
		u64	event_mask;
		u16	opcode;
	} _filter;

friend class Socket;
};

}//namespace Hci

#endif//__HCI_FILTER_H
