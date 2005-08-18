//#include <bluetooth/bluetooth.h>
//#include <bluetooth/hci.h>
//#include <bluetooth/hci_lib.h>

#include "../bluedebug.h"

#include "../btool_names.h"
#include "../btool_device.h"

#include "hci_device.h"

#include <map>

#include <cstring>
#include <cstdio>

HciDevice::HciDevice( int dev_id )
:
	Hci::LocalDevice	( dev_id ),
	DBus::LocalInterface	( BTOOL_DEVICE_ROOT_NAME "hci" ),

	_bus(DBus::Connection::SystemBus())
{
	/*	export methods
	*/
	register_method( HciDevice, Up );
	register_method( HciDevice, Down );
	register_method( HciDevice, GetProperty );
	register_method( HciDevice, SetProperty );
	register_method( HciDevice, StartInquiry );
	register_method( HciDevice, CancelInquiry );
}

HciDevice::~HciDevice()
{
	//clear_cache();
}

void HciDevice::Up( const DBus::CallMessage& msg )
{
	u16 status;
	DBus::ReturnMessage reply(msg);
	try
	{
		Hci::LocalDevice::up();

		status = 0;
		reply.append( DBUS_TYPE_UINT16, &status,
			      DBUS_TYPE_INVALID
		);
	}
	catch(Hci::Exception& e)
	{
		status = 1;
		const char* strerr = e.what();

		reply.append( DBUS_TYPE_UINT16, &status,
			      DBUS_TYPE_STRING, &strerr,
			      DBUS_TYPE_INVALID
		);
	}
	_bus.send(reply);
}

void HciDevice::Down( const DBus::CallMessage& msg )
{
	u16 status;
	DBus::ReturnMessage reply(msg);
	try
	{
		Hci::LocalDevice::down();

		status = 0;
		reply.append( DBUS_TYPE_UINT16, &status,
			       DBUS_TYPE_INVALID
		);
	}
	catch(Hci::Exception& e)
	{
		status = 1;
		const char* strerr = e.what();

		reply.append( DBUS_TYPE_UINT16, &status,
			      DBUS_TYPE_STRING, &strerr,
			      DBUS_TYPE_INVALID
		);
	}
	_bus.send(reply);
}

/*	device properties management
*/

void HciDevice::GetProperty( const DBus::CallMessage& msg )
{
	DBus::MessageIter i = msg.r_iter();
	std::string property = i.get_string();

	blue_dbg("method GetProperty(%s) called on hci%d",property.c_str(),id());

	DBus::ReturnMessage *reply = new DBus::ReturnMessage( msg );

	try
	{

	if( property == "auth_enable" )	
		Hci::LocalDevice::get_auth_enable(reply, HCI_TIMEOUT);
	else
	if( property == "encrypt_mode" )
		Hci::LocalDevice::get_encrypt_mode(reply, HCI_TIMEOUT);
	else
//	if( property == "security_manager_enable" )
//		Hci::LocalDevice::get_secman_enable();
//	else
	if( property == "scan_enable" )
		Hci::LocalDevice::get_scan_enable(reply, HCI_TIMEOUT);
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
		Hci::LocalDevice::get_address(reply, HCI_TIMEOUT);
	else
	if( property == "local_name" )
		Hci::LocalDevice::get_name(reply, HCI_TIMEOUT);
	else
	if( property == "class" )
		Hci::LocalDevice::get_class(reply, HCI_TIMEOUT);
	else
	if( property == "voice_setting" )
		Hci::LocalDevice::get_voice_setting(reply, HCI_TIMEOUT);
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
		Hci::LocalDevice::get_features(reply,HCI_TIMEOUT);
	else
	if( property == "version_info" )
		Hci::LocalDevice::get_version(reply,HCI_TIMEOUT);
	else
	if( property == "revision_info" )
	{
	}
	/*	synchronous methods
		(they're available only as ioctls which cannot be unblocked)
	*/
	else
	if( property == "stats" )
	{
		int status;
		int rx_bytes;
		int rx_errors;
		int tx_bytes;
		int tx_errors;
		Hci::LocalDevice::get_stats
		(
			&status,&rx_bytes,&rx_errors,&tx_bytes,&rx_errors
		);
		u16 err = 0;
		reply->append( DBUS_TYPE_UINT16, &err,
			       DBUS_TYPE_BOOLEAN, &status,
			       DBUS_TYPE_INT32, &rx_bytes,
			       DBUS_TYPE_INT32, &rx_errors,
			       DBUS_TYPE_INT32, &tx_bytes,
			       DBUS_TYPE_INT32, &tx_errors,
			       DBUS_TYPE_INVALID
		);
		_bus.send(*reply);
		delete reply;	}
	
	else
	{
		u16 err = 1;
		const char* strerr = "No such property";

		reply->append( DBUS_TYPE_UINT16, &err,
			       DBUS_TYPE_STRING, &strerr,
			       DBUS_TYPE_INVALID
		);
		_bus.send(*reply);
		delete reply;
	}
	
	}
	catch( std::exception& e )
	{
		u16 err = 1;
		const char* strerr = e.what();

		reply->append( DBUS_TYPE_UINT16, &err,
			       DBUS_TYPE_STRING, &strerr,
			       DBUS_TYPE_INVALID
		);
		_bus.send(*reply);
		delete reply;
	}
}

