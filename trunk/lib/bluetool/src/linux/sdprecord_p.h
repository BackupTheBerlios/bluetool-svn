#ifndef __SDP_RECORD_P_H
#define __SDP_RECORD_P_H

#include <bluetool/sdpdebug.h>
#include <bluetool/sdprecord.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/sdp.h>
#include <bluetooth/sdp_lib.h>

namespace Sdp
{

struct DataElement::Private
{
	sdp_data_t* elem;
	bool alloc;

	DataElementList list;	//used only by SEQ and ALT

	Private();
	~Private();

	bool is_seq();
	bool is_bool();
	bool is_string();
	bool is_uuid();
	bool is_u8();
	bool is_u16();
	bool is_u32();

	void fill( sdp_data_t* );
};


struct Record::Private
{
	sdp_record_t* rec;
	bool alloc;

	AttributeList attrs;

	~Private();
};

}

#endif//__SDP_RECORD_P_H
