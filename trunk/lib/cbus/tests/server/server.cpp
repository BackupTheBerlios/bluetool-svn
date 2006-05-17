#include "server.h"
#include <signal.h>


class EchoServer : public EchoInterface, 
	public DBus::LocalObject
{
public:
	EchoServer( DBus::Connection& conn ) : DBus::LocalObject("/org/test/cbus/EchoServer", conn)
	{}

	void Echo( const DBus::CallMessage& msg )
	{
		try
		{
			DBus::MessageIter r = msg.r_iter();

			DBus::ReturnMessage ret(msg);
			DBus::MessageIter w = ret.w_iter();

			const char* string = r.get_string();

			w.append_string(string);

			conn().send(ret);
		}
		catch( Dbg::Error& e )
		{
			DBus::ErrorMessage err(msg, "org.test.cbus.InvalidParameter", e.what());
			conn().send(err);
		}
	}
};


EventLoop loop;

void niam( int sig )
{
	loop.leave();
}

int main()
{
	signal(SIGTERM, niam);
	signal(SIGINT, niam);

	DBus::Connection conn = DBus::Connection::SessionBus();
	conn.request_name("org.test.cbus");

	EchoServer server(conn);

	loop.enter();
	return 0;
}
