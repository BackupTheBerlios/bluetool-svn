#include <common/refptr_impl.h>
#include <bluetool/hcidebug.h>
#include <bluetool/hciconnection.h>
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
	hci_dbg("handle=%d, link type=%d, enc mode=%d",
		info.handle, info.link_type, info.encrypt_mode
	);
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
	u16* hdl = new u16;
	*hdl = handle();

	RefPtr<Request> req (new Request);

	req->hr.ogf    = OGF_STATUS_PARAM;
	req->hr.ocf    = OCF_READ_LINK_QUALITY;
	req->hr.cparam = hdl;
	req->hr.clen   = sizeof(u16);
	req->hr.rparam = NULL;
	req->hr.rlen   = READ_LINK_QUALITY_RP_SIZE;

	req->to->interval(timeout);
	req->cookie = cookie;

	_from->pvt->post_req(req);
}

void Connection::get_rssi( void* cookie, int timeout )
{
	u16* hdl = new u16;
	*hdl = handle();

	RefPtr<Request> req (new Request);

	req->hr.ogf    = OGF_STATUS_PARAM;
	req->hr.ocf    = OCF_READ_RSSI;
	req->hr.cparam = hdl;
	req->hr.clen   = sizeof(u16);
	req->hr.rparam = NULL;
	req->hr.rlen   = READ_RSSI_RP_SIZE;

	req->to->interval(timeout);
	req->cookie = cookie;

	_from->pvt->post_req(req);
}



}//namespace Hci
