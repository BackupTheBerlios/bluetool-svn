#include <bluetool/sdpdebug.h>
#include "sdp_record.h"

static char __sdprec_path[128];

static const char* __gen_sdprec_path( const char* server, const Sdp::Record& record )
{
	snprintf(__sdprec_path,sizeof(__sdprec_path), "%s/" BTOOL_SDPREC_SUBDIR "%08X",
			server, (uint)record.handle()
	);
	return __sdprec_path;
}

namespace Bluetool
{

SdpRecord::SdpRecord( const char* server, const Sdp::Record& record )
:	Sdp::Record(record),
	DBus::LocalInterface( BTOOL_SDPREC_IFACE ),
	DBus::LocalObject( __gen_sdprec_path(server,record), DBus::Connection::SystemBus() )
{
	register_method( SdpRecord, GetHandle );
	register_method( SdpRecord, GetGroupId );
	register_method( SdpRecord, GetServiceId );
	register_method( SdpRecord, GetAttribute );
	register_method( SdpRecord, GetServiceName );
	register_method( SdpRecord, GetServiceDescription );
	register_method( SdpRecord, GetProviderName );
	register_method( SdpRecord, GetDocUrl );
	register_method( SdpRecord, GetClassIdList );
	register_method( SdpRecord, GetProtocolDescList );

	sdp_dbg("record handle: 0x%08X", (uint)handle());

	Sdp::Record::iterator ait = this->begin();
	while( ait != this->end() )
	{
		sdp_dbg(" attribute id: 0x%04X", ait->id());
		if( ait->elem().is_string() )
		{
			Sdp::String str (ait->elem());
			sdp_dbg(" `-> %s",str.to_string());
		}
		++ait;
	}

}

void SdpRecord::GetAttribute( const DBus::CallMessage& msg )
{
	try
	{
		DBus::MessageIter ri = msg.r_iter();

		u16 attr_id = ri.get_uint16();

		Sdp::DataElement& attr = (*this)[attr_id];

		if( attr.is_string() )
		{
			Sdp::String str (attr);

			ri.append_string(str.to_string());
		}
		else
		if( attr.is_bool() )
		{
			Sdp::Bool b (attr);

			ri.append_bool(b.to_bool());
		}
		else
		if( attr.is_uuid() )
		{
			Sdp::UUID u (attr);

			ri.append_string(u.to_string());
		}
		else
		{
			ri.append_string("Unsupported format"); //TODO
		}
	}
	catch( std::exception& ex )	//both DBus::Error and Sdp::Error
	{
		DBus::ErrorMessage e (msg, BTOOL_ERROR, ex.what());
		conn().send(e);	
	}
}

void SdpRecord::GetHandle( const DBus::CallMessage& msg )
{
	try
	{
		u32 handle = Sdp::Record::handle();

		DBus::ReturnMessage reply (msg);

		reply.append( DBUS_TYPE_UINT32, &handle, DBUS_TYPE_INVALID );

		conn().send(reply);
	}
	catch( Sdp::Error& se )
	{
		DBus::ErrorMessage e (msg, BTOOL_ERROR, se.what());
		conn().send(e);
	}
}

void SdpRecord::GetGroupId( const DBus::CallMessage& msg )
{
	try
	{
		const char* uuid_str = Sdp::Record::get_group_id().to_string();

		DBus::ReturnMessage reply (msg);

		reply.append( DBUS_TYPE_STRING, &uuid_str, DBUS_TYPE_INVALID );

		conn().send(reply);
	}
	catch( Sdp::Error& se )
	{
		DBus::ErrorMessage e (msg, BTOOL_ERROR, se.what());
		conn().send(e);
	}
}

void SdpRecord::GetServiceId( const DBus::CallMessage& msg )
{
	try
	{
		const char* uuid_str = Sdp::Record::get_service_id().to_string();

		DBus::ReturnMessage reply (msg);

		reply.append( DBUS_TYPE_STRING, &uuid_str, DBUS_TYPE_INVALID );

		conn().send(reply);
	}
	catch( Sdp::Error& se )
	{
		DBus::ErrorMessage e (msg, BTOOL_ERROR, se.what());
		conn().send(e);
	}
}

void SdpRecord::GetServiceName( const DBus::CallMessage& msg )
{
	try
	{
		const char* service_name = Sdp::Record::get_service_name().to_string();

		DBus::ReturnMessage reply (msg);

		reply.append( DBUS_TYPE_STRING, &service_name, DBUS_TYPE_INVALID );

		conn().send(reply);
	}
	catch( Sdp::Error& se )
	{
		DBus::ErrorMessage e (msg, BTOOL_ERROR, se.what());
		conn().send(e);
	}
}

void SdpRecord::GetServiceDescription( const DBus::CallMessage& msg )
{
	try
	{
		const char* service_desc = Sdp::Record::get_service_desc().to_string();

		DBus::ReturnMessage reply (msg);

		reply.append( DBUS_TYPE_STRING, &service_desc, DBUS_TYPE_INVALID );

		conn().send(reply);
	}
	catch( Sdp::Error& se )
	{
		DBus::ErrorMessage e (msg, BTOOL_ERROR, se.what());
		conn().send(e);
	}
}

void SdpRecord::GetProviderName( const DBus::CallMessage& msg )
{
	try
	{
		const char* provider_name = Sdp::Record::get_provider_name().to_string();

		DBus::ReturnMessage reply (msg);

		reply.append( DBUS_TYPE_STRING, &provider_name, DBUS_TYPE_INVALID );

		conn().send(reply);
	}
	catch( Sdp::Error& se )
	{
		DBus::ErrorMessage e (msg, BTOOL_ERROR, se.what());
		conn().send(e);
	}
}

void SdpRecord::GetDocUrl( const DBus::CallMessage& msg )
{
	try
	{
		const char* doc_url = Sdp::Record::get_doc_url().to_string();

		DBus::ReturnMessage reply (msg);

		reply.append( DBUS_TYPE_STRING, &doc_url, DBUS_TYPE_INVALID );

		conn().send(reply);
	}
	catch( Sdp::Error& se )
	{
		DBus::ErrorMessage e (msg, BTOOL_ERROR, se.what());
		conn().send(e);
	}
}

void SdpRecord::GetClassIdList( const DBus::CallMessage& msg )
{
	try
	{
		Sdp::DataElementSeq& list = Sdp::Record::get_class_id_list();

		Sdp::DataElementList::iterator it = list.begin();

		DBus::ReturnMessage reply (msg);
		DBus::MessageIter wit = reply.w_iter();
		DBus::MessageIter ait = wit.new_array( DBUS_TYPE_UINT16 );

		while( it != list.end() )
		{
			Sdp::UUID& id = *it;

			ait.append_uint16( id.to_u16() );

			++it;
		}
		wit.close_container(ait);
		conn().send(reply);
	}
	catch( Sdp::Error& se )
	{
		DBus::ErrorMessage e (msg, BTOOL_ERROR, se.what());
		conn().send(e);
	}
}

void SdpRecord::GetProtocolDescList( const DBus::CallMessage& msg )
{
	try
	{
		Sdp::DataElementSeq& list = Sdp::Record::get_protocol_desc_list();

		Sdp::DataElementList::iterator it = list.begin();

		DBus::ReturnMessage reply (msg);
		DBus::MessageIter wit = reply.w_iter();
		DBus::MessageIter ait = wit.new_array("(qq)");

		while( it != list.end() )
		{
			Sdp::DataElementSeq& descriptor = *it;

			Sdp::DataElementList::iterator dit = descriptor.begin();

			Sdp::UUID& proto_id = *dit;
			++dit;

			DBus::MessageIter pit = ait.new_struct("qq");
			pit.append_uint16(proto_id.to_u16());

			if( dit != descriptor.end() )
			{
				Sdp::DataElement& proto_prt = *dit;

				if( proto_prt.is_u16() )
				{
					pit.append_uint16( ((Sdp::U16&)proto_prt).to_u16() );
				}
				else if( proto_prt.is_u8() )
				{
					pit.append_uint16( ((Sdp::U8&)proto_prt).to_u8() );
				}
			}
			else
			{
				pit.append_uint16(0xFFFF);
			}
			ait.close_container(pit);

			++it;
		}
		wit.close_container(ait);
		conn().send(reply);
	}
	catch( Sdp::Error& se )
	{
		DBus::ErrorMessage e (msg, BTOOL_ERROR, se.what());
		conn().send(e);
	}
}

}//namespace Bluetool
