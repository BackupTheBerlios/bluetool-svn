#ifndef __BTOOL_SDP_DATABASE_H
#define __BTOOL_SDP_DATABASE_H

#include "../../btool_service.h"
#include "../../../libbluetool/sdpsession.h"
#include "../../../cbus/cbus.h"

class SdpDatabase_i : DBus::LocalInterface
{
public:
	SdpDatabase_i();

protected:
	/*	interface methods
	*/
	virtual void AddRecord		( const DBus::CallMessage& ) = 0;
	virtual void RemoveRecord	( const DBus::CallMessage& ) = 0;
};

class SdpDatabase : private SdpDatabase_i, public BluetoolService
{
public:
	SdpDatabase();

public:
	/*	implement interface methods
	*/
	void AddRecord		( const DBus::CallMessage& );
	void RemoveRecord	( const DBus::CallMessage& );

private:
	Sdp::Session _local_sess;	//connection to the local database
};

#endif//__BTOOL_SDP_DATABASE_H
