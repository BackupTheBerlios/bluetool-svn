#ifndef __SDP_ATTRIBUTE_H
#define __SDP_ATTRIBUTE_H

#include "../common/types.h"

#include "sdperror.h"

#include <pair>
#include <map>
#include <string>

namespace Sdp
{
	class DataElement;

	class Attribute;

}//namespace Sdp

namespace Sdp
{

class DataElement
{
};

class Attribute : public std::pair<u16, DataElement>
{
public:
	u16 id()
	{
		return this->first;
	}

	DataElement& elem()
	{
		return this->second;
	}
};

}//namespace Sdp

#endif//__SDP_ATTRIBUTE_H
