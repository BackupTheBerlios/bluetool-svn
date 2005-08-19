#include <signal.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/ioctl.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/bnep.h>

#include "pan_service.h"

PanService_i::PanService_i()
:	DBus::LocalInterface(BTOOL_SVC_ROOT_NAME PAN_SVC_NAME)
{
	register_method( PanService_i, ListConnections );
	register_method( PanService_i, KillConnection );
}

PanService::PanService
(
	const std::string& dbus_root,
	const std::string& conf_root,
	const BdAddr& addr
)
:	BluetoolService( dbus_root, conf_root, PAN_SVC_NAME ),
	_addr(addr),
	_pid(-1)
{
}

PanService::~PanService()
{
}

bool PanService::start_service()
{
	if(_pid >= 0)
	{
		_pid = vfork();
		if(_pid < 0)
		{
			return false;
		}
		else if (_pid == 0) //child
		{
			std::string sopt_i = " -i ";
			sopt_i += _addr.to_string();

			const char* copt_i = sopt_i.c_str();

			const char* const args[] = 
			{
				copt_i, NULL
			};

			int ret = execve
			(
				PAN_CMD,
				const_cast<char** const>(args),
				NULL
			);

			exit(ret);
		}
		else return true; //calling process
	}	
	return false;
}

bool PanService::stop_service()
{
	if(_pid >= 0 && kill(_pid, SIGTERM) >= 0)
	{
		_pid = -1;
		return true;
	}
	return false;
}

void PanService::ListConnections( const DBus::CallMessage& msg )
{
	/* this function was adapted from pand.c in Bluez 2.19
	*/
	DBus::ReturnMessage reply (msg);

	char* strerr = NULL;

	int ctl = socket(PF_BLUETOOTH, SOCK_RAW, BTPROTO_BNEP);
	if( ctl < 0 )
	{
		strerr = "Unable to open control connection";
		goto error;
	}

	bnep_connlist_req req;
	bnep_conninfo ci[48];

	req.cnum = 48;
	req.ci   = ci;
	if(ioctl(ctl, BNEPGETCONNLIST, &req))
	{
		strerr = "Unable to read connection list";
		goto error;
	}

	for( uint i = 0; i < req.cnum ; ++i)
	{
		char* nil = "";

		reply.append( DBUS_TYPE_STRING, &(ci[i].device),
			      DBUS_TYPE_STRING, &(ci[i].dst),
			      DBUS_TYPE_STRING, &nil
		);
		/* how can we translate ci.role into a string ???
		*/
	}
	reply.append(DBUS_TYPE_INVALID);
	conn().send(reply);
return;

error:	DBus::ErrorMessage emsg (msg, BTOOL_PAN_ERROR, strerr);
	conn().send(emsg);

return;
}

void PanService::KillConnection( const DBus::CallMessage& )
{
}
