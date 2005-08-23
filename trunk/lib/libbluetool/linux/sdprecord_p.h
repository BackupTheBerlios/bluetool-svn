#ifndef __SDP_RECORD_P_H
#define __SDP_RECORD_P_H

#include "../sdprecord.h"

#include <bluetooth/bluetooth.h>
#include <bluetooth/sdp.h>
#include <bluetooth/sdp_lib.h>

namespace Sdp
{

struct DataElement::Private
{
	sdp_data_t* elem;
	bool alloc;

	static sdp_data_t* new_seq( DataElementSeq& );
};


struct Record::Private
{
	sdp_record_t* rec;
	bool alloc;
};

}

#endif//__SDP_RECORD_P_H
