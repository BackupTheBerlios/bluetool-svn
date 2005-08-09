#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>

#include "../bluedebug.h"
#include "hci_device.h"

#include <map>

#include <cstring>
#include <cstdio>

HciDevice::HciDevice( std::string iface_name )
:
	DBus::LocalInterface	( DBUS_HCIDEV_IFACE ),
	DBus::LocalObject	( (DBUS_HCIDEV_PATH + iface_name).c_str(), DBus::Connection::SystemBus() ),
	_device			( iface_name.c_str() )
{

	/*	export methods
	*/
	register_method( HciDevice, GetProperty );
	register_method( HciDevice, SetProperty );
	register_method( HciDevice, StartInquiry );

	/*	method receiving (asynchronous) hardware events
	*/
	_device.on_event.connect( sigc::mem_fun(this, &HciDevice::on_hci_event ));
}

/*	device properties management
*/

void HciDevice::GetProperty( const DBus::CallMessage& msg )
{
	DBus::MessageIter i = msg.r_iter();
	std::string property = i.get_string();

	blue_dbg("method GetProperty(%s) called on %s",property.c_str(),DBus::LocalObject::name().c_str());

	try
	{

	DBus::ReturnMessage *reply = new DBus::ReturnMessage( msg );

	DBus::MessageIter wr = reply->w_iter();
//	wr.append_bool(true);

	
	if( property == "auth_enable" )
	{
		wr.append_bool( _device.auth_enable() );
	}
	
	else
	if( property == "encrypt_enable" )
	{
		wr.append_bool( _device.encrypt_enable() );
	}
	
	else
	if( property == "security_manager_enable" )
	{
		wr.append_bool( _device.secman_enable() );
	}
	
	else
	if( property == "page_scan_enable" )
	{
		wr.append_bool( _device.pscan_enable() );
	}
	
	else
	if( property == "inquiry_scan_enable" )
	{
		wr.append_bool( _device.iscan_enable() );
	}
	
	else
	if( property == "packet_type" )
	{
	}
	
	else
	if( property == "link_mode" )
	{
	}
	
	else
	if( property == "link_policy" )
	{
	}
	
	else
	if( property == "local_name" )
	{
		_device.local_name(reply, HCI_TIMEOUT);
	}
	
	else
	if( property == "device_class" )
	{
		_device.get_class(reply, HCI_TIMEOUT);
	}
	
	else
	if( property == "voice_setting" )
	{
		_device.get_voice_setting(reply, HCI_TIMEOUT);
	}
	
	else
	if( property == "inquiry_access_mode" )
	{
	}
	
	else
	if( property == "inquiry_mode" )
	{
	}
	
	else
	if( property == "inquiry_scan_type" )
	{
	}
	
	else
	if( property == "inquiry_scan_window" )
	{
	}
	
	else
	if( property == "inquiry_scan_interval" )
	{
	}
	
	else
	if( property == "page_scan_window" )
	{
	}
	
	else
	if( property == "page_scan_interval" )
	{
	}
	else
	
	if( property == "afh_mode" )
	{
	}
	
	else
	if( property == "features" )
	{
		_device.get_features(reply,HCI_TIMEOUT);
	}
	
	else
	if( property == "version_info" )
	{
		_device.get_version(reply,HCI_TIMEOUT);
	}
	
	else
	if( property == "revision_info" )
	{
	}
	
	else
	{
	}
		//send ok reply
		//wr.append_string( _device.local_name() );
		//reply.terminate();
		//conn().send(*reply);
	}
	catch( Hci::Exception& e )
	{
		//send error back to caller
		blue_dbg("hci: %s",e.what());
	}
}

