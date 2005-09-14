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

namespace Bluetool
{

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
	register_method( HciDevice, InquiryCache );

	/*	link with lower-level
	*/
	Hci::LocalDevice::data(this);
}

HciDevice::~HciDevice()
{
	/* XXX: this was commented, and I don't remember why
	*/
	clear_cache();
}

void HciDevice::Up( const DBus::CallMessage& msg )
{
	try
	{
		Hci::LocalDevice::up(id());

		u16 status = 0;
		DBus::ReturnMessage reply(msg);

		_bus.send(reply);
	}
	catch( Dbg::Error& e )
	{
		DBus::ErrorMessage err(msg, BTOOL_ERROR_HCI, e.what());
		_bus.send(err);
	}
}

void HciDevice::Down( const DBus::CallMessage& msg )
{
	try
	{
		Hci::LocalDevice::down(id());

		u16 status;

		DBus::ReturnMessage reply(msg);

		_bus.send(reply);
	}
	catch( Dbg::Error& e )
	{
		DBus::ErrorMessage err(msg, BTOOL_ERROR_HCI, e.what());
		_bus.send(err);
	}
}

/*	device properties management
*/

void HciDevice::GetProperty( const DBus::CallMessage& msg )
{
	DBus::ReturnMessage *reply = new DBus::ReturnMessage( msg );

	try
	{
		DBus::MessageIter i = msg.r_iter();
		std::string property = i.get_string();

		blue_dbg("method GetProperty(%s) called on hci%d",property.c_str(),id());

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
	if( property == "address" )
		Hci::LocalDevice::get_address(reply, HCI_TIMEOUT);
	else
	if( property == "name" )
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
	if( property == "link_mode" )
	{
		u32 lm;

		Hci::LocalDevice::get_link_mode(&lm);
		u16 status = 0;
		reply->append( DBUS_TYPE_UINT32, &lm,
			       DBUS_TYPE_INVALID
		);
		_bus.send(*reply);
		delete reply;
	}
	
	else
	if( property == "link_policy" )
	{
		u32 lp;

		Hci::LocalDevice::get_link_policy(&lp);
		u16 status = 0;
		reply->append( DBUS_TYPE_UINT32, &lp,
			       DBUS_TYPE_INVALID
		);
		_bus.send(*reply);
		delete reply;
	}

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
		reply->append( DBUS_TYPE_BOOLEAN, &status,
			       DBUS_TYPE_INT32, &rx_bytes,
			       DBUS_TYPE_INT32, &rx_errors,
			       DBUS_TYPE_INT32, &tx_bytes,
			       DBUS_TYPE_INT32, &tx_errors,
			       DBUS_TYPE_INVALID
		);
		_bus.send(*reply);
		delete reply;
	}
	
	else
	{
		DBus::ErrorMessage err(msg, BTOOL_ERROR_HCI, "No such property");
		_bus.send(err);

		delete reply;
	}
	
	}
	catch( Dbg::Error& e )
	{
		DBus::ErrorMessage err(msg, BTOOL_ERROR, e.what());
		_bus.send(err);

		delete reply;
	}
}

