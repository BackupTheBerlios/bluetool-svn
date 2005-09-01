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

from bluetool_parser import BluezConfig

if getattr(dbus, 'version', (0,0,0)) >= (0,41,0):
    import dbus.glib

HCID_CONF_PATH = './hcid.conf'

#
#	CONSTANTS (stolen from hci.h in Bluez 2.19)
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
#	Device class decoding tables (straight from the BT assigned numbers page)
#
device_service_classes = [
	#"Limited Discoverable Mode", None, None, 
	"Positioning", "Networking", "Rendering",
	"Capturing", "Object Transfer", "Audio",
	"Telephony", "Information"
]

device_major_minor_classes = [
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

#
#	a simple tristate button
#
DEFAULT = -1
OFF = 0
ON = 1
TRISTATES = set([DEFAULT, ON, OFF])

def set_tristate(widget, state):
	if state not in TRISTATES: return

	widget.tristate = state

	if widget.tristate == ON:
		widget.set_inconsistent(False)
		widget.set_active(True)

	elif widget.tristate == OFF:
		widget.set_inconsistent(False)
		widget.set_active(False)	

	elif widget.tristate == DEFAULT:
		widget.set_inconsistent(True)

def toggle_tristate(widget):

	try:
		if widget.frozen: return
	except:
		pass

	try:
		ts = widget.tristate
	except:
		if widget.get_inconsistent():
			widget.tristate = DEFAULT
		else:
			widget.tristate = OFF

	widget.frozen = True

	if widget.tristate == ON:
		set_tristate(widget, DEFAULT)

	elif widget.tristate == OFF:
		set_tristate(widget, ON)

	elif widget.tristate == DEFAULT:
		set_tristate(widget, OFF)

	widget.frozen = False

	print widget.tristate
	return widget.tristate

#
#	the main window
#
class BluetoolCfgPanel:

	#
	#	populate the gui with settings stored in the model
	#
	def model_to_ui(self):

		self.updating = True
		
		for widget_name in self.cfg_mdl.keys():
			active = self.cfg_mdl[widget_name][0]
			self[widget_name].set_active(active)

		#for dev in self.cfg-
		bt_devices_cb = self['bt_devices_cb']
		idx = bt_devices_cb.get_active()
		print 'idx',idx
		if idx < 0:
			self.updating = False
			return

		row = self.dev_store[(idx,)]

		self['bt_cfgsave_tgb'].set_active(row[3])
		self.curr_dev_mdl = row[1]

		#print repr(dev_mdl)

		for widget_name in self.curr_dev_mdl.keys():
			active = self.curr_dev_mdl[widget_name][0]
			tristate = self.curr_dev_mdl[widget_name][1]
			#print repr(self.curr_dev_mdl[widget_name])
			if   widget_name.endswith('_cb'):
				if tristate:
					set_tristate(self[widget_name],active)
					#toggle_tristate(self[widget_name])
				else:
					if   active == ON:
						self[widget_name].set_active(True)
					elif active == OFF	\
					or   active == DEFAULT:
						self[widget_name].set_active(False)

			elif widget_name == 'bt_devname_txt':

				name = self.curr_dev_mdl[widget_name][3]

				if len(name): 
					name = name.strip('"')

				if   active == ON:
					self['bt_devname_cb'].set_active(True)
				elif active == OFF	\
				or   active == DEFAULT:
					self['bt_devname_cb'].set_active(False)

				if self['bt_devname_cb'].get_active():
					self[widget_name].set_text(name)
					self[widget_name].set_property('editable',True)
				else:
					self[widget_name].set_text('')
					self[widget_name].set_property('editable',False)

			elif widget_name == 'bt_devclass_tgb':

				cls = self.curr_dev_mdl[widget_name][3]
				if len(cls): 
					cls = int(cls,16)
				else:	cls = 0

				if   active == ON:
					self['bt_devclass_cb'].set_active(True)
				elif active == OFF	\
				or   active == DEFAULT:
					self['bt_devclass_cb'].set_active(False)

				self[widget_name].set_label(hex(cls))
				
				self.class_model_to_ui(cls)

		stat_mdl = row[6]

		for widget_name in stat_mdl.keys():
			if widget_name.endswith('_lab'):
				self[widget_name].set_label(stat_mdl[widget_name])

		self.updating = False

	#
	#	update the model from widgets state
	#
	def ui_to_model(self):

		for widget_name in self.cfg_mdl.keys():
			active = self[widget_name].get_active()
			self.cfg_mdl[widget_name][0] = active
			#print repr(self.cfg_mdl[widget_name])


		bt_devices_cb = self['bt_devices_cb']
		idx = bt_devices_cb.get_active()
		print 'idx',idx
		if idx < 0: return

		row = self.dev_store[(idx,)]

		dev_mdl = row[1]
		for widget_name in self.curr_dev_mdl.keys():
			tristate = self.curr_dev_mdl[widget_name][1]

			if tristate:
				active = self[widget_name].tristate
			elif widget_name.endswith('_cb'):		
				active = self[widget_name].get_active()

			self.curr_dev_mdl[widget_name][0] = active

		bt_devname_cb = self['bt_devname_cb']
		nactive = bt_devname_cb.get_active()

		self.curr_dev_mdl['bt_devname_txt'][0] = nactive
		self.curr_dev_mdl['bt_devname_txt'][3] = self['bt_devname_txt'].get_text()

		bt_devclass_cb = self['bt_devclass_cb']
		cactive = bt_devclass_cb.get_active()

		self.curr_dev_mdl['bt_devclass_tgb'][0] = cactive
		self.curr_dev_mdl['bt_devclass_tgb'][3] = self['bt_devclass_tgb'].get_label()
	#
	#	device class dialog
	#
	def class_model_to_ui(self,cls):
		dcw = self.devclass_widgets

		svc   = dcw.get_widget('bt_devclass_service_tv')

		sel   = svc.get_selection()
		sel.unselect_all()

		svc_id = ( cls >> 16 ) & 0xff;
		maj_id = ( cls >> 8 )  & 0xff;
		min_id = ( cls ) & 0xff;

		for bit in range(0,7):
			if svc_id & (1 << bit):
				sel.select_iter(self.devclass_service_store.iter_nth_child(None,bit))

		major = dcw.get_widget('bt_devclass_major_tv')

		minor = dcw.get_widget('bt_devclass_minor_tv')

		sel = major.get_selection()
		it = self.devclass_major_store.iter_nth_child(None,maj_id)
		sel.select_iter(it)

		sel = minor.get_selection()
		#print self.devclass_major_store[it][0]
		store = self.devclass_minor_stores[self.devclass_major_store[it][0]]
		try: 	sel.select_iter(store.iter_nth_child(None,min_id))
		except: pass

	def class_ui_to_model(self):
		dcw = self.devclass_widgets
		svc   = dcw.get_widget('bt_devclass_service_tv')
		major = dcw.get_widget('bt_devclass_major_tv')
		minor = dcw.get_widget('bt_devclass_minor_tv')

		if not self.curr_dev_mdl: return

		svc_id = 0;
		maj_id = 0;
		min_id = 0;

		model,rows   = svc.get_selection().get_selected_rows()
		for bit in [ s[0] for s in rows ]:
			svc_id |= ( 1 << bit )

		model,it = major.get_selection().get_selected()
		maj_id = model.get_path(it)[0]

		model,it = minor.get_selection().get_selected()
		min_id = model.get_path(it)[0]
		
		self.curr_dev_mdl['bt_devclass_tgb'][3] = hex((svc_id << 16 ) | (maj_id << 8) | (min_id))		


	#
	#
	#
	def manager_to_model(self):
		try:

			for row in self.dev_store:

				if row[2] is None: 
					continue	# the device is from the conf file

				found = False

				for devpath in self.dev_paths:
					print row[0], row[2]
					if row[2]['path'] == devpath:
						
						found = True
						break

				if not found:
					self.dev_store.remove(row.iter)

			for devpath in self.dev_paths:

				obj = self.sys_bus.get_object('org.bluetool',devpath)
				hci = dbus.Interface(obj,'org.bluetool.device.hci')

				dev = { 'path':devpath, 'obj':obj, 'hci':hci }

				dev_mdl = self.device_model()
				stat_mdl = self.status_model()

				up,rx_bts,rx_err,tx_bts,tx_err = hci.GetProperty('stats')
	
				name = ''
				address = hci.GetProperty('address')[1]

				if up != 0:
					name = hci.GetProperty('name')[1]

					stat_mdl['bt_status_lab'] = 'ON'
					stat_mdl['bt_rxbytes_lab'] = repr(rx_bts)
					stat_mdl['bt_txbytes_lab'] = repr(tx_bts)
					stat_mdl['bt_address_lab'] = address

					hci_ver, hci_rev, lmp_ver, lmp_rev, man = hci.GetProperty('version_info')[1:]
					stat_mdl['bt_hciver_lab'] = str(hci_ver) + ' ( rev. ' + repr(hci_rev) + ' )'
					stat_mdl['bt_lmpver_lab'] = str(lmp_ver) + ' ( sub. ' + repr(lmp_rev) + ' )'
					stat_mdl['bt_manuf_lab'] = man

					dev_mdl['bt_devname_txt'][0] = ON
					dev_mdl['bt_devname_txt'][3] = name
					print dev_mdl['bt_devname_txt']

					dev_mdl['bt_devclass_tgb'][0] = ON
					dev_mdl['bt_devclass_tgb'][3] = hex(hci.GetProperty('class')[1])

					if hci.GetProperty('auth_enable') == 0:
						ae = OFF
					else:
						ae = ON
					dev_mdl['bt_auth_enable_cb'][0] = ae

					if hci.GetProperty('encrypt_mode') == 0:
						ee = OFF
					else:
						ee = ON
					dev_mdl['bt_encrypt_enable_cb'][0] = ee

					scan = hci.GetProperty('scan_enable')[1]
					if scan & HCI_SCAN_INQUIRY:
						dev_mdl['bt_iscan_enable_cb'][0] = ON
					else:	dev_mdl['bt_iscan_enable_cb'][0] = OFF

					if scan & HCI_SCAN_PAGE:
						dev_mdl['bt_pscan_enable_cb'][0] = ON
					else:	dev_mdl['bt_pscan_enable_cb'][0] = OFF

					lm = hci.GetProperty('link_mode')[1]
					if lm & HCI_LM_ACCEPT:
						dev_mdl['bt_lm_accept_cb'][0] = ON
					else:	dev_mdl['bt_lm_accept_cb'][0] = OFF

					if lm & HCI_LM_MASTER:
						dev_mdl['bt_lm_master_cb'][0] = ON
					else:	dev_mdl['bt_lm_master_cb'][0] = OFF

					lp = hci.GetProperty('link_policy')[1]

					if lp & HCI_LP_RSWITCH:
						dev_mdl['bt_lp_rswitch_cb'][0] = ON
					else:	dev_mdl['bt_lp_rswitch_cb'][0] = OFF

					if lp & HCI_LP_HOLD:
						dev_mdl['bt_lp_hold_cb'][0] = ON
					else:	dev_mdl['bt_lp_hold_cb'][0] = OFF

					if lp & HCI_LP_SNIFF:
						dev_mdl['bt_lp_sniff_cb'][0] = ON
					else:	dev_mdl['bt_lp_sniff_cb'][0] = OFF

					if lp & HCI_LP_PARK:
						dev_mdl['bt_lp_park_cb'][0] = ON
					else:	dev_mdl['bt_lp_park_cb'][0] = OFF
				else:
					stat_mdl['bt_status_lab'] = 'OFF'
	
				if len(name):	lbl = address + ' ( ' + name + ' ) '
				else:		lbl = address

				# if device already present, just update it with the new model

				found = False

				for row in self.dev_store:
					if row[0] == address:
						found = True
						row[1] = dev_mdl
						row[5] = lbl
						row[6] = stat_mdl
						break

				if not found:
					self.dev_store.append([address,dev_mdl,dev, False,'',lbl,stat_mdl])

		except dbus.dbus_bindings.DBusException, e:
			#bt_error_popup('Unable to communicate with device:'+repr(e))
			print 'Unable to communicate with device:'+str(e)


	#
	#
	#
	def model_to_file(self):
		
		opts = self.cfg_file.options

		for widget_name in self.cfg_mdl.keys():
			active, optname, good, bad = self.cfg_mdl[widget_name]

			if not opts.has_key(optname):
				opts[optname] = []

			if active:

				try:	opts[optname].remove(bad)
				except:	pass
					
				if opts[optname].count(good) == 0:
					opts[optname].append(good)

			else:

				try:	opts[optname].remove(good)
				except:	pass
					
				if len(bad) and opts[optname].count(bad) == 0:
					opts[optname].append(bad)

				if len(opts[optname]) == 0:
					opts.pop(optname)

		devs = self.cfg_file.devices

		for (address, model, dbus, save, name, label, stats_mdl) in self.dev_store:

			print 'save', save
			if not save:
				if devs.has_key(address):
					devs.pop(address)
				continue

			if not devs.has_key(address):
				devs[address] = {}

			dev_cfg = devs[address]

			print "[bef]",dev_cfg

			for opt in model.keys():
				active, tristate, optname, val_on, val_off = model[opt]

				print active, tristate, optname, val_on, val_off

				if optname == 'name':
					if active:
						dev_cfg['name'] = ['"' + val_on + '"']
					else:
						if dev_cfg.has_key(optname):
							dev_cfg.pop(optname)
					continue

				if optname == 'class':
					if active:
						dev_cfg['class'] = [ val_on ]
					else:
						if dev_cfg.has_key(optname):
							dev_cfg.pop(optname)
					continue

				if tristate:
					if active == DEFAULT:
						if dev_cfg.has_key(optname):
							dev_cfg.pop(optname)
						continue
					elif active == ON:
						active = True
					elif active == OFF:
						active = False

				if not dev_cfg.has_key(optname):
					dev_cfg[optname] = []
#
				if active:

					try:	dev_cfg[optname].remove(val_off)
					except:	pass
					
					if dev_cfg[optname].count(val_on) == 0:
						dev_cfg[optname].append(val_on)

				else:
					#pdb.set_trace()

					try:	dev_cfg[optname].remove(val_on)
					except:	pass
					
					if len(val_off) and dev_cfg[optname].count(val_off) == 0:
						dev_cfg[optname].append(val_off)

					if len(dev_cfg[optname]) == 0:
						dev_cfg.pop(optname)

			print "[aft]",dev_cfg

#

		self.cfg_file.store()

	#
	#
	#
	def file_to_model(self):
		
		opts = self.cfg_file.options
		for opt in opts.keys():
			#print 'opt',opt
			for val in opts[opt]:
				#print 'val',val
				for row in self.cfg_mdl.keys():
					#print 'row',repr(row)
					if self.cfg_mdl[row][1] == opt:
						if   self.cfg_mdl[row][2] == val:
							self.cfg_mdl[row][0] = True
						#elif self.cfg_mdl[row][3] == val:
						else:
							self.cfg_mdl[row][0] = False
						

		devs = self.cfg_file.devices
		for dev in devs.keys():
			print 'dev',dev
			#for row in self.dev_store:
			#	if dev == row[0]:
			#		pass
			dev_mdl = self.device_model()
			stat_mdl = self.status_model()
			if len(dev) == 0:
				lbl = 'Any Device'
			else:	lbl = dev

			self.dev_store.append([dev,dev_mdl,None,True,'',lbl,stat_mdl])
			for opt in devs[dev]:
				for val in devs[dev][opt]:
					for row in dev_mdl.keys():
						if dev_mdl[row][2] == opt:
							#print 'opt', opt, 'row3', dev_mdl[row][3], 'row4', dev_mdl[row][4]
							if   dev_mdl[row][3] == val:
								dev_mdl[row][0] = ON
							elif dev_mdl[row][4] == val:
								dev_mdl[row][0] = OFF
							#else:
							#	dev_mdl[row][0] = DEFAULT
							#print '>', repr(dev_mdl[row])
							#print repr(dev_mdl[row])

			if devs[dev].has_key('name'):
				dev_mdl['bt_devname_txt'][0] = ON
				name = devs[dev]['name'][0]
				dev_mdl['bt_devname_txt'][3] = name
			else:
				dev_mdl['bt_devname_txt'][0] = OFF
				
			if devs[dev].has_key('class'):
				dev_mdl['bt_devclass_tgb'][0] = ON
				cls = devs[dev]['class'][0]
				dev_mdl['bt_devclass_tgb'][3] = cls
			else:
				dev_mdl['bt_devclass_tgb'][0] = OFF
				

	def ui_to_file(self):
		self.ui_to_model()
		self.model_to_file()

	#
	#
	#
	def device_model(self):
		dic = {

			# widget		# active # tri-state #optname #value-if-on/off

			'bt_devname_txt':	[DEFAULT, False, 'name', '', ''],
			'bt_devclass_tgb':	[DEFAULT, False, 'class', '', ''],

			'bt_auth_enable_cb':	[DEFAULT, True, 'auth', 'enable', 'disable'],
			'bt_encrypt_enable_cb':	[DEFAULT, True, 'encrypt', 'enable', 'disable'],

			'bt_iscan_enable_cb':	[DEFAULT, True, 'iscan', 'enable', 'disable'],
			'bt_pscan_enable_cb':	[DEFAULT, True, 'pscan', 'enable', 'disable'],

			'bt_lm_accept_cb':	[DEFAULT, False, 'lm', 'accept', ''],
			'bt_lm_master_cb':	[DEFAULT, False, 'lm', 'master', ''],
	
			'bt_lp_rswitch_cb':	[DEFAULT, False, 'lp', 'rswitch', ''],
			'bt_lp_hold_cb':	[DEFAULT, False, 'lp', 'hold', ''],
			'bt_lp_sniff_cb':	[DEFAULT, False, 'lp', 'sniff', ''],
			'bt_lp_park_cb':	[DEFAULT, False, 'lp', 'park', '']
		}
		for w in dic.keys():
			self[w].tristate = DEFAULT

		return dic

	def status_model(self):
		dic = {
			# widget		# label

			'bt_status_lab':	'Not present',
			'bt_rxbytes_lab':	'N/A',
			'bt_txbytes_lab':	'N/A',
			'bt_address_lab':	'N/A',
			'bt_hciver_lab':	'N/A',
			'bt_lmpver_lab':	'N/A',
			'bt_firmware_lab':	'',
			'bt_manuf_lab':		'N/A',
			'bt_feats_lab':		''
		}
		return dic

	#
	#	constructor
	#
	def __init__(self):

		#
		#	load graphical interface
		#
		self.widgets = gtk.glade.XML('bluetool_gui.glade', 'bt_cfg_wnd')
		handlers = {
			"on_bt_close"			: self.on_close,
			"on_bt_cfgsave_tgb_toggled"	: self.on_toggle_autosave,
			"on_bt_devices_cb_changed"	: self.on_select_device,
			"on_bt_autoinit_cb_clicked"	: self.on_toggle_autoinit,
			"on_bt_pairing_rb_toggled"	: self.on_toggle_pairing,
			"on_bt_secmgr_rb_toggled"	: self.on_toggle_secmgr,
			"on_bt_devname_cb_toggled"	: self.on_toggle_name,
			"on_bt_devname_txt_enter"	: self.on_change_name,
			"on_bt_devclass_cb_toggled"	: self.on_toggle_devclass,
			"on_bt_devclass_tgb_toggled"	: self.on_popup_devclass,
			"on_bt_auth_cb_clicked"		: self.on_toggle_auth,
			"on_bt_encrypt_cb_clicked"	: self.on_toggle_encrypt,
			"on_bt_iscan_cb_clicked"	: self.on_toggle_iscan,
			"on_bt_pscan_cb_clicked"	: self.on_toggle_pscan,
			"on_bt_lm_accept_clicked"	: self.on_toggle_lm_accept,
			"on_bt_lm_master_clicked"	: self.on_toggle_lm_master,
			"on_bt_lp_rswitch_clicked"	: self.on_toggle_lp_rswitch,
			"on_bt_lp_hold_clicked"		: self.on_toggle_lp_hold,
			"on_bt_lp_sniff_clicked"	: self.on_toggle_lp_sniff,
			"on_bt_lp_park_clicked"		: self.on_toggle_lp_park
		}
		self.widgets.signal_autoconnect(handlers)

		#
		#	initialize device class panel
		#
		self.devclass_widgets = gtk.glade.XML('bluetool_gui.glade', 'bt_devclass_dlg')
		devclass_handlers = {
			"on_bt_devclass_update"		: self.on_update_devclass,
			"on_bt_devclass_change"		: self.on_change_devclass,
			"on_bt_devclass_cancel"		: self.on_cancel_devclass,
			"on_bt_devclass_close"		: self.on_close_devclass
		}
		self.devclass_widgets.signal_autoconnect(devclass_handlers)

		dcw = self.devclass_widgets

		#
		svc   = dcw.get_widget('bt_devclass_service_tv')
		svc.get_selection().set_mode(gtk.SELECTION_MULTIPLE)

		self.devclass_wnd = svc.get_parent_window()
		self.devclass_wnd.hide()

		self.devclass_service_store = gtk.ListStore(str)
		svc.set_model(self.devclass_service_store)

		self.devclass_service_col = gtk.TreeViewColumn('Service classes')
		svc.append_column(self.devclass_service_col)

		svc_cell = gtk.CellRendererText()
		self.devclass_service_col.pack_start(svc_cell,True)
		self.devclass_service_col.add_attribute(svc_cell,'text',0)

		for s in device_service_classes :
			if s: self.devclass_service_store.append([s])

		#
		major = dcw.get_widget('bt_devclass_major_tv')

		self.devclass_major_store = gtk.ListStore(str)
		major.set_model(self.devclass_major_store)

		self.devclass_major_col = gtk.TreeViewColumn('Major device classes')
		major.append_column(self.devclass_major_col)

		major_cell = gtk.CellRendererText()
		self.devclass_major_col.pack_start(major_cell,True)
		self.devclass_major_col.add_attribute(major_cell,'text',0)

		for m in device_major_minor_classes:
			self.devclass_major_store.append([m[0]])

		#
		minor = dcw.get_widget('bt_devclass_minor_tv')

		self.devclass_minor_stores = {}
		for ma in device_major_minor_classes:
			self.devclass_minor_stores[ma[0]] = gtk.ListStore(str)
			for mi in ma[1]:
				if mi: self.devclass_minor_stores[ma[0]].append([mi])

		minor.set_model(self.devclass_minor_stores['Computer'])

		self.devclass_minor_col = gtk.TreeViewColumn('Minor device classes')
		minor.append_column(self.devclass_minor_col)

		minor_cell = gtk.CellRendererText()
		self.devclass_minor_col.pack_start(minor_cell,True)
		self.devclass_minor_col.add_attribute(minor_cell,'text',0)

		#
		#	'options' panel
		#
		self.cfg_mdl = {

			# widget		# state	# optname # value-if-on/off

			'bt_autoinit_cb'     :	[False, 'autoinit', 'yes', 'no'],

			'bt_pairing_none_rb' :	[False, 'pairing', 'none', ''],
			'bt_pairing_multi_rb':	[False, 'pairing', 'multi', ''],
			'bt_pairing_once_rb' :	[False, 'pairing', 'once', ''],

			'bt_security_none_rb':	[False, 'security', 'none', ''],
			'bt_security_auto_rb':	[False, 'security', 'auto', ''],
			'bt_security_user_rb':	[False, 'security', 'user', '']
		}
					# address # model # dbus_object # save # name # label # stats_mdl
		self.dev_store = gtk.ListStore(str, object, object, bool, str, str, object)
					# the order is screwed but it's hard to readjust all indexes at code-time
					# and unfortunately we can't use a dictionary since gtk stores are only lists

		dev_cell = gtk.CellRendererText()
		bt_devices_cb = self['bt_devices_cb']
		bt_devices_cb.pack_start(dev_cell,True)
		bt_devices_cb.add_attribute(dev_cell,'text',5)
		bt_devices_cb.set_model(self.dev_store)

		#
		#	open conf file
		#
		self.cfg_file = BluezConfig(HCID_CONF_PATH)

		#
		#	load configuration from file
		#
		self.file_to_model()

		#
		#	create dbus proxy
		#
		self.sys_bus = dbus.SystemBus()
		self.dbus_manager = dbus.Interface(
			self.sys_bus.get_object('org.bluetool','/org/bluetool/manager'),
			'org.bluetool.manager'
		)
		self.dbus_manager.connect_to_signal('DeviceAdded',self.DeviceAdded)
		self.dbus_manager.connect_to_signal('DeviceRemoved',self.DeviceRemoved)
		self.dbus_manager.connect_to_signal('DeviceUp',self.DeviceUp)
		self.dbus_manager.connect_to_signal('DeviceDown',self.DeviceDown)

		self.dev_paths = self.dbus_manager.ListDevices()

		#
		#	load configuration directly from devices (via dbus)
		#
		self.manager_to_model()

		self.model_to_ui()

		bt_devices_cb.set_active(0)


	def __getitem__(self,key):
		return self.widgets.get_widget(key)

	def DeviceAdded(self,address):

		self.dev_paths.append(address)
		self.manager_to_model()
		self.model_to_ui()

	def DeviceRemoved(self,address):

		self['bt_devices_cb'].set_active(0)	# Fixme: this is not always necessary

		self.dev_paths.remove(address)
		self.manager_to_model()
		self.model_to_ui()

	def DeviceDown(self,address):
		self.manager_to_model()
		self.model_to_ui()

	def DeviceUp(self,address):
		self.manager_to_model()
		self.model_to_ui()

	def on_close(self,widget):
		gtk.main_quit()
		
	def on_popup_devclass(self,widget):
		if widget.get_active():
			self.devclass_wnd.show()
		else:
			self.devclass_wnd.hide()

	def on_update_devclass(self,widget):
		print 'on_update_devclass'
		dcw = self.devclass_widgets

		svc   = dcw.get_widget('bt_devclass_service_tv')

		major = dcw.get_widget('bt_devclass_major_tv')

		minor = dcw.get_widget('bt_devclass_minor_tv')

		sel = major.get_selection()
		if sel: it = sel.get_selected()[1]
		mstr = self.devclass_major_store[it][0]

		minor_store = self.devclass_minor_stores[mstr]
		minor.set_model(minor_store)
		sel = minor.get_selection()
		if widget != minor:
			sel.select_iter(minor_store.get_iter_first())


	def on_toggle_devclass(self,widget):
		if self.updating: return
		self.ui_to_file()

	def on_change_devclass(self,widget):
		print 'on_change_devclass'
		if self.updating: return

		self.class_ui_to_model()
		self.model_to_ui();	# shows the new class code in the box
		self.model_to_file();	# saves it on file

		self['bt_devclass_tgb'].set_active(False)

		dev = self.get_curr_dev()
		if not dev:
			return

		bytes = int(self['bt_devclass_tgb'].get_label(), 16)
		sv = ( bytes >> 16 ) & 0xFF
		ma = ( bytes >> 8 ) & 0xFF
		mi = ( bytes ) & 0xFF

		hci = dev['hci']
		hci.SetProperty('class', dbus.Byte(sv), dbus.Byte(ma), dbus.Byte(mi))

	def on_cancel_devclass(self,wiget):
		self['bt_devclass_tgb'].set_active(False)

	def on_close_devclass(self,widget,data):
		self['bt_devclass_tgb'].set_active(False)
		return True

	def on_toggle_autosave(self,widget):
		if self.updating: return

		idx = self['bt_devices_cb'].get_active()

		if idx < 0:
			return
		elif idx == 0: # 'Any' device
			widget.set_active(True) # you can't disable it

	#	if widget.get_active() == False:
			# todo, ask for confirmation
	#		pass

		self.dev_store[idx][3] = widget.get_active()
		self.ui_to_file()

	def on_select_device(self,widget):
		if self.updating: return
		self.model_to_ui()

	def on_toggle_autoinit(self,widget):
		if self.updating: return
		self.ui_to_file()

	def on_toggle_pairing(self,widget):
		if self.updating: return
		self.ui_to_file()

	def on_toggle_secmgr(self,widget):
		if self.updating: return
		self.ui_to_file()

	def on_toggle_name(self,widget):
		self['bt_devname_txt'].set_editable(widget.get_active())

		if self.updating: return

		self.change_name()

	def on_change_name(self,widget):
		print widget.get_text()
		if self.updating: return

		self.change_name()

	def get_curr_dev(self):
		bt_devices_cb = self['bt_devices_cb']
		idx = bt_devices_cb.get_active()
		dev = self.dev_store[(idx,)][2]
		return dev

	def change_name(self):
		dev = self.get_curr_dev()
		if not dev:
			self.ui_to_file()
			return

		hci = dev['hci']
		hci.SetProperty('name', self['bt_devname_txt'].get_text())

		#self.manager_to_model()
		self.ui_to_file()

	def on_toggle_auth(self,widget):
		if self.updating: return

		if widget == self['bt_auth_enable_cb']:
			toggle_tristate(self['bt_auth_enable_cb'])

		dev = self.get_curr_dev()
		if not dev:
			self.ui_to_file()
			return

		hci = dev['hci']
		authb = dbus.Byte(widget.get_active())
		hci.SetProperty('auth_enable',authb)

		self.ui_to_file()
		return True

	def on_toggle_encrypt(self,widget):
		if self.updating: return

		if widget == self['bt_encrypt_enable_cb']:
			toggle_tristate(self['bt_encrypt_enable_cb'])

		dev = self.get_curr_dev()
		if not dev:
			self.ui_to_file()
			return

		hci = dev['hci']
		if widget.get_active():
			encb = dbus.Byte(2)
		else:
			encb = dbus.Byte(0)

		hci.SetProperty('encrypt_mode',encb)

		self.ui_to_file()
		return True

	def change_scan_enable(self):
		dev = self.get_curr_dev()
		if not dev:
			self.ui_to_file()
			return

		hci = dev['hci']
		scan = 0
		if self['bt_iscan_enable_cb'].tristate == DEFAULT:
			scan |= self.dev_store[(0,)][1]['bt_iscan_enable_cb'][0]
		else:
			if self['bt_iscan_enable_cb'].get_active():
				scan |= HCI_SCAN_INQUIRY
		
		if self['bt_pscan_enable_cb'].tristate == DEFAULT:
			scan |= self.dev_store[(0,)][1]['bt_pscan_enable_cb'][0]
		else:
			if self['bt_pscan_enable_cb'].get_active():
				scan |= HCI_SCAN_PAGE

		hci.SetProperty('scan_enable', dbus.Byte(scan))

		#self.manager_to_model()
		self.ui_to_file()

	def on_toggle_iscan(self,widget):
		if self.updating: return

		if widget == self['bt_iscan_enable_cb']:
			toggle_tristate(self['bt_iscan_enable_cb'])

		self.change_scan_enable()
		return True


	def on_toggle_pscan(self,widget):
		if self.updating: return

		if widget == self['bt_pscan_enable_cb']:
			toggle_tristate(self['bt_pscan_enable_cb'])

		self.change_scan_enable()
		return True

	def change_link_mode(self):
		dev = self.get_curr_dev()
		if not dev:
			self.ui_to_file()
			return

		hci = dev['hci']
		lm = 0
		if self['bt_lm_accept_cb'].get_active():
			lm |= HCI_LM_ACCEPT
		if self['bt_lm_master_cb'].get_active():
			lm |= HCI_LM_MASTER

		hci.SetProperty('link_mode', dbus.UInt32(lm))

		#self.manager_to_model()
		self.ui_to_file()

	def on_toggle_lm_accept(self,widget):
		if self.updating: return
		self.change_link_mode()

	def on_toggle_lm_master(self,widget):
		if self.updating: return
		self.change_link_mode()

	def change_link_policy(self):
		dev = self.get_curr_dev()
		if not dev:
			self.ui_to_file()
			return

		hci = dev['hci']
		lp = 0
		if self['bt_lp_rswitch_cb'].get_active():
			lp |= HCI_LP_RSWITCH
		if self['bt_lp_hold_cb'].get_active():
			lp |= HCI_LP_HOLD
		if self['bt_lp_sniff_cb'].get_active():
			lp |= HCI_LP_SNIFF
		if self['bt_lp_park_cb'].get_active():
			lp |= HCI_LP_PARK

		hci.SetProperty('link_policy', dbus.UInt32(lp))

		#self.manager_to_model()
		self.ui_to_file()

	def on_toggle_lp_rswitch(self,widget):
		if self.updating: return
		self.change_link_policy()

	def on_toggle_lp_hold(self,widget):
		if self.updating: return
		self.change_link_policy()

	def on_toggle_lp_sniff(self,widget):
		if self.updating: return
		self.change_link_policy()

	def on_toggle_lp_park(self,widget):
		if self.updating: return
		self.change_link_policy()
#
#	program entry point
#
def main(argv):
	bt = BluetoolCfgPanel()
	gtk.main()

#
if __name__ == '__main__':
	main(sys.argv)
