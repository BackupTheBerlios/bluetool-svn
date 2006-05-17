#ifndef __EEPLE_REF_PTR_H
#define __EEPLE_REF_PTR_H

//#define USE_BOOST_PTR

#ifdef USE_BOOST_PTR

#include <boost/shared_ptr.hpp>
#define RefPtr boost::shared_ptr 

#else

#include "debug.h"
#include "refcnt.h"

template <class T>
class RefPtr
{
public:
	RefPtr( T* ptr = NULL );

	~RefPtr();

	RefPtr& operator = (const RefPtr& ref)
	{
		if( this != &ref )
		{
			if(__cnt.one()) delete __ptr;

			__ptr = ref.__ptr;
			__cnt = ref.__cnt;
		}
		return *this;
	}

	T& operator *() const
	{
		return *__ptr;
	}


	T* operator ->() const
	{
		if(__cnt.noref()) return NULL;
		
		return __ptr;
	}

	T* get() const
	{
		if(__cnt.noref()) return NULL;
		
		return __ptr;
	}

private:
	T*     __ptr;
	RefCnt __cnt;
};

#endif//USE_BOOST_PTR

#endif//__EEPLE_REF_PTR_H
