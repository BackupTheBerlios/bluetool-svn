#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>

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

HciDevice::~HciDevice()
{
	clear_cache();
}

/*	device properties management
*/

void HciDevice::GetProperty( const DBus::CallMessage& msg )
{
	DBus::MessageIter i = msg.r_iter();
	std::string property = i.get_string();

	blue_dbg("method GetProperty(%s) called on %s",property.c_str(),oname().c_str());

	DBus::ReturnMessage *reply = new DBus::ReturnMessage( msg );

	try
	{

	if( property == "auth_enable" )	
		_device.get_auth_enable(reply, HCI_TIMEOUT);
	else
	if( property == "encrypt_mode" )
		_device.get_encrypt_mode(reply, HCI_TIMEOUT);
	else
//	if( property == "security_manager_enable" )
//		_device.get_secman_enable();
//	else
	if( property == "scan_type" )
		_device.get_scan_type(reply, HCI_TIMEOUT);
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
	if( property == "address" )
		_device.get_addr(reply, HCI_TIMEOUT);
	else
	if( property == "local_name" )
		_device.get_local_name(reply, HCI_TIMEOUT);
	else
	if( property == "device_class" )
		_device.get_class(reply, HCI_TIMEOUT);
	else
	if( property == "voice_setting" )
		_device.get_voice_setting(reply, HCI_TIMEOUT);
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
		_device.get_features(reply,HCI_TIMEOUT);
	else
	if( property == "version_info" )
		_device.get_version(reply,HCI_TIMEOUT);
	else
	if( property == "revision_info" )
	{
	}
	
	else
	{
		u16 err = 1;
		const char* strerr = "No such property";

		reply->append( DBUS_TYPE_UINT16, &err,
			       DBUS_TYPE_STRING, &strerr,
			       DBUS_TYPE_INVALID
		);
		conn().send(*reply);
		delete reply;
	}
	
	}
	catch( Hci::Exception& e )
	{
		u16 err = 1;
		const char* strerr = e.what();

		reply->append( DBUS_TYPE_UINT16, &err,
			       DBUS_TYPE_STRING, &strerr,
			       DBUS_TYPE_INVALID
		);
		conn().send(*reply);
		delete reply;
	}
}

void HciDevice::SetProperty( const DBus::CallMessage& msg )
{
	DBus::MessageIter i = msg.r_iter();
	std::string property = i.get_string();
	++i;

	blue_dbg("method SetProperty(%s) called on %s",property.c_str(),oname().c_str());

	DBus::ReturnMessage *reply = new DBus::ReturnMessage(msg);

	try
	{

	if( property == "auth_enable" )
	{
		u8 enable = i.get_byte();
		_device.set_auth_enable(enable,reply,HCI_TIMEOUT);	
	}
	
	else
	if( property == "encrypt_mode" )
	{
		u8 mode = i.get_byte();
		_device.set_encrypt_mode(mode,reply,HCI_TIMEOUT);
	}
	
//	else
//	if( property == "security_manager_enable" )
//	{
//		bool enable = i.get_bool();
//		_device.set_secman_enable(enable,reply);
//	}
	
	else
	if( property == "scan_type" )
	{
		u8 type = i.get_byte();
		_device.set_scan_type(type,reply,HCI_TIMEOUT);
	}
	
//	else
//	if( property == "inquiry_scan_enable" )
//	{
	//	bool enable = i.get_bool();
	//	_device.set_iscan_enable(enable, reply);	
//	}
	
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
	if( property == "address" )
	{
	}

	else
	if( property == "local_name" )
	{
		const char* name = i.get_string();
		_device.set_local_name(name, reply, HCI_TIMEOUT);
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
		u16 err = 1;
		const char* strerr = "No such property";

		reply->append( DBUS_TYPE_UINT16, &err,
			       DBUS_TYPE_STRING, &strerr,
			       DBUS_TYPE_INVALID
		);
		conn().send(*reply);
		delete reply;
	}
	
	}
	catch( Hci::Exception& e )
	{
		u16 err = 1;
		const char* strerr = e.what();

		reply->append( DBUS_TYPE_UINT16, &err,
			       DBUS_TYPE_STRING, &strerr,
			       DBUS_TYPE_INVALID
		);
		conn().send(*reply);
		delete reply;
	}
}

void HciDevice::StartInquiry( const DBus::CallMessage& msg )
{
	DBus::ReturnMessage* reply = new DBus::ReturnMessage(msg);

	_device.start_inquiry(NULL, IREQ_CACHE_FLUSH, reply);

	timeval now;
	gettimeofday(&now, NULL);

	_time_last_inquiry = now.tv_sec*1000 + now.tv_usec/1000.0;
}

void HciDevice::update_cache( const BdAddr& addr, u8 pscan_rpt_mode, u8 pscan_mode, u16 clk_offset )
{
	const std::string straddr = addr.to_string();

	HciRemotePTable::iterator i = _inquiry_cache.find(straddr);
	if( i == _inquiry_cache.end() )
	{
		/* create new entry
		*/
		_inquiry_cache[straddr] = new HciRemote(*this, addr, pscan_rpt_mode, pscan_mode, clk_offset);
	}
	else
	{
		/* update cache entry
		*/
		i->second->update(pscan_rpt_mode, pscan_mode, clk_offset);
	}
}

