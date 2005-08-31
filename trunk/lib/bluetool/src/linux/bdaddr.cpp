#include <bluetool/bdaddr.h>

#include <bluetooth/bluetooth.h>

BdAddr::BdAddr()
{
	memset(_baddr,0,6);
}

BdAddr::BdAddr( const char* addr )
{
	bdaddr_t a;
	str2ba(addr,&a);

	memcpy(_baddr, a.b, 6);
}

BdAddr::BdAddr( const u8* data )
{
	memcpy(_baddr,data,6);
}

bool BdAddr::operator == ( const BdAddr& a ) const
{
	return memcmp(_baddr, a._baddr, 6) == 0;
}

const std::string BdAddr::to_string() const
{
	char straddr[18] = {0};
	ba2str((bdaddr_t*)&(_baddr), straddr);

	return std::string(straddr);
}

ushort BdAddr::operator [] ( uint i ) const
{
	if(i < 6) return _baddr[i];
	return 0;
}
