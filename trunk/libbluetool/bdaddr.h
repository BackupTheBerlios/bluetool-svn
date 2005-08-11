#ifndef __BD_ADDR_H
#define __BD_ADDR_H

#include "../common/types.h"

#include <string>

struct BdAddr
{

public:
	BdAddr( );

	BdAddr( const u8* );

	void set( const char* str );

	bool operator == ( const BdAddr& ba ) const;
	ushort operator [] ( uint i ) const;

	const std::string to_string() const;

	inline u8* addr();

private:

	u8 _baddr[6];
};

u8* BdAddr::addr()
{
	return _baddr;
}

#endif//__BD_ADDR_H
