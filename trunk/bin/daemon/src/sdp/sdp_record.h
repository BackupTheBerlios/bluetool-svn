#ifndef __BTOOL_SDP_RECORD_H
#define __BTOOL_SDP_RECORD_H

#include <map>
#include <cbus/cbus.h>
#include <common/refptr.h>
#include <bluetool/sdprecord.h>
#include <bluetool/sdperror.h>

#include "../btool_names.h"

namespace Bluetool
{

class SdpRecord;

typedef std::map<u32, SdpRecord* > SdpRecordPTable;

}

namespace Bluetool
{

class SdpRecord : public Sdp::Record, public DBus::LocalInterface, public DBus::LocalObject
{
public:
	SdpRecord( const char*, const Sdp::Record& );

	/* methods
	*/
	void GetAttribute( const DBus::CallMessage& );

	void GetHandle( const DBus::CallMessage& );

	void GetGroupId( const DBus::CallMessage& );

	void GetServiceId( const DBus::CallMessage& );

	void GetServiceName( const DBus::CallMessage& );

	void GetServiceDescription( const DBus::CallMessage& );

	void GetProviderName( const DBus::CallMessage& );

	void GetDocUrl( const DBus::CallMessage& );

	void GetClassIdList( const DBus::CallMessage& );

	void GetProtocolDescList( const DBus::CallMessage& );
};

}//namespace Bluetool

#endif//__BTOOL_SDP_RECORD_H