void HciDevice::SetProperty( const DBus::CallMessage& msg )
{
	DBus::MessageIter i = msg.r_iter();
	std::string property = i.get_string();
	++i;

	blue_dbg("method SetProperty(%s) called on hci%d",property.c_str(),id());

	DBus::ReturnMessage *reply = new DBus::ReturnMessage(msg);

	try
	{

	if( property == "auth_enable" )
	{
		u8 enable = i.get_byte();
		Hci::LocalDevice::set_auth_enable(enable,reply,HCI_TIMEOUT);	
	}
	
	else
	if( property == "encrypt_mode" )
	{
		u8 mode = i.get_byte();
		Hci::LocalDevice::set_encrypt_mode(mode,reply,HCI_TIMEOUT);
	}
	
//	else
//	if( property == "security_manager_enable" )
//	{
//		bool enable = i.get_bool();
//		Hci::LocalDevice::set_secman_enable(enable,reply);
//	}
	
	else
	if( property == "scan_enable" )
	{
		u8 type = i.get_byte();
		Hci::LocalDevice::set_scan_enable(type,reply,HCI_TIMEOUT);
	}
	
//	else
//	if( property == "inquiry_scan_enable" )
//	{
	//	bool enable = i.get_bool();
	//	Hci::LocalDevice::set_iscan_enable(enable, reply);	
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
		Hci::LocalDevice::set_name(name, reply, HCI_TIMEOUT);
	}
	
	else
	if( property == "class" )
	{
		u8 major = i.get_byte(); ++i;
		u8 minor = i.get_byte(); ++i;
		u8 ver   = i.get_byte();
		Hci::LocalDevice::set_class(major,minor,ver,reply,HCI_TIMEOUT);
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

		blue_dbg("error: %s", strerr);

		reply->append( DBUS_TYPE_UINT16, &err,
			       DBUS_TYPE_STRING, &strerr,
			       DBUS_TYPE_INVALID
		);
		_bus.send(*reply);
		delete reply;
	}
	
	}
	catch( std::exception& e )
	{
		u16 err = 1;
		const char* strerr = e.what();

		blue_dbg("error: %s", strerr);

		reply->append( DBUS_TYPE_UINT16, &err,
			       DBUS_TYPE_STRING, &strerr,
			       DBUS_TYPE_INVALID
		);
		_bus.send(*reply);
		delete reply;
	}
}

void HciDevice::StartInquiry( const DBus::CallMessage& msg )
{
	DBus::ReturnMessage* reply = new DBus::ReturnMessage (msg);

	Hci::LocalDevice::start_inquiry(NULL, 0/*IREQ_CACHE_FLUSH*/, reply);
}

void HciDevice::CancelInquiry( const DBus::CallMessage& msg )
{
	DBus::ReturnMessage* reply = new DBus::ReturnMessage (msg);

	Hci::LocalDevice::cancel_inquiry(reply, HCI_TIMEOUT);
}

/*	signals
*/

void HciDevice::DeviceInRange( const HciRemote& hr )
{
	/* todo: I don't like this way of sending signals
	*/
	DBus::SignalMessage sig
	(
		BTOOL_DEVMAN_PATH, //TODO
		iname().c_str(),
		"DeviceInRange"
	);

	const char* name = "b00rp";//hr.oname().c_str();

	sig.append
	(
		DBUS_TYPE_STRING, &(name),
		DBUS_TYPE_INVALID
	);

	_bus.send(sig);
}

