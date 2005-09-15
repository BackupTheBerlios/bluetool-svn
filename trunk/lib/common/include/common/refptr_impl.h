#ifndef __REF_PTR_IMPL_H
#define __REF_PTR_IMPL_H

#include "refptr.h"

#ifndef USE_BOOST_PTR

template <class T>
RefPtr<T>::RefPtr( T* ptr )
: __ptr(ptr)
{}

template <class T>
RefPtr<T>::~RefPtr()
{
	if(__cnt.one()) delete __ptr;
}

#endif//USE_BOOST_PTR

#endif//__REF_PTR_IMPL_H
