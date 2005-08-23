#include "hcifilter_p.h"

namespace Hci
{

Filter::Filter()
{
	pvt = new Private;
}

Filter::Filter( const Filter& f )
{
	pvt = new Private;
	memcpy(&(pvt->filter),&(f.pvt->filter),sizeof(pvt->filter));
}

Filter::~Filter()
{
	delete pvt;
}

void Filter::clear()
{
	hci_filter_clear(&(pvt->filter));
}
void Filter::set_type( int bit )
{
	hci_filter_set_ptype(bit, &(pvt->filter));
}
void Filter::clear_type( int bit )
{
	hci_filter_clear_ptype(bit,&(pvt->filter));
}
bool Filter::test_type( int bit )
{
	return hci_filter_test_ptype(bit,&(pvt->filter)) != 0;
}
void Filter::set_event( int bit )
{
	hci_filter_set_event(bit,&(pvt->filter));
}
void Filter::clear_event( int bit )
{
	hci_filter_clear_event(bit,&(pvt->filter));
}
bool Filter::test_event( int bit )
{
	return hci_filter_test_event(bit,&(pvt->filter)) != 0;
}

u16 Filter::opcode()
{
	return pvt->filter.opcode;
}
void Filter::set_opcode( u16 opcode )
{
	pvt->filter.opcode = opcode;
}
void Filter::clear_opcode()
{
	pvt->filter.opcode = 0;
}
bool Filter::test_opcode( u16 opcode )
{
	return opcode == pvt->filter.opcode;
}

}//namespace Hci
