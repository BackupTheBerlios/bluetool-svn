#ifndef __SDP_RECORD_H
#define __SDP_RECORD_H

#include "../common/types.h"
#include "../common/refcnt.h"

#include <list>
#include <string>

namespace Sdp
{
	class DataElement;	
	typedef std::list<DataElement> DataElementList;

	class Bool;
	class String;
	class UUID;

	class Attribute;

	class Record;	//there's no AttributeList, since we've records :)
}

/*
*/

namespace Sdp
{

class DataElement
{
public:
	DataElement();
	DataElement( const DataElement& );
	~DataElement();

	const DataElement& operator = ( const DataElement& );

	operator Bool&();
	operator String&();
	operator UUID&();

protected:
	struct Private;
	Private* pvt;

friend class Record;
};

class Bool : public DataElement
{
public:
	Bool( bool );

	bool to_bool();

	void operator = ( bool b );
};

class String : public DataElement
{
public:
	String( const char* );

	std::string to_string();

	void operator = ( std::string& s );
};

class UUID : public DataElement
{
public:
	UUID( u16 );

	UUID( u32 );

	UUID( u128 );

	std::string to_string();

	bool operator == ( const UUID& );

	void operator = ( u16 u );
	void operator = ( u32 u );
	void operator = ( u128 u );
};

class Attribute : public DataElement
{
public:
	Attribute( u16 id, DataElement& );

	u16 id();

	DataElement& elem();
};

class Record
{
public:
	Record();

	Record( const Record& );

	~Record();

	Attribute operator[]( u16 attr_id );

	u32 handle();

	void add( Attribute& );

	void remove( u16 attr_id );
/*
	languages();

	profile_descriptions();

	server_versions();

	UUID& service_id();

	UUID& group_id();

	U32& record_state();

	U8& service_availability();
	
	U32& service_ttl();

	U32& database_state();
*/
	void add_service_id( UUID& );

	void add_group_id( UUID& );

private:
	struct Private;
	Private* pvt;
};

}//namespace Sdp

#endif//__SDP_RECORD_H
