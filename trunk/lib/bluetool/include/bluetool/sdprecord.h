#ifndef __SDP_RECORD_H
#define __SDP_RECORD_H

#include <common/types.h>
#include <common/refptr.h>

#include <sigc++/sigc++.h>
#include <list>
#include <string>
#include <vector>

namespace Sdp
{
	class DataElement;	
	typedef std::vector<DataElement> DataElementList;

	class DataElementSeq;
	typedef std::list<DataElementSeq> DataElementSeqList;

	class Bool;
	class U16;
	class U32;
	class String;
	class UUID;

	class Attribute;
	typedef std::list<Attribute> AttributeList;
	typedef std::list< RefPtr<Attribute> > AttributeRList;

	class Record;
	typedef std::list<Record> RecordList;

	typedef sigc::signal<void, u16, const RecordList&> SdpEvent;
}

namespace Sdp
{

class DataElement
{
protected:
	DataElement();

public:
	virtual ~DataElement();

	const DataElement& operator = ( const DataElement& );

	bool is_seq();
	bool is_bool();
	bool is_string();
	bool is_uuid();

	operator DataElement&();	//needed by subclasses

	operator DataElementSeq&();
	operator Bool&();
	operator U16&();
	operator U32&();
	operator String&();
	operator UUID&();

	struct Private;

protected:
	RefPtr<Private> pvt;

friend class Record;
friend class Client;
friend class DataElementSeq;
};

class DataElementSeq : public DataElement
{
public:
	DataElementSeq();

	DataElementList::iterator begin();

	DataElementList::iterator end();

	DataElementList::const_iterator begin() const;

	DataElementList::const_iterator end() const;

	void push_back( DataElement& );

	void erase( DataElementList::iterator& );
};

#if 0
class DataElementSeq::iterator
{
public:
	const iterator& operator ++ ();

	bool operator == ( const iterator& );
	bool operator != ( const iterator& i )
	{
		return !((*this)==i);
	}

	DataElement* operator * ();
	DataElement* operator -> ();

private:
	iterator( DataElementList::iterator );

	iterator( DataElementList::const_iterator );

	DataElementList::iterator it;

friend class DataElementSeq;
};
#endif

class Bool : public DataElement
{
public:
	Bool( bool );

	bool to_bool();

	void operator = ( bool b );
};


class U16 : public DataElement
{
public:
	U16( u16 );

	u16 to_u16();

	void operator = ( u16 u );
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

	const char* to_string();

	void operator = ( std::string& s );
};

class UUID : public DataElement
{
public:
	UUID( u16 );

	UUID( u32 );

	UUID( u128 );

	const char* to_string();
	u16 to_u16();

	bool operator == ( const UUID& );

	void operator = ( u16 u );
	void operator = ( u32 u );
	void operator = ( u128 u );
};

class Record
{
public:
	Record();

	DataElement& operator[]( u16 attr_id ) const;

//	DataElementSeq& get_seq_attr( u16 attr_id );

	u32 handle() const;

	void add( Attribute& );

	void remove( u16 attr_id );

	String& get_service_name() const;

	String& get_service_desc() const;

	String& get_provider_name() const;

	String& get_doc_url() const;

	U32& get_state() const;

	UUID& get_service_id() const;

	UUID& get_group_id() const;

	DataElementSeq& get_class_id_list() const;

	DataElementSeq& get_profile_desc_list() const;

	DataElementSeq& get_protocol_desc_list() const;
/*
	languages();

	profile_descriptions();

	server_versions();

	U32& record_state();

	U8& service_availability();
	
	U32& service_ttl();

	U32& database_state();
*/
	void add_service_id( UUID& );

	void add_group_id( UUID& );

	class iterator;

	iterator begin() const;
	iterator end() const;

	struct Private;

	Record( Private* );

private:
	RefPtr<Private> pvt;
};

class Record::iterator
{
public:
	const iterator& operator ++();

	bool operator == (const iterator& i);
	bool operator != (const iterator& i)
	{
		return !((*this)==i);
	}

	Attribute* operator *();

	Attribute* operator ->();
	
private:
	iterator( AttributeList::iterator );

	AttributeList::iterator it;

friend class Record;
};


class Attribute : public DataElement
{
public:
	Attribute( u16 id, DataElement& );

	u16 id();

	DataElement& elem();

private:
	Attribute();

friend class Record;
friend class Record::iterator;
};

}//namespace Sdp

#endif//__SDP_RECORD_H
