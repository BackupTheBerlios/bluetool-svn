#ifndef __HCI_FILTER_P_H
#define __HCI_FILTER_P_H

#include <bluetool/hcifilter.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>

namespace Hci
{

struct Filter::Private
{
	hci_filter filter;
};

}

#endif//__HCI_FILTER_P_H
