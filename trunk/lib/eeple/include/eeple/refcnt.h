#ifndef __EEPLE_REF_CNT_H
#define __EEPLE_REF_CNT_H

#include "debug.h"

/* implements VERY simple reference counting
*/

class RefCnt
{
public:
	RefCnt()
	{
		__ref = new int;
		(*__ref) = 1;
	}

	
	RefCnt( const RefCnt& rc )
	{
		__ref = rc.__ref;
		ref();
	}

	virtual ~RefCnt()
	{
		unref();
	}

	RefCnt& operator = ( const RefCnt& ref )
	{
		ref.ref();
		unref();
		__ref = ref.__ref;
		return *this;
	}

	bool noref() const
	{
		return (*__ref) == 0;
	}

	bool one() const
	{
		return (*__ref) == 1;
	}

private:
	void ref() const
	{
		++ (*__ref);	//_dbg("%p+)%d",__ref,*__ref);
	}
	void unref() const
	{
		-- (*__ref);

		if( (*__ref) < 0 )
		{
			//_dbg("refcount dropped below zero!!!!"); //should throw stg here
		}

		//_dbg("%p-)%d",__ref,*__ref);

		if( noref() )
		{
			delete __ref;
		}
	}
private:
	int *__ref;
};

#endif//__EEPLE_REF_CNT_H
