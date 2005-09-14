
'''This python module contains various constants and functions
	useful for using the Bluetool DBUS api from python'''

import dbus

try:	import dbus.glib
except	ImportError:
	pass

#
#	constants (stolen from hci.h in Bluez 2.20)
#
HCI_SCAN_INQUIRY= 1
HCI_SCAN_PAGE	= 2

HCI_LM_ACCEPT	= 0x8000
HCI_LM_MASTER	= 0x0001

HCI_LP_RSWITCH	= 0x0001
HCI_LP_HOLD	= 0x0002
HCI_LP_SNIFF	= 0x0004
HCI_LP_PARK	= 0x0008

#
#	device class decoding tables ( from the Bluetooth assigned numbers )
#
HCI_SERVICE_CLASSES = [
	#"Limited Discoverable Mode", None, None, 
	"Positioning", "Networking", "Rendering",
	"Capturing", "Object Transfer", "Audio",
	"Telephony", "Information"
]

HCI_DEVICE_CLASSES = [
	("Unknown"	,
	["Unknown"]
	),

	("Computer"	,
	["Unknown", "Workstation", "Server", "Laptop", "Handheld", "Palm" "Wearable"]
	),

	("Phone"	,
	["Unknown", "Cellular", "Cordless", "Smartphone", 
	"Modem or voice gateway", "ISDN access", "Sim card reader"]
	),

	("Lan"		,
	["Unknown", "1-17% available", "17-33% available", "33-50% available",
	"50-67 available%", "67-83 available%", "83-99 available%", "Not available"]
	),

	("Audio/Video"	,
	["Unknown", "Headset", "Hands-free", "Microphone", "Loudspeaker", "Headphones",
	"Portable audio", "Car audio", "Set-top box", "HIFI audio", "VCR", "Videocamera",
	"Camcorder", "Video monitor", "Video display and loudspeaker", "Video conferencing",
	None, "Gaming" ]
	),

	("Peripherial"	,
	["Unknown", "Keyboard", "Pointing Device", "Combo"]
	),

	("Imaging"	,
	["Unknown","Display","Camera","Scanner","Printer"]
	), #todo: this field is wrong

	("Wearable"	,
	["Wirst watch","Pager","Jacket","Helmet","Glasses"]
	)
]

def devclass2icon( major ):

	pixmaps = [ 
		'bt-logo.png', #Unknown
		'btdevice-computer.png',
		'btdevice-phone.png',
		'btdevice-lan.png',
		'bt-logo.png',	#A/V
		'bt-logo.png', #Peripherial
		'btdevice-imaging.png', 
		'bt-logo.png' #Wearable
	];
	return pixmaps[major]

#
#	service class ID list ( from the Bluetooth assigned numbers )
#
SDP_SVCLASS_IDS = {
	0x1101:	'Serial Port',
	0x1102: 'LAN Access over PPP',
	0x1103:	'Dialup Networking',
	0x1104:	'IrMC Sync',
	0x1105:	'OBEX Object Push',
	0x1106:	'OBEX File Transfer',
	0x1107: 'IrMC Sync Command',
	0x1108:	'Headset',
	0x1109:	'Cordless Telephony',
	0x110A:	'Audio source',
	0x110B:	'Audio Sink',
	0x110C:	'A/V Remote Control Target',
	0x110D: 'Advanced Audio Distribution',
	0x110E: 'A/V Remote Control',
	0x110F: 'Video Conferencing',
	0x1110: 'Intercom',
	0x1111: 'Fax',
	0x1112:	'Headset Audio Gateway',
	0x1113: 'WAP',
	0x1114: 'WAP Client',
	0x1115: 'PAN User',
	0x1116: 'Network Access Protocol',
	0x1117: 'Network Gateway',
	0x1118: 'Direct Printing',
	0x1119:	'Reference Printing',
	0x111A: 'Imaging',
	0x111B: 'Imaging Responder',
	0x111C: 'Imaging Automatic Archive',
	0x111D: 'Imaging Referenced Objects',
	0x111E: 'Handsfree',
	0x111F:	'Handsfree Audio Gateway',
	0x1120: 'Direct Printing Reference Objects',
	0x1121: 'Reflected UI',
	0x1122: 'Basic Printing',
	0x1123: 'Printing Status',
	0x1124: 'Human Interface Device',
	0x1125: 'Hardcopy Cable Replacement',
	0x1126: 'HCR Print',
	0x1127: 'HCR Scan',
	0x1128: 'Common ISDN Access',
	0x1120: 'Video Conferencing Gateway',
	0x112A: 'UDI MT',
	0x112B: 'UDI TA',
	0x112C: 'Audio/Video',
	0x112D: 'SIM Access',
	0x112E: 'Phonebook Access - PCE',
	0x112F: 'Phonebook Access - PSE',
	0x1200: 'PnP Information',
	0x1201: 'Generic Networking',
	0x1202: 'Generic File Transfer',
	0x1203: 'Generic Audio',
	0x1204: 'Generic Telephony',
	0x1205: 'UPNP Service',
	0x1206: 'UPNP IP Service',
	0x1300: 'EDSP_UPNP_IP_PAN',
	0x1301: 'EDSP_UPNP_IP_LAP',
	0x1302: 'EDSP_UPNP_IP_L2CAP',
	0x1303:	'Video Source',
	0x1304: 'Video Sink',
	0x1305: 'Video Distribution'
}