void HciDevice::SetProperty( const DBus::CallMessage& msg )
{
	DBus::ReturnMessage *reply = new DBus::ReturnMessage(msg);

	try
	{
		DBus::MessageIter i = msg.r_iter();
		std::string property = i.get_string();
		++i;
	
		blue_dbg("method SetProperty(%s) called on hci%d",property.c_str(),id());

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

	else
	if( property == "packet_type" )
	{
	}

	else
	if( property == "name" )
	{
		const char* name = i.get_string();
		Hci::LocalDevice::set_name(name, reply, HCI_TIMEOUT);
	}
	
	else
	if( property == "class" )
	{
		u8 major     = i.get_byte(); ++i;
		u8 minor     = i.get_byte(); ++i;
		u8 service   = i.get_byte();
		Hci::LocalDevice::set_class(major,minor,service,reply,HCI_TIMEOUT);
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
	if( property == "link_mode" )
	{
		u32 lm = i.get_uint32();

		Hci::LocalDevice::set_link_mode(lm);
		u16 status = 0;
		reply->append( DBUS_TYPE_INVALID
		);
		_bus.send(*reply);
		delete reply;
	}
	
	else
	if( property == "link_policy" )
	{
		u32 lp = i.get_uint32();

		Hci::LocalDevice::set_link_policy(lp);

		_bus.send(*reply);
		delete reply;
	}	
	else
	{
		DBus::ErrorMessage err(msg, BTOOL_ERROR_HCI, "No such property");
		_bus.send(err);

		delete reply;
	}
	
	}
	catch( Dbg::Error& e )
	{
		DBus::ErrorMessage err(msg, BTOOL_ERROR, e.what());
		_bus.send(err);

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

void HciDevice::InquiryCache( const DBus::CallMessage& msg )
{
	DBus::ReturnMessage reply (msg);
	DBus::MessageIter rw = reply.w_iter();
	DBus::MessageIter sa = rw.new_array(DBUS_TYPE_STRING);

	const Hci::RemoteDevPTable& inquiry_cache = Hci::LocalDevice::get_inquiry_cache();
	Hci::RemoteDevPTable::const_iterator i = inquiry_cache.begin();
	while( i != inquiry_cache.end() )
	{
		/* we CAN do that because they're actually the same thing
		*/
		const HciRemote* rem = (const HciRemote*)i->second->data();

		const char* fullname = rem->object()->oname().c_str();
		sa.append_string(fullname);
		++i;
	}
	rw.close_container(sa);
	reply.append(DBUS_TYPE_INVALID);
	_bus.send(reply);
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
	void* cookie,
	u8 auth
)
{
	DBus::ReturnMessage* reply = (DBus::ReturnMessage *) cookie;

	reply->append( DBUS_TYPE_BYTE,   &(auth),
		       DBUS_TYPE_INVALID
	);
}

void HciDevice::on_set_auth_enable
(
	void* cookie
)
{
	DBus::ReturnMessage* reply = (DBus::ReturnMessage *) cookie;
}

void HciDevice::on_get_encrypt_mode
(
	void* cookie,
	u8 encrypt
)
{
	DBus::ReturnMessage* reply = (DBus::ReturnMessage *) cookie;

	reply->append( DBUS_TYPE_BYTE,   &(encrypt),
		       DBUS_TYPE_INVALID
	);
}

void HciDevice::on_set_encrypt_mode
(
	void* cookie
)
{
}

void HciDevice::on_get_scan_enable
(
	void* cookie,
	u8 type
)
{
	DBus::ReturnMessage* reply = (DBus::ReturnMessage *) cookie;

	reply->append( DBUS_TYPE_BYTE,   &(type),
		       DBUS_TYPE_INVALID
	);
}

void HciDevice::on_set_scan_enable
(
	void* cookie
)
{
}

void HciDevice::on_get_name
(
	void* cookie,
	const char* name
)
{
	DBus::ReturnMessage* reply = (DBus::ReturnMessage *) cookie;

	reply->append( DBUS_TYPE_STRING, &(name),
		       DBUS_TYPE_INVALID
	);
}

void HciDevice::on_set_name
(
	void* cookie
)
{
}

void HciDevice::on_get_class
(
	void* cookie,
	u8* dev_class
)
{
	DBus::ReturnMessage* reply = (DBus::ReturnMessage *) cookie;

	reply->append( DBUS_TYPE_BYTE,   dev_class+0,
		       DBUS_TYPE_BYTE,   dev_class+1,
		       DBUS_TYPE_BYTE,   dev_class+2,
		       DBUS_TYPE_INVALID
	);
}

void HciDevice::on_set_class
(
	void* cookie
)
{
}

void HciDevice::on_get_voice_setting
(
	void* cookie,
	u16 setting
)
{
	DBus::ReturnMessage* reply = (DBus::ReturnMessage *) cookie;

	reply->append( DBUS_TYPE_UINT16, &(setting),
		       DBUS_TYPE_INVALID
	);
}

void HciDevice::on_set_voice_setting
(
	void* cookie
)
{
}

void HciDevice::on_get_address
(
	void* cookie,
	const char* address
)
{
	DBus::ReturnMessage* reply = (DBus::ReturnMessage *) cookie;

	reply->append( DBUS_TYPE_STRING, &(address),
		       DBUS_TYPE_INVALID
	);
}

void HciDevice::on_get_version
(
	void* cookie,
	const char* hci_ver,
	u16 hci_rev,
	const char* lmp_ver,
	u16 lmp_subver,
	const char* manufacturer
)
{
	DBus::ReturnMessage* reply = (DBus::ReturnMessage *) cookie;

	reply->append( DBUS_TYPE_STRING, &(hci_ver),
		       DBUS_TYPE_UINT16, &(hci_rev),
		       DBUS_TYPE_STRING, &(lmp_ver),
		       DBUS_TYPE_UINT16, &(lmp_subver),
		       DBUS_TYPE_STRING, &(manufacturer),
		       DBUS_TYPE_INVALID
	);
}

void HciDevice::on_get_features
(
	void* cookie,
	const char* features
)
{
	DBus::ReturnMessage* reply = (DBus::ReturnMessage *) cookie;

	reply->append( DBUS_TYPE_STRING, &(features),
		       DBUS_TYPE_INVALID
	);
}

void HciDevice::on_inquiry_complete
(
	void* cookie
)
{
}

void HciDevice::on_inquiry_cancel
(
	void* cookie
)
{
}

void HciDevice::on_periodic_inquiry_started
(
	void* cookie
)
{
}

void HciDevice::on_periodic_inquiry_cancel
(
	void* cookie
)
{
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
		emsg.destination(reply->destination());
		emsg.sender(reply->sender());

		_bus.send(emsg);
	}
	else
	{
		_bus.send(*reply);
	}
	delete reply;
}

/*	remote device
*/

HciRemote::HciRemote
(
	//const char* obj_name,
	Hci::LocalDevice* parent,
	Hci::RemoteInfo& info
)
:	Hci::RemoteDevice( parent, info ),
	DBus::LocalInterface( BTOOL_HCIREMOTE_IFACE ),
	_bus(DBus::Connection::SystemBus())
	//DBus::LocalObject( obj_name, DBus::Connection::SystemBus() )
{
	register_method( HciRemote, GetProperty );

	Hci::RemoteDevice::data(this);
}

HciRemote::~HciRemote()
{}

void HciRemote::GetProperty( const DBus::CallMessage& msg )
{
	DBus::ReturnMessage *reply = new DBus::ReturnMessage( msg );

	try
	{
		DBus::MessageIter i = msg.r_iter();
		std::string property = i.get_string();

		blue_dbg("method GetProperty(%s) called on %s",property.c_str(),iname().c_str());

		if( property == "name" )	
			Hci::RemoteDevice::get_name(reply, HCI_TIMEOUT);
		else
		if( property == "address" )	
			Hci::RemoteDevice::get_address(reply, HCI_TIMEOUT);
		else
		if( property == "class" )	
			Hci::RemoteDevice::get_class(reply, HCI_TIMEOUT);
		else
		{
			DBus::ErrorMessage err(msg, BTOOL_ERROR_HCI, "No such property");
			_bus.send(err);

			delete reply;
		}
	
	}
	catch( Dbg::Error& e )
	{
		DBus::ErrorMessage err(msg, BTOOL_ERROR, e.what());
		_bus.send(err);

		delete reply;
	}
}

void HciRemote::CreateConnection( const DBus::CallMessage& msg )
{
	DBus::ReturnMessage* reply = new DBus::ReturnMessage(msg);

//	Hci::RemoteDevice::create_connection(  );
}

void HciRemote::on_get_name
(	
	void* cookie,
	const char* name
)
{
	DBus::ReturnMessage* reply = (DBus::ReturnMessage *) cookie;

	reply->append( DBUS_TYPE_STRING, &(name),
		       DBUS_TYPE_INVALID
	);
}

void HciRemote::on_get_address
(	
	void* cookie,
	const char* addr
)
{
	DBus::ReturnMessage* reply = (DBus::ReturnMessage *) cookie;

	reply->append( DBUS_TYPE_STRING, &(addr),
		       DBUS_TYPE_INVALID
	);
}

void HciRemote::on_get_class
(
	void* cookie,
	u8* dev_class
)
{
	DBus::ReturnMessage* reply = (DBus::ReturnMessage *) cookie;

	reply->append( DBUS_TYPE_BYTE,   dev_class+0,
		       DBUS_TYPE_BYTE,   dev_class+1,
		       DBUS_TYPE_BYTE,   dev_class+2,
		       DBUS_TYPE_INVALID
	);
}

void HciRemote::on_get_version
(
	void* cookie,
	const char* lmp_ver,
	u16 lmp_subver,
	const char* manufacturer
)
{
	DBus::ReturnMessage* reply = (DBus::ReturnMessage *) cookie;

	reply->append( DBUS_TYPE_STRING, &(lmp_ver),
		       DBUS_TYPE_UINT16, &(lmp_subver),
		       DBUS_TYPE_STRING, &(manufacturer),
		       DBUS_TYPE_INVALID
	);
}

void HciRemote::on_get_features
(
	void* cookie,
	const char* features
)
{
	DBus::ReturnMessage* reply = (DBus::ReturnMessage *) cookie;

	reply->append( DBUS_TYPE_STRING, &(features),
		       DBUS_TYPE_INVALID
	);
}

void HciRemote::on_get_clock_offset
(
	void* cookie,
	u16 clock_offset
)
{
	DBus::ReturnMessage* reply = (DBus::ReturnMessage *) cookie;

	reply->append( DBUS_TYPE_UINT16, &(clock_offset),
		       DBUS_TYPE_INVALID
	);
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
#endif

HciConnection::HciConnection
(
	Hci::RemoteDevice* parent,
	Hci::ConnInfo& info
)
:	Hci::Connection( parent, info ),
	DBus::LocalInterface( BTOOL_HCICONN_IFACE ),
	_bus( DBus::Connection::SystemBus() )
{
	register_method( HciConnection, GetProperty );
	register_method( HciConnection, SetProperty );
}

HciConnection::~HciConnection()
{
}


void HciConnection::GetProperty( const DBus::CallMessage& msg )
{
	DBus::ReturnMessage *reply = new DBus::ReturnMessage( msg );

	try
	{
		DBus::MessageIter i = msg.r_iter();
		std::string property = i.get_string();

		if( property == "link_quality" )	
			Hci::Connection::get_link_quality(reply, HCI_TIMEOUT);
		else if( property == "rssi" )
			Hci::Connection::get_rssi(reply, HCI_TIMEOUT);
		
		else
		{
			DBus::ErrorMessage err(msg, BTOOL_ERROR_HCI, "No such property");
			_bus.send(err);
	
			delete reply;
		}
	
	}
	catch( Dbg::Error& e )
	{
		DBus::ErrorMessage err(msg, BTOOL_ERROR, e.what());
		_bus.send(err);

		delete reply;
	}
}

void HciConnection::SetProperty( const DBus::CallMessage& msg )
{
}

void HciConnection::on_get_link_quality
(
	void* cookie,
	u8 lq
)
{
	DBus::ReturnMessage* reply = (DBus::ReturnMessage *) cookie;

	reply->append( DBUS_TYPE_BYTE,   &(lq),
		       DBUS_TYPE_INVALID
	);
}

void HciConnection::on_get_rssi
(
	void* cookie,
	i8 rssi		
)
{
	DBus::ReturnMessage* reply = (DBus::ReturnMessage *) cookie;

	reply->append( DBUS_TYPE_BYTE,   &(rssi),
		       DBUS_TYPE_INVALID
	);
}

}//namespace Bluetool