void HciDevice::finalize_cache()
{
	HciRemotePTable::iterator ri = _inquiry_cache.begin();
	while( ri != _inquiry_cache.end() )
	{
		if(ri->second->last_updated() < _time_last_inquiry)
		{
			delete ri->second;
			_inquiry_cache.erase(ri);
		}
		++ri;
	}
}

void HciDevice::clear_cache()
{
	HciRemotePTable::iterator ri = _inquiry_cache.begin();
	while( ri != _inquiry_cache.end() )
	{
		HciRemotePTable::iterator ti = ri;
		ti++;
	
		delete ri->second;
		_inquiry_cache.erase(ri);
		ri = ti;
	}
}

void HciDevice::on_hci_event( const Hci::EventPacket& evt, void* cookie, bool timedout )
{
	DBus::ReturnMessage* reply = (DBus::ReturnMessage *) cookie;

	if( timedout )
	{
		u16 error = 1;
		const char* estring = "Request timed out";
		reply->append( DBUS_TYPE_UINT16, &error,
			       DBUS_TYPE_STRING, &estring,
			       DBUS_TYPE_INVALID
		);
	}
	else
	{
		switch( evt.code )
		{
			case EVT_CMD_STATUS:
			{
				evt_cmd_status* cs = (evt_cmd_status*) evt.edata;

				if(!cs->status) goto nodel;

				char* error_string = strerror(bt_error(cs->status));
				reply->append( DBUS_TYPE_UINT16, &cs->status,
					       DBUS_TYPE_STRING, &error_string,
					       DBUS_TYPE_INVALID
				);
				break;
			}
			case EVT_CMD_COMPLETE:
			{
				handle_command_complete(evt,reply);
				break;
			}

			/*	command-specific events
			*/
			case EVT_INQUIRY_RESULT:
			{
				inquiry_info* r = (inquiry_info*) evt.edata;

				char straddr[18] = {0};

				ba2str(&(r->bdaddr), straddr);

				blue_dbg("Found remote device with address %s !",straddr);

				goto nosend;
			}
			case EVT_INQUIRY_COMPLETE:
			{
				finalize_cache();

				/* TODO: emit appropriate signals
				*/
				goto nosend;
			}
		}
	}

	conn().send(*reply);
nosend:	delete reply;
nodel:	return;
}

