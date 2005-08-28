#ifndef __HCI_EVENT_H
#define __HCI_EVENT_H

#include <sigc++/sigc++.h>
#include <common/types.h>

namespace Hci
{

/*enum EventType
{
;;TODO
};*/

struct EventPacket;

typedef sigc::signal<void, EventPacket&, void*, bool> Event;

//typedef u16 bstatus;
//typedef u16 hcihandle;
//typedef u8 baddr[6];

/*	HCI Event Packet
*/

struct EventPacket
{
u16	status;
u8	code;
u8	ogf;
u8	ocf;
void*	edata;

#if 0
union 
{
	struct __PACKED
	{
		//TODO
	} inquiry_result;

	struct __PACKED
	{
		bstatus 	status;
		hcihandle	handle;
		baddr		addr;
		u8		link_type;
		u8		encryption_mode;
	} connection_complete;

	struct __PACKED
	{
		baddr  	addr;
		u8         	dev_class[3];
		u8		link_type;
	} connection_request;
	

	struct __PACKED
	{
		bstatus 	status;
		hcihandle	handle;
		u8		reason;
	} disconnection;

	struct __PACKED
	{
		bstatus 	status;
		hcihandle	handle;
	} auth_complete;
		

	struct __PACKED
	{
		bstatus 	status;
		baddr		addr;
		u8		remote_name[248];
	} remote_name;

	struct __PACKED
	{
		bstatus 	status;
		hcihandle	handle;
		u8		enable;
	} encryption_change;

	struct __PACKED
	{
	} lkey_changed;

	struct __PACKED
	{
		bstatus 	status;
		hcihandle	handle;
		u8      	key_flag;
	} master_lkey;

	struct __PACKED
	{
		bstatus 	status;
		hcihandle	handle;
		//TODO
	} remote_features;
	

	struct __PACKED
	{
		bstatus 	status;
		hcihandle	handle;
	//	HciVersion	info;
	} remote_version;

	struct __PACKED
	{
		bstatus 	status;
		hcihandle	handle;
		//TODO
	} qos_setup;
	

	struct __PACKED
	{
	//	bstatus 	status;
		u8		packets;
		u16		opcode;
		//TODO
	} command_complete;

	struct __PACKED
	{
		bstatus 	status;
		u8		packets;
		u16		opcode;
	} status_changed;
	

	struct __PACKED
	{
		u8		error_code;
	} hw_error;
	

	struct __PACKED
	{
		hcihandle	handle;
	} flush_complete;
	

	struct __PACKED
	{
		bstatus 	status;
		baddr		addr;
		u8		new_role;
	} role_change;

	struct __PACKED
	{
		u8		packets;
		//TODO
	} completition;

	struct __PACKED
	{
		bstatus 	status;
		hcihandle	handle;
		u8		mode;
		u16		interval;
	} mode_change;
		

	struct __PACKED
	{
		u8		keys;
		//TODO
	} return_lkey;
		

	struct __PACKED
	{
		baddr		addr;
	} pin_request;
	

	struct __PACKED
	{
		baddr		addr;
	} lkey_needed;
	

	struct __PACKED
	{
		u8		link_key[16];
		u8		key_type;
	} lkey_creation;
 
	struct __PACKED
	{
		u8		link_type;
	} data_overflow;

	struct __PACKED
	{
		hcihandle	handle;
		u8		max_slots;
	} max_slots_change;

	struct __PACKED
	{
		bstatus 	status;
		hcihandle	handle;
		u16		offset;
	} clock_offset;

	struct __PACKED
	{
		bstatus 	status;
		hcihandle	handle;
		u16		packet_type;
	} packet_type_change;
		

	struct __PACKED
	{
		hcihandle	handle;
	} qos_violation;
		

	struct __PACKED
	{
		baddr		addr;
		u8		pscan_mode;
	} page_scan_mode;

	struct __PACKED
	{
		baddr		addr;
		u8		pscan_rep_mode;
	} page_scan_repetition_mode;
};
#endif
};

}//namespace Hci

#endif//__HCI_EVENT_H
