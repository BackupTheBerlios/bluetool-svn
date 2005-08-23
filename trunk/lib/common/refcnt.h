	#ifndef __REF_CNT_H
#define __REF_CNT_H

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
		if(noref())	delete __ref;
		else		unref();
	}

protected:
	bool noref()
	{
		return (*__ref) == 1;
	}

private:
	void ref()
	{
		++ (*__ref);
	}
	void unref()
	{	// returns true when object can be finalized
	
		-- (*__ref);

		if( (*__ref) < 0 )
		{
			_dbg("refcount dropped below zero!!!!");
		}
	
	}
private:
	int *__ref;
};

#endif//__REF_CNT_H