void HciDevice::SetProperty( const DBus::CallMessage& msg )
{
	DBus::MessageIter i = msg.r_iter();
	std::string property = i.get_string();

	blue_dbg("method SetProperty(%s) called on %s",property.c_str(),DBus::LocalObject::name().c_str());

	try
	{

	DBus::ReturnMessage *reply = new DBus::ReturnMessage(msg);
	DBus::MessageIter wr = reply->w_iter();

	if( property == "auth_enable" )
	{
		bool enable = i.get_bool();
		_device.auth_enable(enable);
		
	}
	
	else
	if( property == "encrypt_enable" )
	{
		bool enable = i.get_bool();
		_device.encrypt_enable(enable);
		
	}
	
	else
	if( property == "security_manager_enable" )
	{
		bool enable = i.get_bool();
		_device.secman_enable(enable);
		
	}
	
	else
	if( property == "page_scan_enable" )
	{
		bool enable = i.get_bool();
		_device.pscan_enable(enable);
		
	}
	
	else
	if( property == "inquiry_scan_enable" )
	{
		bool enable = i.get_bool();
		_device.iscan_enable(enable);
		
	}
	
	else
	if( property == "packet_type" )
	{
	}
	
	else
	if( property == "link_mode" )
	{
	}
	
	else
	if( property == "link_policy" )
	{
	}
	
	else
	if( property == "local_name" )
	{
		const char* name = i.get_string();
		_device.local_name(name, reply, HCI_TIMEOUT);
	}
	
	else
	if( property == "device_class" )
	{
	}
	
	else
	if( property == "voice_setting" )
	{
	}
	
	else
	if( property == "inquiry_access_mode" )
	{
	}
	
	else
	if( property == "inquiry_mode" )
	{
	}
	
	else
	if( property == "inquiry_scan_type" )
	{
	}
	
	else
	if( property == "inquiry_scan_window" )
	{
	}
	
	else
	if( property == "inquiry_scan_interval" )
	{
	}
	
	else
	if( property == "page_scan_window" )
	{
	}
	
	else
	if( property == "page_scan_interval" )
	{
	}
	else
	
	if( property == "afh_mode" )
	{
	}
	
	else
	if( property == "features" )
	{
	}
	
	else
	if( property == "version_info" )
	{
	}
	
	else
	if( property == "revision_info" )
	{
	}
	
	else
	{
	}
	
		
		//everything went fine, reply
		//conn().send(*reply);
	
	}
	catch( Hci::Exception& e )
	{
		//send error back to caller
	}
}

void HciDevice::StartInquiry( const DBus::CallMessage& msg )
{
	DBus::ReturnMessage* reply = new DBus::ReturnMessage(msg);

	_device.start_inquiry(NULL, 0, reply);
}

void HciDevice::on_hci_event( const Hci::EventPacket& evt, void* cookie, bool timedout )
{
	DBus::ReturnMessage* reply = (DBus::ReturnMessage *) cookie;

	if( timedout )
	{
		
	}

	switch( evt.code )
	{
		case EVT_CMD_STATUS:
		{
			handle_command_status(evt,reply);
			break;
		}
		case EVT_CMD_COMPLETE:
		{
			handle_command_complete(evt,reply);
			break;
		}
	}

	conn().send(*reply);
	delete reply;
}

void HciDevice::handle_command_status( const Hci::EventPacket& evt, DBus::ReturnMessage* reply )
{
}

void HciDevice::handle_command_complete( const Hci::EventPacket& evt, DBus::ReturnMessage* reply )
{
	switch( evt.ogf )
	{
		case OGF_LINK_CTL:
		{
			switch( evt.ocf )
			{
			}
			break;
		}
		case OGF_LINK_POLICY:
		{
			switch( evt.ocf )
			{
			}
			break;
		}
		case OGF_HOST_CTL:
		{
			switch( evt.ocf )
			{
				case OCF_CHANGE_LOCAL_NAME:
				{
					break;
				}
				case OCF_READ_LOCAL_NAME:
				{
					read_local_name_rp* r = (read_local_name_rp*)evt.edata;
					const char* local_name = (char*)r->name;

					blue_dbg("local name request returned '%s'",local_name);

					reply->append(DBUS_TYPE_STRING,&local_name,DBUS_TYPE_INVALID);
					break;
				}
			}
			break;
		}
		case OGF_INFO_PARAM:
		{
			switch( evt.ocf )
			{
			}
			break;
		}
		case OGF_STATUS_PARAM:
		{
			break;
		}
	}
}
