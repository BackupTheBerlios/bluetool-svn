#!/usr/bin/env python

import sys
import os
import pygtk
pygtk.require('2.0')
import gtk
import gtk.glade
import gobject
import dbus
import pdb

if getattr(dbus, 'version', (0,0,0)) >= (0,41,0):
    import dbus.glib

#from bluetool_dbus import BluetoolManagerProxy, BluetoolDeviceProxy


#
#	error reporting
#
def bt_popup_error(list):
	dialog = gtk.MessageDialog(
		parent         = None,
		flags          = gtk.DIALOG_DESTROY_WITH_PARENT,# | gtk.DIALOG_MODAL,
		type           = gtk.MESSAGE_ERROR,
		buttons        = gtk.BUTTONS_OK,
		message_format = list[0]
	)
	dialog.connect('response',lambda x,y:dialog.destroy())
	dialog.show()		

def bt_print_error(list):
	print "error: ",list[0]

def bt_empty_handler(list):
	#print list
	pass

#
#	util
#
SVC_CLASS_IDS = {
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

PROTO_UUIDS = {
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

def major_to_pixmap(major):
	
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
	return gtk.gdk.pixbuf_new_from_file( pixmaps[major] )

class BrowserModel:

	def __init__(self):
		#
		#	create the treestore to be used as a model,
		#	its contents will be:
		#	( dbus_object, dev_name, dev_address, label, class, icon )
		#	while for a service
		#	( dbus_object, service_name, none, label, none, icon )

		self.device_store = gtk.TreeStore(object, gtk.TreeStore, str, str, str, int, gtk.gdk.Pixbuf)

		#
		#	create proxy for remote object (the manager)
		#
		self.bus = dbus.SystemBus()
		self.obj = self.bus.get_object('org.bluetool','/org/bluetool/manager')
		self.iface = dbus.Interface(self.obj,'org.bluetool.manager')

		#
		#	bind signals to local hook functions
		#
		self.iface.connect_to_signal('DeviceAdded',self.sig_DeviceAdded);
		self.iface.connect_to_signal('DeviceRemoved',self.sig_DeviceRemoved);

		#
		#	find devices
		#
		devices = self.iface.ListDevices()
		for devpath in devices:
			self.sig_DeviceAdded(devpath)

		#
		#	list modules
		#
		self.module_store = gtk.ListStore(str,str,str)

		moddb = dbus.Interface(
			self.bus.get_object('org.bluetool', '/org/bluetool/manager/modules'), 				'org.bluetool.modules'
		)

		mods = moddb.ListModules()

		print mods

		for mod_path in mods:
			mod = dbus.Interface(
				self.bus.get_object('org.bluetool', mod_path), 'org.bluetool.module'
			)
			self.module_store.append([mod_path, mod.Name(), mod.Description()])


	#
	#	signal handlers
	#
	def sig_DeviceAdded(self,devpath):
		#
		#	create an object to handle the local device
		#
		print "adding ", devpath

		dev = self.bus.get_object('org.bluetool',devpath)
		hci = dbus.Interface(dev, 'org.bluetool.device.hci')

		obj = { 'path': devpath, 'dev': dev, 'hci': hci }

		name = hci.GetProperty('name')[1]
		address = hci.GetProperty('address')[1]

		label = name + ' ('+address+')'

		inquiry_store = gtk.TreeStore(object, gtk.TreeStore, str, str, str, int, gtk.gdk.Pixbuf)

		newit = self.device_store.append(
			None, [ obj, inquiry_store, name, address, label, 0, None ]
		)
		newref = gtk.TreeRowReference(self.device_store,self.device_store.get_path(newit))

		hci.connect_to_signal('DeviceInRange',lambda path:self.sig_DeviceInRange(inquiry_store,path))
		hci.connect_to_signal('DeviceOutOfRange',lambda path:self.sig_DeviceOutOfRange(inquiry_store,path))

		cache = hci.InquiryCache()
		for cached_path in cache:
			self.sig_DeviceInRange(inquiry_store,cached_path)

	def sig_DeviceRemoved(self,devpath):

		print "removing", devpath

		it = self.device_store.iter_children(None)
		while it:
			path = self.device_store[it][0]['path']
			if path == devpath:
				self.device_store.remove(it)
				break
			it = self.device_store.iter_next(it)
			

	def sig_DeviceInRange(self,inquiry_store,devpath):
#		it = self.device_store.get_iter(parent_ref.get_path())

#		inquiry_store = self.device_store[it][1]

		dev = self.bus.get_object('org.bluetool',devpath)
		hci = dbus.Interface(dev, 'org.bluetool.remote.hci')
		sdp = dbus.Interface(dev, 'org.bluetool.remote.sdp')

		obj = { 'path': devpath, 'dev': dev, 'hci': hci, 'sdp': sdp }

		service_store = gtk.TreeStore(object,str,str)

		newit = inquiry_store.append(
			None, [ obj, service_store, '', '', None, 0, None ]
		)
		newref = gtk.TreeRowReference(inquiry_store,inquiry_store.get_path(newit))

		hci.GetProperty('name',
			reply_handler=lambda st,name:
				self.remote_name_hdl(inquiry_store,newref,st,name),
			error_handler=bt_popup_error)

		hci.GetProperty('address',
			reply_handler=lambda st,addr:
				self.remote_address_hdl(inquiry_store,newref,st,addr),
			error_handler=bt_popup_error)

		hci.GetProperty('class',
			reply_handler=lambda st,svc,major,minor:
				self.remote_class_hdl(inquiry_store,newref,st,svc,major,minor),
			error_handler=bt_popup_error)

		#
		#	if some services are already in cache, show them
		#
		cached_services = sdp.SearchAllRecordsCached()
		print cached_services
		for record_path in cached_services[1]:
			self.add_record(service_store,record_path)

	def add_record(self,service_store,rec_path):

		print 'new record @', rec_path
		rec_obj = self.bus.get_object('org.bluetool',rec_path)
		rec = dbus.Interface(rec_obj,'org.bluetool.remote.sdp.record')

		obj = { 'path': rec_path, 'rec': rec }

		name = '<Unkown>'
		desc = ''
		try:
			name = rec.GetServiceName()
		except:
			pass

		try:
			svc_id = rec.GetClassIdList()
			#print svc_id
			desc = SVC_CLASS_IDS[svc_id[0]]
		except:
			pass

		service_store.append(None,[obj, name, desc])

		#print rec.GetClassIdList()
		#try: print rec.GetProtocolDescList()
		#except:	pass

	def remote_name_hdl(self,store,ref,st,name):
		it = store.get_iter(ref.get_path())
		store.set_value(it,2,name)

	def remote_address_hdl(self,store,ref,st,addr):
		it = store.get_iter(ref.get_path())
		store.set_value(it,3,addr)

	def remote_class_hdl(self,store,ref,st,svc,major,minor):
		it = store.get_iter(ref.get_path())
		store.set_value(it,5,major)
		store.set_value(it,6,major_to_pixmap(major))

	def sig_DeviceOutOfRange(self,inquiry_store,devpath):

		subit = inquiry_store.iter_children(None)
		while subit:
			path = inquiry_store[subit][0]['path']
			if path == devpath:
				inquiry_store.remove(subit)
				return
			subit = self.device_store.iter_next(subit)

	#
	#	functions to be called by the controller (the gui, actually)
	#
	def get_device_store(self):
		return self.device_store

	def get_module_store(self):
		return self.module_store


class BluetoolBrowser:

	def __getitem__(self,key):
		return self.widgets.get_widget(key)

	def __init__(self,bmodel):

		#
		#	load graphical interface
		#
		self.widgets = gtk.glade.XML('bluetool_gui.glade', 'bt_browser_wnd')
		handlers = {
			"on_bt_browser_close"		: self.on_close,
			"on_bt_inqstart_btn_clicked"	: self.on_start_inquiry,
			"on_bt_inqstop_btn_clicked"	: self.on_stop_inquiry,
			"on_bt_devices_cb_selected"	: self.on_select_device,
			"on_bt_discovered_tv_row_activated" : self.on_click_remote,
			"on_bt_svcscan_btn_clicked"	: self.on_click_refresh,
			"on_bt_services_tv_row_activated" : self.on_click_service
		}
		self.widgets.signal_autoconnect(handlers)

		#
		#	add progress bar
		#
		self.inquiry_running = False
		self.inquiry_runner = None
		self.inquiry_progress = gtk.ProgressBar()
		bt_progress_box = self['bt_progress_box']
		bt_progress_box.pack_start(self.inquiry_progress,True,True)
		bt_progress_box.show_all()

		self.bmodel = bmodel
		self.bmodel.get_device_store().connect('row-changed', self.on_devices_change)

		self.curr_inquiry_store = None

		#
		#	use the combobox to show a view of the model
		#
		tc = gtk.CellRendererText()
		bt_devices_cb = self['bt_devices_cb']
		bt_devices_cb.set_model(self.bmodel.get_device_store())
		bt_devices_cb.pack_start(tc, True)
		bt_devices_cb.add_attribute(tc, 'text', 4)

		#
		#	another view of the same data is in the treeview
		#
		bt_discovered_tv = self['bt_discovered_tv']

		icon_cell = gtk.CellRendererPixbuf()
		icon_col = gtk.TreeViewColumn('Type')
		icon_col.pack_start(icon_cell, True)
		icon_col.add_attribute(icon_cell, 'pixbuf', 6)
		bt_discovered_tv.append_column(icon_col)

		name_cell = gtk.CellRendererText()
		name_col = gtk.TreeViewColumn('Device name')
		name_col.pack_start(name_cell, True)
		name_col.add_attribute(name_cell, 'text', 2)
		bt_discovered_tv.append_column(name_col)

		addr_cell = gtk.CellRendererText()
		addr_col = gtk.TreeViewColumn('Address')
		addr_col.pack_start(addr_cell, True)
		addr_col.add_attribute(addr_cell, 'text', 3)
		bt_discovered_tv.append_column(addr_col)

		bt_discovered_tv.set_model(None)

		bt_selected_discovery = bt_discovered_tv.get_selection()
		bt_selected_discovery.connect('changed',self.on_select_remote)

		#
		#	yet another part of the model is shown in the services view
		#
		bt_services_tv = self['bt_services_tv']

		svcname_cell = gtk.CellRendererText()
		svcname_col = gtk.TreeViewColumn('Service name')
		svcname_col.pack_start(svcname_cell,True)
		svcname_col.add_attribute(svcname_cell,'text', 1)

		bt_services_tv.append_column(svcname_col)

		svcdesc_cell = gtk.CellRendererText()
		svcdesc_col = gtk.TreeViewColumn('Class')
		svcdesc_col.pack_start(svcdesc_cell,True)
		svcdesc_col.add_attribute(svcdesc_cell,'text', 2)

		bt_services_tv.append_column(svcdesc_col)

		self.empty_svc_store = gtk.ListStore(str,str)
		bt_services_tv.set_model(self.empty_svc_store)

		if len(self.bmodel.get_device_store()):
			bt_devices_cb.set_active(0)

		#
		#	and now, the module list
		#
		bt_pi_pan = self['bt_pi_pan']
		bt_pi_pan.set_position(250)

		bt_installedpi_tv = self['bt_installedpi_tv']

		modname_cell = gtk.CellRendererText()
		modname_col = gtk.TreeViewColumn('Module name')
		modname_col.pack_start(modname_cell, True)
		modname_col.add_attribute(modname_cell, 'text', 1)

		bt_installedpi_tv.append_column(modname_col)

		moddesc_cell = gtk.CellRendererText()
		moddesc_col = gtk.TreeViewColumn('Description')
		moddesc_col.pack_start(moddesc_cell, True)
		moddesc_col.add_attribute(moddesc_cell, 'text', 2)

		bt_installedpi_tv.append_column(moddesc_col)

		bt_installedpi_tv.set_model(self.bmodel.get_module_store())


	def on_close(self,window,event):
		gtk.main_quit()

	def on_devices_change(self,model,path,it):

		self.curr_inquiry_store = model[it][1]

		bt_discovered_tv = self['bt_discovered_tv']
		bt_discovered_tv.set_model(self.curr_inquiry_store)

		bt_services_tv = self['bt_services_tv']
		bt_services_tv.set_model(self.empty_svc_store)

		bt_devices_cb = self['bt_devices_cb']

		if bt_devices_cb.get_active() < 0:
			bt_devices_cb.set_active(0)

	def on_select_device(self,widget):

		if self.inquiry_running: return True

		bt_devices_cb = self['bt_devices_cb']
		idx = bt_devices_cb.get_active()
		if idx < 0: return

		self.curr_inquiry_store = self.bmodel.get_device_store()[(idx)][1]

		bt_discovered_tv = self['bt_discovered_tv']
		bt_discovered_tv.set_model(self.curr_inquiry_store)

		bt_services_tv = self['bt_services_tv']
		bt_services_tv.set_model(self.empty_svc_store)
	#
	#	service discovery
	#
	def on_select_remote(self, widget):

		bt_devices_cb = self['bt_devices_cb']
		bt_discovered_tv = self['bt_discovered_tv']
		bt_services_tv = self['bt_services_tv']

		mdl, it = bt_discovered_tv.get_selection().get_selected()

		print mdl.get_path(it) 
		bt_services_tv.set_model(mdl[it][1])

	def on_click_remote(self, tv, path, col):

		store = tv.get_model()

		if not store: return

		obj = store[path][0]
		service_store = store[path][1]

		self.scan_services(path)

	def on_click_refresh(self,widget):
		bt_devices_cb = self['bt_devices_cb']
		idx = bt_devices_cb.get_active()
		if idx < 0:
			bt_error_popup(['Select a remote device to scan'])
			return

		self.scan_services((idx,))

	def scan_all_devices(self):
		
		it = self.curr_inquiry_store.iter_children(None)
		while it:
			self.scan_services(it)
			it = self.curr_inquiry_store.iter_next(it)

	def scan_services(self, path_or_it):

		obj = self.curr_inquiry_store[path_or_it][0]
		service_store = self.curr_inquiry_store[path_or_it][1]
		addr = self.curr_inquiry_store[path_or_it][3]

		self.inquiry_progress.set_text("Performing service scan on "+addr+"...")
		self.inquiry_progress.set_fraction(0.0)
		obj['sdp'].SearchAllRecords(
			reply_handler=lambda st,recs: self.sdp_scan_complete_hdl(service_store,st,recs),
			error_handler=bt_popup_error)

	def sdp_scan_complete_hdl(self,service_store,st,recs):

		service_store.clear()

		bus = dbus.SystemBus()

		for rec_path in recs:

			self.bmodel.add_record(service_store,rec_path)

		self.inquiry_progress.set_text("Service scan complete")
		self.inquiry_progress.set_fraction(1.0)
	
	#
	#	device inquiry
	#
	def cb_update_inquiry_progress(self):
		f = self.inquiry_progress.get_fraction()+0.005
		if f > 1.0: f = 1.0
		self.inquiry_progress.set_fraction(f)
		self.inquiry_progress.set_text("Scanning...")
		if f == 1.0:
			gobject.source_remove(self.inquiry_runner)
			self.inquiry_runner = None
			return False
		else:
			return True

	def inquiry_complete_hdl(self,list):
		self.inquiry_progress.set_text("Device scan complete")

		if self.inquiry_running:	# but don't go on if scan was canceled by user
			self.scan_all_devices()

		self.inquiry_running = False
		self['bt_discovered_tv'].get_selection().select_path((0,))

	def on_start_inquiry(self,widget):
		#if not len(self.model): return

		bt_devices_cb = self['bt_devices_cb']
		idx = bt_devices_cb.get_active()
		if idx < 0: return

		it = self.bmodel.get_device_store().iter_nth_child(None,idx)
		obj = self.bmodel.get_device_store()[it][0]

		if self.inquiry_running:
			bt_popup_error(['Operation already in progress'])
			return

		self.inquiry_progress.set_fraction(0)
		self.inquiry_runner = gobject.timeout_add(100,self.cb_update_inquiry_progress)
		self.inquiry_running = True

		obj['hci'].StartInquiry(
			reply_handler=self.inquiry_complete_hdl,error_handler=bt_popup_error)

	def inquiry_canceled_hdl(self,list):
		gobject.source_remove(self.inquiry_runner)		
		self.inquiry_progress.set_text("Canceled")
		self.inquiry_progress.set_fraction(0)
		self.inquiry_running = False

	def on_stop_inquiry(self,widget):
		if not len(self.bmodel.get_device_store()): return

		bt_devices_cb = self['bt_devices_cb']
		idx = bt_devices_cb.get_active()
		if idx < 0: return

		self.inquiry_running = False

		it = self.bmodel.get_device_store().iter_nth_child(None,idx)
		obj = self.bmodel.get_device_store()[it][0]
		obj['hci'].CancelInquiry(
			reply_handler=self.inquiry_canceled_hdl, error_handler=bt_popup_error)

	#
	#
	#
	def on_click_service(self, tv, path, col):

		model = tv.get_model()

		if not model: return

		obj = model[path][0]

		dialog = BluetoolServiceDialog(obj)

class BluetoolServiceDialog:

	def __getitem__(self,key):
		return self.widgets.get_widget(key)

	def __init__(self,dbus_object):

		self.widgets = gtk.glade.XML('bluetool_gui.glade', 'bt_svcinfo_dlg')

		self.dbus_obj = dbus_object

		self.class_store = gtk.ListStore(str)

		class_cell = gtk.CellRendererText()
		bt_svcprofile_tv = self['bt_svcprofile_tv']
		class_col = gtk.TreeViewColumn()
		class_col.pack_start(class_cell, True)
		class_col.add_attribute(class_cell, 'text', 0)

		bt_svcprofile_tv.append_column(class_col)
		bt_svcprofile_tv.set_model(self.class_store)

		cid_list = self.dbus_obj['rec'].GetClassIdList()

		for cid_uuid in cid_list:
			cid_name = SVC_CLASS_IDS[cid_uuid]

			self.class_store.append([ cid_name ])

		proto_list = self.dbus_obj['rec'].GetProtocolDescList()

		self.proto_store = gtk.ListStore(str)

		proto_cell = gtk.CellRendererText()
		bt_svcproto_tv = self['bt_svcproto_tv']
		proto_col = gtk.TreeViewColumn()
		proto_col.pack_start(proto_cell, True)
		proto_col.add_attribute(proto_cell, 'text', 0)
		
		bt_svcproto_tv.append_column(proto_col)
		bt_svcproto_tv.set_model(self.proto_store)

		for proto_desc in proto_list:
			proto_uuid = proto_desc[0]
			proto_label = PROTO_UUIDS[proto_uuid]
			proto_port = proto_desc[1]
			if proto_port != 0xffff:
				if proto_label == 'L2CAP':
					proto_label += ' ( PSM='+repr(proto_port)+' )'
				elif proto_label == 'RFCOMM':
					proto_label += ' ( Channel='+repr(proto_port)+' )'

			self.proto_store.append([ proto_label ])

def main(argv):
	bm = BrowserModel()
	bb = BluetoolBrowser(bm)
	gtk.main()

if __name__ == '__main__':
	main(sys.argv)
	
