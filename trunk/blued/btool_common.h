#ifndef __BTOOL_COMMON_H
#define __BTOOL_COMMON_H

#define DBUS_HCI_SERVICE	"org.bluetool.hci"

#define	DBUS_HCIMAN_IFACE	"org.bluetool.hci.manager"
#define DBUS_HCIMAN_PATH	"/org/bluetool/hci/manager"

#define DBUS_HCIDEV_IFACE	"org.bluetool.hci.device"
#define DBUS_HCIDEV_PATH	"/org/bluetool/hci/"//+ hciX

#define DBUS_HCIREM_IFACE	"org.bluetool.hci.remote"
#define DBUS_HCIREM_SUBPATH	/* /org/bluetool/hci/hciX + */ "/cached/" /* + 00:11:AA:22:BB:CC */

#define DBUS_HCICONN_IFACE	"org.bluetool.hci.connection"
#define DBUS_HCICONN_SUBPATH	/* /org/bluetool/hci/hciX + */ "/connections/" /* + 0x12AC */

#define SDP_CLIENT_PATH		"/org/bluetool/sdp/client"
#define SDP_CLIENT_IFACE	"org.bluetool.sdp.client"

#endif//__BTOOL_COMMON_H