void HciDevice::DeviceOutOfRange( const HciRemote& hr )
{
	/* todo: I don't like this way of sending signals
	*/
	DBus::SignalMessage sig
	(
		BTOOL_DEVMAN_PATH, //TODO
		iname().c_str(),
		"DeviceOutOfRange"
	);

	const char* name = "b00rp";//hr.oname().c_str();

	sig.append
	(
		DBUS_TYPE_STRING, &(name),
		DBUS_TYPE_INVALID
	);

	_bus.send(sig);
}

#if 0
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
#endif
void HciDevice::on_get_auth_enable
(
	u16 status,
	void* cookie,
	u8 auth
)
{
	DBus::ReturnMessage* reply = (DBus::ReturnMessage *) cookie;

	reply->append( DBUS_TYPE_UINT16, &(status),
		       DBUS_TYPE_BYTE,   &(auth),
		       DBUS_TYPE_INVALID
	);
}

void HciDevice::on_set_auth_enable
(
	u16 status,
	void* cookie
)
{
	DBus::ReturnMessage* reply = (DBus::ReturnMessage *) cookie;

	reply->append( DBUS_TYPE_UINT16, &(status),
		       DBUS_TYPE_INVALID
	);
}

void HciDevice::on_get_encrypt_mode
(
	u16 status,
	void* cookie,
	u8 encrypt
)
{
	DBus::ReturnMessage* reply = (DBus::ReturnMessage *) cookie;

	reply->append( DBUS_TYPE_UINT16, &(status),
		       DBUS_TYPE_BYTE,   &(encrypt),
		       DBUS_TYPE_INVALID
	);
}

void HciDevice::on_set_encrypt_mode
(
	u16 status,
	void* cookie
)
{
	DBus::ReturnMessage* reply = (DBus::ReturnMessage *) cookie;

	reply->append( DBUS_TYPE_UINT16, &(status),
		       DBUS_TYPE_INVALID
	);
}

void HciDevice::on_get_scan_enable
(
	u16 status,
	void* cookie,
	u8 type
)
{
	DBus::ReturnMessage* reply = (DBus::ReturnMessage *) cookie;

	reply->append( DBUS_TYPE_UINT16, &(status),
		       DBUS_TYPE_BYTE,   &(type),
		       DBUS_TYPE_INVALID
	);
}

void HciDevice::on_set_scan_enable
(
	u16 status,
	void* cookie
)
{
	DBus::ReturnMessage* reply = (DBus::ReturnMessage *) cookie;

	reply->append( DBUS_TYPE_UINT16, &(status),
		       DBUS_TYPE_INVALID
	);
}

void HciDevice::on_get_name
(
	u16 status,
	void* cookie,
	const char* name
)
{
	DBus::ReturnMessage* reply = (DBus::ReturnMessage *) cookie;

	reply->append( DBUS_TYPE_UINT16, &(status),
		       DBUS_TYPE_STRING, &(name),
		       DBUS_TYPE_INVALID
	);
}

void HciDevice::on_set_name
(
	u16 status,
	void* cookie
)
{
	DBus::ReturnMessage* reply = (DBus::ReturnMessage *) cookie;

	reply->append( DBUS_TYPE_UINT16, &(status),
		       DBUS_TYPE_INVALID
	);
}

void HciDevice::on_get_class
(
	u16 status,
	void* cookie,
	u8* dev_class
)
{
	DBus::ReturnMessage* reply = (DBus::ReturnMessage *) cookie;

	reply->append( DBUS_TYPE_UINT16, &(status),
		       DBUS_TYPE_BYTE,   dev_class+0,
		       DBUS_TYPE_BYTE,   dev_class+1,
		       DBUS_TYPE_BYTE,   dev_class+2,
		       DBUS_TYPE_INVALID
	);
}

void HciDevice::on_set_class
(
	u16 status,
	void* cookie
)
{
	DBus::ReturnMessage* reply = (DBus::ReturnMessage *) cookie;

	reply->append( DBUS_TYPE_UINT16, &(status),
		       DBUS_TYPE_INVALID
	);
}

void HciDevice::on_get_voice_setting
(
	u16 status,
	void* cookie,
	u16 setting
)
{
	DBus::ReturnMessage* reply = (DBus::ReturnMessage *) cookie;

	reply->append( DBUS_TYPE_UINT16, &(status),
		       DBUS_TYPE_UINT16, &(setting),
		       DBUS_TYPE_INVALID
	);
}

