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

	inline const u8* ptr() const;

private:

	u8 _baddr[6];
};

const u8* BdAddr::ptr() const
{
	return _baddr;
}

#endif//__BD_ADDR_H
