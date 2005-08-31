#include <bluetool/sdpdebug.h>
#include <errno.h>

#include "sdp_browser.h"

namespace Bluetool
{

SdpBrowser::SdpBrowser( const BdAddr& from, const BdAddr& to )
:	DBus::LocalInterface(BTOOL_SDPBROWSER_IFACE),
	Sdp::Client(from, to),
	_bus(DBus::Connection::SystemBus()),
	_reply(NULL)
{
	register_method( SdpBrowser, SearchServices );
	register_method( SdpBrowser, SearchAttributes );
	register_method( SdpBrowser, SearchServAttrs );
	register_method( SdpBrowser, SearchAllRecords );
	register_method( SdpBrowser, SearchAllRecordsCached );

	//this->on_response.connect( sigc::mem_fun( this, &SdpBrowser::on_read_response ));
}

SdpBrowser::~SdpBrowser()
{
	SdpRecordPTable::iterator it = _records.begin();
	
	while( it != _records.end() )
	{
		delete it->second;
		++it;
	}
}

void SdpBrowser::SearchServices( const DBus::CallMessage& msg )
{
	if(_reply)
	{
		DBus::ErrorMessage e (msg, BTOOL_ERROR, strerror(EBUSY));
		_bus.send(e);
		return;
	}
}

void SdpBrowser::SearchAttributes( const DBus::CallMessage& msg )
{
	if(_reply)
	{
		DBus::ErrorMessage e (msg, BTOOL_ERROR, strerror(EBUSY));
		_bus.send(e);
		return;
	}
}

void SdpBrowser::SearchServAttrs( const DBus::CallMessage& msg )
{
	if(_reply)
	{
		DBus::ErrorMessage e (msg, BTOOL_ERROR, strerror(EBUSY));
		_bus.send(e);
		return;
	}
}

void SdpBrowser::SearchAllRecords( const DBus::CallMessage& msg )
{	
	if(_reply)
	{
		DBus::ErrorMessage e (msg, BTOOL_ERROR, strerror(EBUSY));
		_bus.send(e);
		return;
	}

	_reply = new DBus::ReturnMessage(msg);

	Sdp::Client::start_complete_search();
}

void SdpBrowser::SearchAllRecordsCached( const DBus::CallMessage& msg )
{
	if(_reply)
	{
		DBus::ErrorMessage e (msg, BTOOL_ERROR, strerror(EBUSY));
		_bus.send(e);
		return;
	}

	_reply = new DBus::ReturnMessage(msg);

	Sdp::Client::cached_complete_search();
}

void SdpBrowser::on_read_response( u16 status, const Sdp::RecordList& data )
{
	DBus::MessageIter rw = _reply->w_iter();

	rw.append_uint16(status);

	DBus::MessageIter sa = rw.new_array(DBUS_TYPE_STRING);

	Sdp::RecordList::const_iterator rit = data.begin();
	while( rit != data.end() )
	{
		sa.append_string( _records[rit->handle()]->oname().c_str() );

		++rit;
	}

	rw.close_container(sa);
	_reply->append(DBUS_TYPE_INVALID);

	_bus.send(*_reply);
	delete _reply;
	_reply = NULL;
}


void SdpBrowser::on_new_cache_entry( Sdp::Record& rec )
{
	sdp_dbg_enter();

	/*	the record list owns a reference to the record now
	*/
	SdpRecordPTable::iterator rip = _records.find(rec.handle());

	if( rip == _records.end() )
	{
		_records[rec.handle()] = new SdpRecord(object()->oname().c_str(), rec);
	}
	else
	{
		delete rip->second;
		rip->second = new SdpRecord(object()->oname().c_str(), rec);
	}
	sdp_dbg_leave();
}

void SdpBrowser::on_purge_cache_entry( Sdp::Record& rec )
{
	sdp_dbg_enter();

	/*	we allocated a dbus object and acquired 	
		a reference to the record before, now release both
	*/
	SdpRecordPTable::iterator rip = _records.find(rec.handle());

	if( rip != _records.end() )
	{
		delete rip->second;
		_records.erase(rip);
	}
	sdp_dbg_leave();
}

}//namespace Bluetool
