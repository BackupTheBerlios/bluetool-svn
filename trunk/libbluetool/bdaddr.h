#ifndef __BD_ADDR_H
#define __BD_ADDR_H

#include "../common/types.h"

#include <string>

struct BdAddr
{

public:

	//BdAddr(const char* str);

	void set(const char* str);

	bool operator == (const BdAddr& ba) const;
	ushort operator [] (uint i) const;

	std::string to_string();

	inline u8* addr();

private:

	u8 _baddr[6];
};

u8* BdAddr::addr()
{
	return _baddr;
}

#endif//__BD_ADDR_H