#
#	protocols 16-bit UUIDs ( from the Bluetooth assigned numbers )
#
SDP_PROTO_UUIDS = {
 	0x0001:	'SDP',
	0x0002:	'UDP',
	0x0003:	'RFCOMM',
	0x0004:	'TCP',
	0x0005:	'TCS-BIN',
	0x0006:	'TCS-AT',
	0x0008:	'OBEX',
	0x0009:	'IP',
	0x000a:	'FTP',
	0x000c:	'HTTP',
	0x000e:	'WSP',
	0x000f:	'BNEP',
	0x0010:	'UPNP',
	0x0011:	'HIDP',
	0x0012:	'HardcopyControlChannel',
	0x0014:	'HardcopyDataChannel',
	0x0016:	'HardcopyNotification',
	0x0017:	'Audio/Video Control Protocol',
	0x0019:	'Audio/Video Distribution Protocol',
	0x001b:	'CMTP',
	0x001d:	'UDI_C-Plane',
	0x0100:	'L2CAP',
}

#
#	frequently used dbus names
#
DBUS_BLUETOOL_SERVICE	= 'org.bluetool'

DBUS_MANAGER_IFACE	= 'org.bluetool.manager'
DBUS_MANAGER_PATH	= '/org/bluetool/manager'

DBUS_MODULES_IFACE	= 'org.bluetool.modules'
DBUS_MODULES_PATH	= '/org/bluetool/manager/modules'

DBUS_MODULE_IFACE	= 'org.bluetool.module'
#DBUS_MODULE_PATH	= <dinamically generated>

DBUS_INSTANCE_IFACE	= 'org.bluetool.instance'
#DBUS_INSTANCE_PATH	= <dinamically generated>

DBUS_DEVICE_HCI_IFACE	= 'org.bluetool.device.hci'
#DBUS_DEVICE_HCI_PATH	= <dinamically generated>

DBUS_REMOTE_HCI_IFACE	= 'org.bluetool.remote.hci'
#DBUS_REMOTE_HCI_PATH	= <dinamically generated>

DBUS_REMOTE_SDP_IFACE	= 'org.bluetool.remote.sdp'
#DBUS_REMOTE_SDP_PATH	= <dinamically generated>

DBUS_RECORD_IFACE	= 'org.bluetool.remote.sdp.record'
#DBUS_RECORD_PATH	= <dinamically generated>

#
#	functions to build dbus proxies
#
def get_manager(bus):
	return dbus.Interface(
		bus.get_object( DBUS_BLUETOOL_SERVICE, DBUS_MANAGER_PATH ),
		DBUS_MANAGER_IFACE
	)

def get_modules(bus):
	return dbus.Interface(
		bus.get_object( DBUS_BLUETOOL_SERVICE, DBUS_MODULES_PATH ),
		DBUS_MODULES_IFACE
	)

def get_module(bus,path):
	return dbus.Interface(
		bus.get_object( DBUS_BLUETOOL_SERVICE, path ),
		DBUS_MODULE_IFACE
	)

def get_instance(bus,path):
	return dbus.Interface(
		bus.get_object( DBUS_BLUETOOL_SERVICE, path ),
		DBUS_INSTANCE_IFACE
	)

def get_local_hci(bus,path):
	return dbus.Interface(
		bus.get_object( DBUS_BLUETOOL_SERVICE, path ),
		DBUS_DEVICE_HCI_IFACE
	)

def get_remote_hci(bus,path):
	return dbus.Interface(
		bus.get_object( DBUS_BLUETOOL_SERVICE, path ),
		DBUS_REMOTE_HCI_IFACE
	)

def get_remote_sdp(bus,path):
	return dbus.Interface(
		bus.get_object( DBUS_BLUETOOL_SERVICE, path ),
		DBUS_REMOTE_SDP_IFACE
	)

def get_remote_interfaces(bus,path):
	obj = bus.get_object( DBUS_BLUETOOL_SERVICE, path )
	hci = dbus.Interface( obj, DBUS_REMOTE_HCI_IFACE )
	sdp = dbus.Interface( obj, DBUS_REMOTE_HCI_IFACE )
	return ( obj, sdp )

def get_record(bus,path):
	return dbus.Interface(
		bus.get_object( DBUS_BLUETOOL_SERVICE, path ),
		DBUS_RECORD_IFACE
	)
