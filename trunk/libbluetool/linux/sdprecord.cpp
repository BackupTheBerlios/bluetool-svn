#include "../sdperror.h"
#include "sdprecord_p.h"

namespace Sdp
{

sdp_data_t* DataElement::Private::new_seq( DataElementSeq& des )
{
	DataElementSeq::iterator i = des.begin();

	sdp_data_t* list = i->pvt->elem;

	while(i != des.end())
	{
		DataElementSeq::iterator n = i;
		if(++n != des.end())
		{
			i->pvt->elem->next = n->pvt->elem;
			break;
		}
		else
		{
			i->pvt->elem->next = NULL;		
		}
		++i;
	}
	sdp_data_t* seq = sdp_data_alloc(SDP_SEQ8, list);
	return seq;
}

/*
*/

DataElement::DataElement()
{
	pvt = new Private;
}

DataElement::DataElement( const DataElement& de )
{
	pvt = new Private;
	pvt->elem = sdp_data_alloc(SDP_DATA_NIL, NULL);
	pvt->alloc = true;
	memcpy(pvt->elem, de.pvt->elem, sizeof(sdp_data_t));

	//TODO: this is BAD for copying strings or sequences
}

DataElement::~DataElement()
{
	if(pvt->alloc) sdp_data_free(pvt->elem);
	delete pvt;
}

const DataElement& DataElement::operator = ( const DataElement& de )
{
	if( &de != this )
	{
		if(pvt->alloc) sdp_data_free(pvt->elem);
		delete pvt;

		DataElement::DataElement(de);
	}
	return *this;
}
/*
DataElement::operator Bool&()
{
	if( pvt->elem->dtd != SDP_BOOL )
		throw Error("type mismatch");

	return static_cast<Bool&>(*this);
}

DataElement::operator U32&()
{
	if( pvt->elem->dtd != SDP_UINT32 )
		throw Error("type mismatch");

	return static_cast<U32&>(*this);
}

DataElement::operator String&()
{
	switch( pvt->elem->dtd )
	{
		case SDP_URL_STR8:
		case SDP_TEXT_STR8:
		case SDP_URL_STR16:
		case SDP_TEXT_STR16:
			break;
		default:
			throw Error("type mismatch");		
	}
	return static_cast<String&>(*this);
}

DataElement::operator UUID&()
{
	switch( pvt->elem->dtd )
	{
		case SDP_UUID16:
		case SDP_UUID32:
		case SDP_UUID128:
			break;
		default:
			throw Error("type mismatch");		
	}
	return static_cast<UUID&>(*this);
}
*/
/*	booleans
*/

Bool::Bool( bool b )
{
	pvt->alloc = true;
	pvt->elem = sdp_data_alloc(SDP_BOOL,&b);
}

bool Bool::to_bool()
{
	return pvt->elem->val.uint8 != 0; 
}

void Bool::operator = ( bool b )
{
	pvt->elem->val.uint8 = (u8)b;
}

/*	various format integers
*/

U32::U32( u32 u )
{
	pvt->alloc = true;
	pvt->elem = sdp_data_alloc(SDP_UINT32, &u);
}

u32 U32::to_u32()
{
	return pvt->elem->val.uint32;
}

void U32::operator = ( u32 u )
{
	pvt->elem->val.uint32 = u;
}

/*	strings
*/

String::String( const char* string )
{
	pvt->alloc = true;
	pvt->elem = sdp_data_alloc(SDP_TEXT_STR16, string);
}

std::string String::to_string()
{
	return std::string(pvt->elem->val.str);
}

void String::operator = ( std::string& s )
{
	free(pvt->elem->val.str);
	pvt->elem->val.str = (char*)malloc(s.length());
	strcpy(pvt->elem->val.str, s.c_str());
}

/*	UUIDs
*/

UUID::UUID( u16 value )
{
	pvt->alloc = true;
	pvt->elem = sdp_data_alloc(SDP_UUID16, &value);

//	sdp_uuid16_to_uuid128( &(pvt->elem->val.uuid), &tmp );
}

UUID::UUID( u32 value )
{
	pvt->alloc = true;
	pvt->elem = sdp_data_alloc(SDP_UUID32, &value);

	//sdp_uuid32_to_uuid128( &(pvt->elem->val.uuid), &tmp ); //todo
}

UUID::UUID( u128 value )
{
	pvt->alloc = true;
	pvt->elem = sdp_data_alloc(SDP_UUID128,&value);

	sdp_uuid128_create( &(pvt->elem->val.uuid), value);
}

bool UUID::operator == ( const UUID& u )
{
	return sdp_uuid128_cmp( &(pvt->elem->val.uuid), &(u.pvt->elem->val.uuid) ); //TODO: compare types
}

std::string UUID::to_string()
{
	char buf[64] = {0};

	if( sdp_uuid2strn(&(pvt->elem->val.uuid), buf, sizeof(buf)) < 0 )
		throw Error("invalid UUID");

	return std::string(buf);
}

void UUID::operator = ( u16 u )
{
	UUID tu = UUID(u);
	DataElement::operator = (tu);
}

void UUID::operator = ( u32 u )
{
	UUID tu = UUID(u);
	DataElement::operator = (tu);
}

void UUID::operator = ( u128 u )
{
	UUID tu = UUID(u);
	DataElement::operator = (tu);
}

/*	Attributes
*/

Attribute::Attribute()
{}

Attribute::Attribute( u16 id, DataElement& de )
:	DataElement(de)
{
	pvt->elem->attrId = id;
}

u16 Attribute::id()
{
	return pvt->elem->attrId;
}

DataElement& Attribute::elem()
{
	return (*this);
}

/*	Records
*/

Record::Record()
{
	pvt = new Private;
	pvt->alloc = true;
	pvt->rec = sdp_record_alloc();
}

Record::Record( Private* p )
{
	pvt = p;
}

Record::Record( const Record& r )
{
	pvt = new Private;
	pvt->alloc = true;
	pvt->rec = sdp_record_alloc();
	memcpy(pvt->rec,r.pvt->rec,sizeof(sdp_record_t));
	//todo : this is wrong and leads to double free()s
}

Record::~Record()
{
	if(pvt->alloc) sdp_record_free(pvt->rec);
	delete pvt;
}

Attribute Record::operator []( u16 attr_id )
{
	sdp_data_t* d = sdp_data_get(pvt->rec,attr_id);
	if(d)
	{
		DataElement de;
		de.pvt->alloc = false;
		de.pvt->elem = d;
		return Attribute(attr_id, de);
	}
	else	throw Error("No such attribute");
}

void Record::add( Attribute& attr )
{
	sdp_attr_add(pvt->rec, attr.id(), attr.pvt->elem);
}

void Record::remove( u16 id )
{
	sdp_attr_remove(pvt->rec, id);
}

struct Record::iterator::Private
{};

Record::iterator::iterator(Record::Private* _pvt)
{
	pvt = _pvt && _pvt->rec
		? (Record::iterator::Private*)
			(_pvt->rec->attrlist)
		: NULL;
}

const Record::iterator& Record::iterator::operator ++()
{
	if(pvt)
		pvt = (Record::iterator::Private*)
			((sdp_list_t*)pvt)->next;
	return (*this);
}

bool Record::iterator::operator == (const iterator& i)
{
	return pvt == i.pvt;
}

Attribute Record::iterator::operator *()
{
	Attribute a;
	a.pvt->elem = (sdp_data_t*)((sdp_list_t*)pvt)->data;
	return a;
}

Attribute Record::iterator::operator ->()
{
	return *(*this);
}

Record::iterator Record::begin()
{
	return iterator (pvt);
}

Record::iterator Record::end()
{
	return iterator (NULL);
}


}//namespace Sdp