void HciDevice::handle_command_complete( const Hci::EventPacket& evt, DBus::ReturnMessage* reply )
{
	u16 error_status = 0;

	switch( evt.ogf )
	{
		case OGF_LINK_CTL:
		{
			switch( evt.ocf )
			{
				case OCF_INQUIRY_CANCEL:
				{
					finalize_cache();
					
					/* todo: emit appropriate signal
					*/
					break;
				}
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
				case OCF_READ_AUTH_ENABLE:
				case OCF_READ_ENCRYPT_MODE:
				{
					u8* data = (u8*) evt.edata;
					u16 status = 0;

					reply->append( DBUS_TYPE_UINT16, &(status),
						       DBUS_TYPE_BYTE,   (data),
						       DBUS_TYPE_INVALID
					);
					break;
				}
				case OCF_READ_INQUIRY_SCAN_TYPE:
				{
					read_inquiry_scan_type_rp* r = (read_inquiry_scan_type_rp*) evt.edata;

					if(r->status)
					{
						error_status = r->status;
						goto write_error;
					}

					reply->append( DBUS_TYPE_UINT16, &(r->status),
						       DBUS_TYPE_BYTE,   &(r->type),
						       DBUS_TYPE_INVALID
					);
					break;
				}
				case OCF_READ_LOCAL_NAME:
				{
					read_local_name_rp* r = (read_local_name_rp*) evt.edata;

					if(r->status)
					{
						error_status = r->status;
						goto write_error;
					}

					const char* local_name = (char*)r->name;

					blue_dbg("local name request returned '%s'",local_name);

					reply->append( DBUS_TYPE_UINT16, &(r->status),
						       DBUS_TYPE_STRING, &(local_name),
						       DBUS_TYPE_INVALID
					);
					break;
				}
				case OCF_READ_CLASS_OF_DEV:
				{
					read_class_of_dev_rp* r = (read_class_of_dev_rp*) evt.edata;

					if(r->status)
					{
						error_status = r->status;
						goto write_error;
					}

					/* I'm leaving this as a numerical value to allow clients
					   to translate the device properties in the local language
					*/
					reply->append( DBUS_TYPE_UINT16, &(r->status),
						       DBUS_TYPE_UINT32, &(r->dev_class),
						       DBUS_TYPE_INVALID
					);
					break;
				}
				case OCF_READ_VOICE_SETTING:
				{
					read_voice_setting_rp* r = (read_voice_setting_rp*) evt.edata;

					if(r->status)
					{
						error_status = r->status;
						goto write_error;
					}

					reply->append( DBUS_TYPE_UINT16, &(r->status),
						       DBUS_TYPE_UINT16, &(r->voice_setting),
						       DBUS_TYPE_INVALID
					);
					break;
				}
				case OCF_WRITE_INQUIRY_SCAN_TYPE:
				case OCF_CHANGE_LOCAL_NAME:
				{
					u16 status = 0;

					reply->append( DBUS_TYPE_UINT16, &(status),
						       DBUS_TYPE_INVALID
					);
					break;
				}
			}
			break;
		}
		case OGF_INFO_PARAM:
		{
			switch( evt.ocf )
			{
				case OCF_READ_BD_ADDR:
				{
					read_bd_addr_rp* r = (read_bd_addr_rp*) evt.edata;

					if(r->status)
					{
						error_status = r->status;
						goto write_error;
					}

					char local_addr[32] = {0};
					ba2str(&(r->bdaddr),local_addr);
					char* local_addr_ptr = local_addr;

					reply->append( DBUS_TYPE_UINT16, &(r->status),
						       DBUS_TYPE_STRING, &local_addr_ptr,
						       DBUS_TYPE_INVALID
					);
					break;
				}
				case OCF_READ_LOCAL_VERSION:
				{
					read_local_version_rp* r = (read_local_version_rp*) evt.edata;

					if(r->status)
					{
						error_status = r->status;
						goto write_error;
					}

					const char* hci_ver = hci_vertostr(r->hci_ver);
					const char* lmp_ver = lmp_vertostr(r->hci_ver);
					const char* comp_id = bt_compidtostr(r->manufacturer);

					reply->append( DBUS_TYPE_UINT16, &(r->status),
						       DBUS_TYPE_STRING, &(hci_ver),
						       DBUS_TYPE_UINT16, &(r->hci_rev),
						       DBUS_TYPE_STRING, &(lmp_ver),
						       DBUS_TYPE_UINT16, &(r->lmp_subver),
						       DBUS_TYPE_STRING, &comp_id,
						       DBUS_TYPE_INVALID
					);
					break;
				}
				case OCF_READ_LOCAL_FEATURES:
				{
					read_local_features_rp* r = (read_local_features_rp*) evt.edata;

					if(r->status)
					{
						error_status = r->status;
						goto write_error;
					}
					
					char* lmp_feat = lmp_featurestostr(r->features," ",0);

					reply->append( DBUS_TYPE_UINT16, &(r->status),
						       DBUS_TYPE_STRING, &(lmp_feat),
						       DBUS_TYPE_INVALID
					); //todo: format this field as a string array

					free(lmp_feat);
					break;
				}
			}
			break;
		}
		case OGF_STATUS_PARAM:
		{
			break;
		}
	}
	return;

write_error:

	char* error_string = strerror(bt_error(error_status));
	reply->append( DBUS_TYPE_UINT16, &error_status,
		       DBUS_TYPE_STRING,&error_string,
		       DBUS_TYPE_INVALID
	);
}

/*	remote device
*/

char __hci_remote_name[256];

const char* __remote_name( const HciDevice& parent, const BdAddr& addr )
{
	memset(__hci_remote_name, 0, 256);

	/* that's UGLY!
	*/
	strncpy(__hci_remote_name,
		(std::string(
			  parent.oname()
			+ DBUS_HCIREM_SUBPATH
			+ addr.to_string()
		)).c_str(),
		256
	);
	return __hci_remote_name;
}

HciRemote::HciRemote( HciDevice& parent, const BdAddr& addr, u8 pscan_rpt_mode, u8 pscan_mode, u16 clk_offset )
:	DBus::LocalInterface(DBUS_HCIREM_IFACE),
	DBus::LocalObject( __remote_name(parent, addr), DBus::Connection::SystemBus() ),
	_device(parent._device, addr, pscan_rpt_mode, pscan_mode, clk_offset)
	//_parent(parent)
{
	register_method( HciRemote, GetProperty );
	register_method( HciRemote, SetProperty );
}

void HciRemote::update( u8 pscan_rpt_mode, u8 pscan_mode, u16 clk_offset )
{
	_device.page_scan_repeat_mode(pscan_rpt_mode);
	_device.page_scan_mode(pscan_mode);
	_device.clock_offset(clk_offset);

	timeval now;
	gettimeofday(&now, NULL);

	_time_last_update = now.tv_sec*1000 + now.tv_usec/1000.0;	
}

double HciRemote::last_updated() const
{
	return _time_last_update;
}

void HciRemote::GetProperty( const DBus::CallMessage& msg )
{
	DBus::MessageIter i = msg.r_iter();
	std::string property = i.get_string();
	++i;

	blue_dbg("method GetProperty(%s) called on %s",property.c_str(),oname().c_str());

	//DBus::ReturnMessage *reply = new DBus::ReturnMessage(msg);
}

void HciRemote::SetProperty( const DBus::CallMessage& msg )
{
	DBus::MessageIter i = msg.r_iter();
	std::string property = i.get_string();
	++i;

	blue_dbg("method SetProperty(%s) called on %s",property.c_str(),oname().c_str());

	//DBus::ReturnMessage *reply = new DBus::ReturnMessage(msg);
}
