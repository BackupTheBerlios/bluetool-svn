#ifndef __SDP_RECORD_H
#define __SDP_RECORD_H

#include "../common/types.h"
#include "../common/refcnt.h"

#include <sigc++/sigc++.h>
#include <list>
#include <string>

namespace Sdp
{
	class DataElement;	
	typedef std::list<DataElement> DataElementSeq;

	class Bool;
	class U32;
	//typedef std::list<U32> U32Seq;
	class String;
	class UUID;
	//typedef std::list<UUID> UUIDSeq;

	class Attribute;

	class Record;
	typedef std::list<Record> RecordList;

	typedef sigc::signal<void, u16, const RecordList&> SdpEvent;
}

#include "sdpsession.h"

/*
*/

namespace Sdp
{

class DataElement
{
protected:
	DataElement();
public:
	DataElement( const DataElement& );
	virtual ~DataElement();

	const DataElement& operator = ( const DataElement& );

/*	operator Bool&();
	operator U32&();
	operator String&();
	operator UUID&();
*/
	struct Private;

protected:
	Private* pvt;

friend class Session;
friend class Record;
};

class Bool : public DataElement
{
public:
	Bool( bool );

	bool to_bool();

	void operator = ( bool b );
};

class U32 : public DataElement
{
public:
	U32( u32 );

	u32 to_u32();

	void operator = ( u32 u );
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

	class iterator;

	iterator begin();
	iterator end();

	struct Private;

	Record( Private* );

private:
	Private* pvt;
};


class Record::iterator
{
public:
	iterator(Record::Private* );

	const iterator& operator ++();

	bool operator == (const iterator& i);
	bool operator != (const iterator& i)
	{
		return !((*this)==i);
	}

	Attribute operator *();

	Attribute operator ->();
	
private:
	struct Private;

	Private* pvt;
};


class Attribute : public DataElement
{
public:
	Attribute( u16 id, DataElement& );

	u16 id();

	DataElement& elem();
private:
	Attribute();

friend class Record::iterator;
};

}//namespace Sdp

#endif//__SDP_RECORD_H