void HciDevice::on_set_voice_setting
(
	u16 status,
	void* cookie
)
{
	DBus::ReturnMessage* reply = (DBus::ReturnMessage *) cookie;

	reply->append( DBUS_TYPE_UINT16, &(status),
		       DBUS_TYPE_INVALID
	);
}

void HciDevice::on_get_address
(
	u16 status,
	void* cookie,
	const char* address
)
{
	DBus::ReturnMessage* reply = (DBus::ReturnMessage *) cookie;

	reply->append( DBUS_TYPE_UINT16, &(status),
		       DBUS_TYPE_STRING, &(address),
		       DBUS_TYPE_INVALID
	);
}

void HciDevice::on_get_version
(
	u16 status,
	void* cookie,
	const char* hci_ver,
	u16 hci_rev,
	const char* lmp_ver,
	u16 lmp_subver,
	const char* manufacturer
)
{
	DBus::ReturnMessage* reply = (DBus::ReturnMessage *) cookie;

	reply->append( DBUS_TYPE_UINT16, &(status),
		       DBUS_TYPE_STRING, &(hci_ver),
		       DBUS_TYPE_UINT16, &(hci_rev),
		       DBUS_TYPE_STRING, &(lmp_ver),
		       DBUS_TYPE_UINT16, &(lmp_subver),
		       DBUS_TYPE_STRING, &(manufacturer),
		       DBUS_TYPE_INVALID
	);
}

void HciDevice::on_get_features
(
	u16 status,
	void* cookie,
	const char* features
)
{
	DBus::ReturnMessage* reply = (DBus::ReturnMessage *) cookie;

	reply->append( DBUS_TYPE_UINT16, &(status),
		       DBUS_TYPE_STRING, &(features),
		       DBUS_TYPE_INVALID
	);
}

void HciDevice::on_inquiry_complete
(
	u16 status,
	void* cookie
)
{
	DBus::ReturnMessage* reply = (DBus::ReturnMessage *) cookie;

	reply->append( DBUS_TYPE_UINT16, &(status),
		       DBUS_TYPE_INVALID
	);
}

void HciDevice::on_inquiry_cancel
(
	u16 status,
	void* cookie
)
{
	DBus::ReturnMessage* reply = (DBus::ReturnMessage *) cookie;

	reply->append( DBUS_TYPE_UINT16, &(status),
		       DBUS_TYPE_INVALID
	);
}

void HciDevice::on_periodic_inquiry_started
(
	u16 status,
	void* cookie
)
{
	DBus::ReturnMessage* reply = (DBus::ReturnMessage *) cookie;

	reply->append( DBUS_TYPE_UINT16, &(status),
		       DBUS_TYPE_INVALID
	);
}

void HciDevice::on_periodic_inquiry_cancel
(
	u16 status,
	void* cookie
)
{
	DBus::ReturnMessage* reply = (DBus::ReturnMessage *) cookie;

	reply->append( DBUS_TYPE_UINT16, &(status),
		       DBUS_TYPE_INVALID
	);
}

/*	special handlers
*/
void HciDevice::on_after_event( int error, void* cookie )
{
	DBus::ReturnMessage* reply = (DBus::ReturnMessage *) cookie;
	if(error)
	{
		char* strerr = strerror(error);

		DBus::ErrorMessage emsg;
		emsg.name(BTOOL_ERROR_HCI);
		emsg.append(DBUS_TYPE_STRING, &strerr, DBUS_TYPE_INVALID);
		emsg.reply_serial(reply->reply_serial());

		_bus.send(emsg);
	}
	else
	{
		_bus.send(*reply);
	}
	delete reply;
}
#if 0
Hci::RemoteDevice* HciDevice::on_new_cache_entry( Hci::RemoteInfo& info )
{
	/* bluetooth addresses contain colons, which are not 
	   allowed in the DBUS specification
	*/
	std::string valid_addr = info.addr.to_string();
	for( uint i = 0; i < valid_addr.length(); ++i)
	{
		if(valid_addr[i] == ':')
			valid_addr[i] = '_';
	}
	std::string path = Bluetool::Device::generate_dev_path(addr());
	std::string rem_name = path + BTOOL_REM_SUBDIR + valid_addr;

	HciRemote* hr = new HciRemote(rem_name.c_str(),this,info);

	this->DeviceInRange(*hr);

	return hr;
}
#endif
/*	remote device
*/

