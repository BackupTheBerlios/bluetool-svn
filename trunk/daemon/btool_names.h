#ifndef __BTOOL_NAMES_H
#define __BTOOL_NAMES_H

#define BTOOL_ROOT_SERVICE	"org.bluetool"

#define BTOOL_ROOT_NAME		"org.bluetool."
#define BTOOL_ROOT_PATH		"/org/bluetool/"

#define BTOOL_ERROR_MALFORMED	BTOOL_ROOT_NAME "error.Malformed"
#define BTOOL_ERROR_TIMEDOUT	BTOOL_ROOT_NAME "error.Timedout"

#define BTOOL_DEVMAN_IFACE	BTOOL_ROOT_NAME "manager"
#define BTOOL_DEVMAN_PATH	BTOOL_ROOT_PATH "manager"

#define BTOOL_DEVICE_ROOT_NAME	BTOOL_ROOT_NAME "device."

#define BTOOL_DEVICES_PATH	BTOOL_ROOT_PATH "devices/"

#define BTOOL_SVC_SUBDIR	"services/"
#define BTOOL_SVC_IFACE		BTOOL_DEVICE_ROOT_NAME "service"
#define BTOOL_SVC_ROOT_NAME	BTOOL_DEVICE_ROOT_NAME "services."

#define	BTOOL_REM_SUBDIR	"remote/"
#define BTOOL_REM_IFACE		BTOOL_DEVICE_ROOT_NAME "remote"

#define BTOOL_HCICONN_IFACE	BTOOL_DEVICE_ROOT_NAME "hci.connection"

#define BTOOL_SDP_IFACE		BTOOL_DEVICE_ROOT_NAME "sdp"

#define BTOOL_SERVICEDB_IFACE	BTOOL_DEVICE_ROOT_NAME "servicedb"

#endif//__BTOOL_NAMES_H
