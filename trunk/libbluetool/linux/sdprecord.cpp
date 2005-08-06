#include "../sdperror.h"
#include "../sdprecord.h"

#include <bluetooth/bluetooth.h>
#include <bluetooth/sdp.h>
#include <bluetooth/sdp_lib.h>

namespace Sdp
{

struct DataElement::Private
{
/*	Private(sdp_data_t* data = NULL) : elem(data)
	{
		if(!data) data = new sdp_data_t;
	}
*/
	sdp_data_t* elem;
	bool alloc;
};

DataElement::DataElement
{
	pvt = new Private;
}

DataElement::~DataElement
{
	if(noref())
	{
		if(alloc) sdp_data_free(pvt->elem);
		delete pvt;
	}
}

DataElement::operator Bool&()
{
	if( pvt->elem.dtd != SDP_BOOL )
		throw Error("type mismatch");

	return static_cast<Bool&>(*this);
}

DataElement::operator String&()
{
	switch( pvt->elem.dtd )
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
	switch( pvt->elem.dtd )
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

/*	booleans
*/

Bool::Bool( bool b )
{
	pvt->alloc = true;
	pvt->elem = sdp_data_alloc(SDP_BOOL,&b);
}

bool to_bool()
{
	return pvt->elem->val.uint8 != 0; 
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

/*	UUIDs
*/

UUID::UUID( u16 value )
{
	pvt->alloc = true;
	pvt->elem = (sdp_data_t*)malloc(sizeof(sdp_data_t));

	uuid_t tmp;
	sdp_uuid16_create( &tmp, value);
	sdp_uuid16_to_uuid128( &(pvt->elem->val.uuid), &tmp
}

UUID::UUID( u32 value )
{
	pvt->alloc = true;
	pvt->elem = (sdp_data_t*)malloc(sizeof(sdp_data_t));

	uuid_t tmp;
	sdp_uuid32_create( &tmp, value);
	sdp_uuid32_to_uuid128( &(pvt->elem->val.uuid), &tmp );
}

UUID::UUID( u128 value )
{
	pvt->alloc = true;
	pvt->elem = (sdp_data_t*)malloc(sizeof(sdp_data_t));

	sdp_uuid128_create( &(pvt->elem->val.uuid), value);
}

bool UUID::operator == ( const UUID& u )
{
	return sdp_uuid128_cmp( &(pvt->elem->val.uuid), &(u.pvt->elem->val.uuid) );
}

std::string UUID::to_string()
{
	char buf[64] = {0};

	if( sdp_uuid2strn(&(pvt->elem->val.uuid), buf, sizeof(buf)) < 0 )
		throw Error("invalid UUID");

	return std::string(buf);
}

/*	Attributes
*/

Attribute::Attribute( u16 id, DataElement& de )
:	DataElement(de)
{
	pvt->elem->attrId = id;
}

u16 Attribute::id()
{
	return pvt->elem->attrId;
}

DataElement& Attribute::DataElement()
{
	return (*this);
}

/*	Records
*/

struct Record::Private
{	
	Private(sdp_record_t* data = NULL) : rec(data)
	{
		if(!rec) rec = sdp_record_alloc();
	}

	sdp_record_t* rec;
};

Record::Record()
{
	pvt = new Private;
}

Record::~Record()
{
	if(noref())
	{
		sdp_record_free(pvt->rec);
		delete pvt;
	}
}

Attribute Record::operator []( u16 attr_id )
{
	sdp_data_t* d = sdp_data_get(pvt->rec,id);
	if(d)
	{
		DataElement de;
		de.alloc = false;
		de.elem = d;
		return Attribute(attr_id, de);
	}
	else	throw Error("No such attribute");
}

void Record::add( Attribute& attr )
{
	sdp_attr_add(pvt->rec, attr.id(), attr->pvt->elem);
}

void Record::remove( u16 id )
{
	//TODO
}

}//namespace Sdp