HciRemote::HciRemote
(
	//const char* obj_name,
	Hci::LocalDevice* parent,
	Hci::RemoteInfo& info
)
:	Hci::RemoteDevice( parent, info ),
	DBus::LocalInterface( BTOOL_REM_IFACE ),
	_bus(DBus::Connection::SystemBus())
	//DBus::LocalObject( obj_name, DBus::Connection::SystemBus() )
{
	register_method( HciRemote, GetProperty );
	register_method( HciRemote, SetProperty );
}

HciRemote::~HciRemote()
{}

void HciRemote::GetProperty( const DBus::CallMessage& msg )
{
}


void HciRemote::SetProperty( const DBus::CallMessage& msg )
{
}

void HciRemote::CreateConnection( const DBus::CallMessage& msg )
{
	DBus::ReturnMessage* reply = new DBus::ReturnMessage(msg);

//	Hci::RemoteDevice::create_connection(  );
}

void HciRemote::on_get_name
(	
	u16 status,
	void* cookie,
	const char* name
)
{
}

void HciRemote::on_get_version
(
	u16 status,
	void* cookie,
	const char* lmp_ver,
	u16 lmp_sub,
	const char* manufacturer
)
{
}

void HciRemote::on_get_features
(
	u16 status,
	void* cookie,
	const char* features
)
{
}

void HciRemote::on_get_clock_offset
(
	u16 status,
	void* cookie,
	u16 clock_offset
)
{
}


Hci::Connection* HciRemote::on_new_connection( Hci::ConnInfo& ci )
{
	char path[256];
//	snprintf(path,sizeof(path),"%s"BTOOL_CONN_SUBDIR"0x%04X",oname().c_str(),ci.handle);

//	HciConnection* c = new HciConnection(path, this, ci);

	return NULL;
}


#if 0
void HciRemote::update( u8 pscan_rpt_mode, u8 pscan_mode, u16 clk_offset )
{
	Hci::LocalDevice::page_scan_repeat_mode(pscan_rpt_mode);
	Hci::LocalDevice::page_scan_mode(pscan_mode);
	Hci::LocalDevice::clock_offset(clk_offset);

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

	DBus::ReturnMessage *reply = new DBus::ReturnMessage(msg);

	try
	{

//	if( property == "auth_enable" )	
//		Hci::LocalDevice::get_auth_enable(reply, HCI_TIMEOUT);
//	else
	{
		u16 err = 1;
		const char* strerr = "No such property";

		reply->append( DBUS_TYPE_UINT16, &err,
			       DBUS_TYPE_STRING, &strerr,
			       DBUS_TYPE_INVALID
		);
		_bus.send(*reply);
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
		_bus.send(*reply);
		delete reply;
	}
}

void HciRemote::SetProperty( const DBus::CallMessage& msg )
{
	DBus::MessageIter i = msg.r_iter();
	std::string property = i.get_string();
	++i;

	blue_dbg("method SetProperty(%s) called on %s",property.c_str(),oname().c_str());

	DBus::ReturnMessage *reply = new DBus::ReturnMessage(msg);

	try
	{

//	if( property == "auth_enable" )	
//		Hci::LocalDevice::get_auth_enable(reply, HCI_TIMEOUT);
//	else
	{
		u16 err = 1;
		const char* strerr = "No such property";

		reply->append( DBUS_TYPE_UINT16, &err,
			       DBUS_TYPE_STRING, &strerr,
			       DBUS_TYPE_INVALID
		);
		_bus.send(*reply);
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
		_bus.send(*reply);
		delete reply;
	}
}
#endif


HciConnection::HciConnection
(
	const char* obj_name,
	Hci::RemoteDevice* parent,
	Hci::ConnInfo& info
)
:	Hci::Connection( parent, info ),
	DBus::LocalInterface( BTOOL_HCICONN_IFACE ),
	DBus::LocalObject( obj_name, DBus::Connection::SystemBus() )
{
	register_method( HciConnection, GetProperty );
	register_method( HciConnection, SetProperty );
}

HciConnection::~HciConnection()
{
}


void HciConnection::GetProperty( const DBus::CallMessage& msg )
{
}

void HciConnection::SetProperty( const DBus::CallMessage& msg )
{
}
