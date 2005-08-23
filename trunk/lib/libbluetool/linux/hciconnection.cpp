#include "../hcidebug.h"
#include "../hciconnection.h"
#include "hcidevice_p.h"

namespace Hci
{

Connection::Connection
(
	RemoteDevice* to,
	ConnInfo& info
)
:	_from(to->local()),
	_to(to),
	_info(info)
{
}

Connection::~Connection()
{
/*	we don't care about disconnection
	we should be already disconnected
	when reaching this point
*/
}

void Connection::get_link_quality( void* cookie, int timeout )
{
	Request* req = new Request;
	req->hr.ogf    = OGF_INFO_PARAM;
	req->hr.ocf    = OCF_READ_LINK_QUALITY;
	req->hr.rparam = NULL;
	req->hr.rlen   = READ_LINK_QUALITY_RP_SIZE;

	req->to.interval(timeout);
	req->cookie = cookie;

	_from->pvt->post_req(req);
}

void Connection::get_rssi( void* cookie, int timeout )
{
	Request* req = new Request;
	req->hr.ogf    = OGF_INFO_PARAM;
	req->hr.ocf    = OCF_READ_RSSI;
	req->hr.rparam = NULL;
	req->hr.rlen   = READ_RSSI_RP_SIZE;

	req->to.interval(timeout);
	req->cookie = cookie;

	_from->pvt->post_req(req);
}



}//namespace Hci
