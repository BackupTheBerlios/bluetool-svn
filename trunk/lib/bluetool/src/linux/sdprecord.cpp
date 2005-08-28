#include <bluetool/sdperror.h>
#include <bluetool/sdpdebug.h>
#include "sdprecord_p.h"

namespace Sdp
{

DataElement::Private::Private()
{
	sdp_dbg_enter();
	sdp_dbg_leave();
}

DataElement::Private::~Private()
{
	sdp_dbg_enter();
	if(alloc)
	{
		if(is_seq())
			elem->val.dataseq = NULL;

		sdp_data_free(elem);
	}
	sdp_dbg_leave();
}

bool DataElement::Private::is_seq()
{
	switch( elem->dtd )
	{
		case SDP_SEQ8:
		case SDP_SEQ16:
		case SDP_SEQ32:
		case SDP_ALT8:
		case SDP_ALT16:
		case SDP_ALT32:
			return true;
		default:
			return false;
	}
}

bool DataElement::Private::is_bool()
{
	return elem->dtd == SDP_BOOL;
}

bool DataElement::Private::is_string()
{
	switch( elem->dtd )
	{
		case SDP_URL_STR8:
		case SDP_TEXT_STR8:
		case SDP_URL_STR16:
		case SDP_TEXT_STR16:
			return true;
		default:
			return false;
	}
}

bool DataElement::Private::is_uuid()
{
	switch( elem->dtd )
	{
		case SDP_UUID16:
		case SDP_UUID32:
		case SDP_UUID128:
			return true;
		default:
			return false;
	}
}
#if 0
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
#endif
/*
*/
void DataElement::Private::fill( sdp_data_t* raw )
{
	sdp_dbg_enter();
	alloc = true;
	elem = raw;

	if( is_seq() )
	{
		for( sdp_data_t* p = elem->val.dataseq; p; p = p->next )
		{
			DataElement de;
			de.pvt->fill( p );

			list.push_back(de);
		}
		sdp_dbg("-------->list len=%d",list.size());
	}
	if( is_uuid() )
	{
		sdp_dbg("UUID: %X", elem->val.uuid.value.uuid16);
	}
	if( is_string() )
	{
		sdp_dbg("String: %s", elem->val.str);
	}

	sdp_dbg_leave();
}

DataElement::DataElement()
: 	pvt(new Private)
{
}

DataElement::~DataElement()
{}

bool DataElement::is_seq()
{
	return pvt->is_seq();
}

bool DataElement::is_bool()
{
	return pvt->elem->dtd == SDP_BOOL;
}

bool DataElement::is_string()
{
	return pvt->is_string();
}

bool DataElement::is_uuid()
{
	return pvt->is_uuid();
}
/*
DataElement::DataElement( const DataElement& de )
: 	pvt(new Private)
{
	pvt->elem = sdp_data_alloc(SDP_DATA_NIL, NULL);
	pvt->alloc = true;
	memcpy(pvt->elem, de.pvt->elem, sizeof(sdp_data_t));

	//TODO: this is BAD for copying strings or sequences
}
*/
/*
DataElement::~DataElement()
{
	if(pvt->alloc) sdp_data_free(pvt->elem);
}
*/

const DataElement& DataElement::operator = ( const DataElement& de )
{
	if( &de != this )
	{
		if(pvt->alloc) sdp_data_free(pvt->elem);

		DataElement::DataElement(de);
	}
	return *this;
}

DataElement::operator DataElement&()
{
	return (*this);
}

DataElement::operator DataElementSeq&()
{
	if(!is_seq())
		throw Error("type mismatch");		

	return static_cast<DataElementSeq&>(*this);
}

DataElement::operator Bool&()
{
	if( pvt->elem->dtd != SDP_BOOL )
		throw Error("type mismatch");

	return static_cast<Bool&>(*this);
}

DataElement::operator U16&()
{
	if( pvt->elem->dtd != SDP_UINT16 )
		throw Error("type mismatch");

	return static_cast<U16&>(*this);
}

DataElement::operator U32&()
{
	if( pvt->elem->dtd != SDP_UINT32 )
		throw Error("type mismatch");

	return static_cast<U32&>(*this);
}

DataElement::operator String&()
{
	if(!is_string())
		throw Error("type mismatch");		

	return static_cast<String&>(*this);
}

DataElement::operator UUID&()
{
	if(!is_uuid())
		throw Error("type mismatch");		

	return static_cast<UUID&>(*this);
}

/*	data element sequence
*/

DataElementSeq::DataElementSeq()
{
	pvt->alloc = true;
	pvt->elem = sdp_data_alloc(SDP_SEQ8,NULL);
}

void DataElementSeq::push_back( DataElement& de )
{
	pvt->list.push_back(de);
	
	de.pvt->elem->next = pvt->elem->val.dataseq;
	pvt->elem->val.dataseq = de.pvt->elem;

	pvt->elem->unitSize += de.pvt->elem->unitSize;
}

void DataElementSeq::erase( DataElementList::iterator& it )
{
	//todo, unlink the list first
	pvt->elem->unitSize -= it->pvt->elem->unitSize;

	pvt->list.erase(it);
}

DataElementList::iterator DataElementSeq::begin()
{
	return pvt->list.begin();
}

DataElementList::iterator DataElementSeq::end()
{
	return pvt->list.end();
}

DataElementList::const_iterator DataElementSeq::begin() const
{
	return pvt->list.begin();
}

DataElementList::const_iterator DataElementSeq::end() const
{
	return pvt->list.end();
}

#if 0
DataElementSeq::iterator::iterator( DataElementList::iterator lit )
: it(lit)
{}

const DataElementSeq::iterator& DataElementSeq::iterator::operator ++()
{
	++it;
	return (*this);
}

bool DataElementSeq::iterator::operator == ( const DataElementSeq::iterator& oit )
{
	return it == oit.it;
}

DataElement* DataElementSeq::iterator::operator * ()
{
	return *it;
}

DataElement* DataElementSeq::iterator::operator -> ()
{
	return *it;
}
#endif
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


U16::U16( u16 u )
{
	pvt->alloc = true;
	pvt->elem = sdp_data_alloc(SDP_UINT16, &u);
}

u16 U16::to_u16()
{
	return pvt->elem->val.uint16;
}

void U16::operator = ( u16 u )
{
	pvt->elem->val.uint16 = u;
}

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

/*
std::string String::to_string()
{
	return std::string(pvt->elem->val.str);
}
*/
const char* String::to_string()
{
	return pvt->elem->val.str;
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

const char* UUID::to_string()
{
	static char buf[64] = {0};

	if( sdp_uuid2strn(&(pvt->elem->val.uuid), buf, sizeof(buf)) < 0 )
		throw Error("invalid UUID");

	return buf;
}

u16 UUID::to_u16()
{
	return pvt->elem->val.uuid.value.uuid16;
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

Record::Private::~Private()
{
	if(alloc)
	{
		rec->attrlist = NULL;
		sdp_record_free(rec);
	}
}

Record::Record()
: 	pvt(new Private)
{
	pvt->alloc = true;
	pvt->rec = sdp_record_alloc();
}

Record::Record( Private* p )
: pvt(p)
{
	for( sdp_list_t* l = pvt->rec->attrlist; l; l = l->next )
	{
		sdp_data_t* a = (sdp_data_t*)l->data;

		DataElement de;
		de.pvt->fill(a);

		pvt->attrs.push_back(Attribute(a->attrId,de));
	}
}

/*
Record::Record( const Record& r )
:	pvt(new Private)
{
//	pvt->alloc = true;
//	pvt->rec = sdp_record_alloc();
//	memcpy(pvt->rec,r.pvt->rec,sizeof(sdp_record_t));
	//todo : this is wrong and leads to double free()s
}
*/

u32 Record::handle() const
{
	return pvt->rec->handle;
}

DataElement& Record::operator []( u16 attr_id ) const
{
	AttributeList::iterator ait = pvt->attrs.begin();

	while( ait != pvt->attrs.end() )
	{
		if( ait->id() == attr_id )
			return ait->elem();
		++ait;
	}
	throw Error("No such attribute");
}
#if 0
DataElementSeq& Record::get_seq_attr( u16 attr_id )
{
	DataElementSeqList::iterator sit = pvt->seqs.begin();

	while( sit != pvt->seqs.end() )
	{
		if( sit->pvt->elem->attrId == attr_id )
			return  *sit;
		++sit;
	}
	throw Error("No such element");
}
#endif
void Record::add( Attribute& attr )
{
	throw "Unimpl";
	//sdp_attr_add(pvt->rec, attr.id(), attr.pvt->elem);
}

void Record::remove( u16 id )
{
	throw "Unimpl";
	//sdp_attr_remove(pvt->rec, id);
}

String& Record::get_service_name() const
{
	return (*this)[SDP_ATTR_SVCNAME_PRIMARY];
}

String& Record::get_service_desc() const
{
	return (*this)[SDP_ATTR_SVCDESC_PRIMARY];
}

String& Record::get_provider_name() const
{
	return (*this)[SDP_ATTR_PROVNAME_PRIMARY];
}

String& Record::get_doc_url() const
{
	return (*this)[SDP_ATTR_DOC_URL];
}

U32& Record::get_state() const
{
	return (*this)[SDP_ATTR_RECORD_STATE];
}

UUID& Record::get_service_id() const
{
	return (*this)[SDP_ATTR_SERVICE_ID];
}

UUID& Record::get_group_id() const
{
	return (*this)[SDP_ATTR_GROUP_ID];
}

DataElementSeq& Record::get_class_id_list() const
{
	return (*this)[SDP_ATTR_SVCLASS_ID_LIST];
}

DataElementSeq& Record::get_protocol_desc_list() const
{
	return (*this)[SDP_ATTR_PROTO_DESC_LIST];
}

/*DataElementSeq& Record::get_profile_desc_list()
{
	return (*this)[SDP_ATTR_SVCLASS_ID_LIST];
}*/

Record::iterator::iterator(AttributeList::iterator ait)
{
	it = ait;
}

const Record::iterator& Record::iterator::operator ++()
{
	++it;
	return (*this);
}

bool Record::iterator::operator == (const iterator& i)
{
	return it == i.it;
}

Attribute* Record::iterator::operator *()
{
	return &(*it);
}

Attribute* Record::iterator::operator ->()
{
	return &(*it);
}

Record::iterator Record::begin() const
{
	return iterator (pvt->attrs.begin());
}

Record::iterator Record::end() const
{
	return iterator (pvt->attrs.end());
}


}//namespace Sdp
