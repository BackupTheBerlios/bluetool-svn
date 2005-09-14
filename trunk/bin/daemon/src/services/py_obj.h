#ifndef __PY_OBJ_H
#define __PY_OBJ_H

/*	simple class to automate the chore
	of reference counting on python objects
	( XXX: move it in a header on its own )
*/

namespace Py
{

class Obj
{
public:
	explicit Obj( PyObject* o ) : _o ( o )//, rc(1)
	{
		//blue_dbg("%p=)%d",_o,rc);
	}

	Obj( const Obj& o )
	{
		_o = o._o;
		//rc = o.rc;
		ref();
	}

	~Obj()
	{
		unref();
	}

	Obj& operator = ( PyObject* o )
	{
		unref();
		//rc = 1;
		_o = o;
		return *this;
	}

	Obj& operator = ( const Obj& o )
	{
		if( this != &o )
		{
			unref();
			_o = o._o;
			//rc = o.rc;
			ref();
		}
		return *this;
	}

	PyObject* operator * ()
	{
		return _o;
	}

	operator bool()
	{
		return _o != NULL;
	}

private:
	void ref()
	{
		//++rc;
		//blue_dbg("%p+)%d",_o,rc);
		Py_XINCREF( _o );
	}

	void unref()
	{
		//--rc;
		//blue_dbg("%p-)%d",_o,rc);
		Py_XDECREF( _o );
	}

private:
	PyObject* _o;
	//int rc;
};

}

#endif//__PY_OBJ_H
