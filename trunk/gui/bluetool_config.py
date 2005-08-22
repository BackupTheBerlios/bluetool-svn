#! /usr/bin/env python

import sys
import os
import pygtk
pygtk.require('2.0')
import gtk
import gtk.glade
import gobject
import dbus

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


from bluetool_dbus import BluetoolManagerProxy, BluetoolDeviceProxy
from bluetool_parser import BluezConfig

HCID_CONF = 'hcid.conf'

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
	return widget.tristate

#
#	Device class encoding tables (straight from the BT assigned numbers page)
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
#	the main window
#
class BluetoolCfgPanel:

	#
	#	functions to display conf data in the gui and vice versa
	#
	def load_radio(self,opdict,opt):
		if self.updating: return

		self.updating = True

		self['bt_'+opt+'_'+opdict[opt][0]+'_rb'].set_active(True)

		self.updating = False

	def save_radio(self,prop,vals):
		if self.updating: return

		self.updating = True

		for val in vals:
			if self['bt_'+prop+'_'+val+'_rb'].get_active():
				print 'bt_'+prop+'_'+val+'_rb', 'is', True
				self.config.options[prop] = [val]
				break

		self.updating = False

	def save_toggle(self,cdict,prop,val):
		act = self['bt_'+prop+'_'+val+'_cb'].get_active()
		print 'bt_'+prop+'_'+val+'_cb', 'is', act
		if act:
			if not cdict.has_key(prop):
				cdict[prop] = []

			if cdict[prop].count(val) == 0:
				cdict[prop].append(val)

		elif cdict.has_key(prop):
			if cdict[prop].count(val) != 0:
				cdict[prop].remove(val)

				if len(cdict[prop]) == 0:
					cdict.pop(prop)
			
	def save_enabled(self,cdict,prop):
		self.save_toggle(cdict,prop,'enable')

	#
	#	clear the gui
	#
	def clear_device_panel(self):
		self['bt_devname_txt'].set_text('')
		self['bt_devclass_tgb'].set_label('')
		self['bt_auth_enable_cb'].set_active(False)	
		self['bt_encrypt_enable_cb'].set_active(False)
		self['bt_pscan_enable_cb'].set_active(False)
		self['bt_iscan_enable_cb'].set_active(False)
		self['bt_lm_accept_cb'].set_active(False)
		self['bt_lm_master_cb'].set_active(False)
		self['bt_lp_rswitch_cb'].set_active(False)
		self['bt_lp_hold_cb'].set_active(False)
		self['bt_lp_sniff_cb'].set_active(False)
		self['bt_lp_park_cb'].set_active(False)
		self['bt_voice_btn'].set_label(repr(0))
	
	def clear_keys_panel(self):
		pass

	def clear_status_panel(self):
		self['bt_status_lab'].set_text('not present')
		self['bt_rxbytes_lab'].set_text('0')
		self['bt_txbytes_lab'].set_text('0')
		self['bt_address_lab'].set_text('00:00:00:00:00:00')
		self['bt_hciver_lab'].set_text('')
		self['bt_lmpver_lab'].set_text('')
		self['bt_firmware_lab'].set_text('')
		self['bt_manuf_lab'].set_text('')
		self['bt_feats_lab'].set_text('')



	#
	#	reads stored configuration into the gui
	#
	def load_dev_cfg(self,cdict):

		self.updating = True

		self.clear_device_panel()
		self.clear_status_panel()

		print "cdict",cdict

		if cdict.has_key('name'):
			name = cdict['name'][0].strip('"')
		else:	name = ''

		self['bt_devname_txt'].set_text(name)

		if cdict.has_key('class'):
			clss = cdict['class'][0]
		else:	clss = '0x000000'

		self['bt_devclass_tgb'].set_label(clss)

		for key in ('auth','encrypt','iscan','pscan'):
			val = cdict.has_key(key) and cdict[key][0] == 'enable'

			self['bt_'+key+'_enable_cb'].set_active(val)

		if cdict.has_key('lm'):
			for man in ('accept','master'):
				val = cdict['lm'].count(man) != 0
				self['bt_lm_'+man+'_cb'].set_active(val)

		if cdict.has_key('lp'):
			for man in ('rswitch','hold','sniff','park'):
				val = cdict['lp'].count(man) != 0
				self['bt_lp_'+man+'_cb'].set_active(val)

		self.updating = False

	#
	#	...and the other way around
	#
	def read_ui_and_store_cfg(self):
		
		# if autosave disabled, quit
		if not self.dev_liststore.get_value(self.curr_row,3):
			self.config.store()
			return

		self.updating = True

		if self['bt_autoinit_cb'].get_active():
			p = ['yes']
		else:	p = ['no']
		self.config.options['autoinit'] = p

		self.save_radio('pairing',('none','multi','once'))

		self.save_radio('security',('auto','user','none'))

		self.save_enabled(self.curr_dev_cfg,'auth')
		self.save_enabled(self.curr_dev_cfg,'encrypt')
		self.save_enabled(self.curr_dev_cfg,'iscan')
		self.save_enabled(self.curr_dev_cfg,'pscan')
		self.save_toggle(self.curr_dev_cfg,'lm','accept')
		self.save_toggle(self.curr_dev_cfg,'lm','master')
		self.save_toggle(self.curr_dev_cfg,'lp','rswitch')
		self.save_toggle(self.curr_dev_cfg,'lp','hold')
		self.save_toggle(self.curr_dev_cfg,'lp','sniff')
		self.save_toggle(self.curr_dev_cfg,'lp','park')

		self.updating = False

		self.config.store()

	#
	#	reads current configuration directly from the device into the ui
	#
	def live_dev_cfg(self,dev):
		#
		#	first of all, get status, if interface is down, just give up
		#
		self.updating = True

		self.clear_device_panel()
		self.clear_keys_panel()
		self.clear_status_panel()

		up,rx,re,tx,te = dev['stats']
		if not up:
			self['bt_status_lab'].set_text('OFF');
			# the rest was cleared by clear_xxx_panel()

			self.updating = False
			return

		#
		#	get status
		#
		self['bt_status_lab'].set_text('ON')
		self['bt_rxbytes_lab'].set_text(repr(rx))
		self['bt_txbytes_lab'].set_text(repr(tx))

		addr = dev['address']
		self['bt_address_lab'].set_text(addr)

		hci_ver, hci_rev, lmp_ver, lmp_sub, manuf = dev['version_info']
		self['bt_hciver_lab'].set_text(str(hci_ver)+' (rev. '+repr(hci_rev)+')')
		self['bt_lmpver_lab'].set_text(str(lmp_ver)+' (sub. '+repr(lmp_sub)+')')
		self['bt_manuf_lab'].set_text(manuf)

		feats = dev['features']
		#self['bt_feats_lab'].set_text(feats)

		#
		#	get link keys (TODO)
		#

		#
		#	get settings
		#
		name = dev['local_name']
		self['bt_devname_txt'].set_text(name)

		clsb = dev['class']
		clsx = repr(clsb) #todo
		self['bt_devclass_tgb'].set_label(clsx)

		auth = dev['auth_enable']
		self['bt_auth_enable_cb'].set_active(auth)
	
		encrypt = dev['encrypt_mode']
		self['bt_encrypt_enable_cb'].set_active(encrypt)

		scan = dev['scan_enable']

		pscan = scan & HCI_SCAN_PAGE
		self['bt_pscan_enable_cb'].set_active(pscan)

		iscan = scan & HCI_SCAN_INQUIRY
		self['bt_iscan_enable_cb'].set_active(iscan)

		lm = dev['link_mode']

		accept = lm & HCI_LM_ACCEPT
		self['bt_lm_accept_cb'].set_active(accept)

		master = lm & HCI_LM_MASTER
		self['bt_lm_master_cb'].set_active(master)

		lp = dev['link_policy']
		rswitch = lp & HCI_LP_RSWITCH
		self['bt_lp_rswitch_cb'].set_active(rswitch)

		hold = lp & HCI_LP_HOLD
		self['bt_lp_hold_cb'].set_active(hold)

		sniff = lp & HCI_LP_SNIFF
		self['bt_lp_sniff_cb'].set_active(sniff)

		park = lp & HCI_LP_PARK
		self['bt_lp_park_cb'].set_active(park)

		vs = dev['voice_setting']
		self['bt_voice_btn'].set_label(repr(vs))

		self.updating = False

	#
	#	constructor
	#
	def __init__(self):

		#
		#	load graphical interface
		#
		self.widgets = gtk.glade.XML('bluetool-gui.glade', 'bt_cfg_wnd')
		handlers = {
			"on_bt_close"			: self.on_close,
			"on_bt_cfgsave_tgb_toggled"	: self.on_toggle_autosave,
			"on_bt_devices_cb_changed"	: self.on_select_device,
			"on_bt_autoinit_cb_clicked"	: self.on_toggle_autoinit,
			"on_bt_pairing_rb_toggled"	: self.on_toggle_pairing,
			"on_bt_secmgr_rb_toggled"	: self.on_toggle_secmgr,
			"on_bt_devname_txt_key_press"	: self.on_name_changed,
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
		self.devclass_widgets = gtk.glade.XML('bluetool-gui.glade', 'bt_devclass_panel')
		devclass_handlers = {
			"on_bt_devclass_update"		: self.on_update_devclass,
			"on_bt_devclass_change"		: self.on_change_devclass,
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

		self.class_to_ui(0x3e,0x01,0x00) # start with a default value

		#major.pack_start(cell,True)
		#major.add_attribute(cell,'text',1)

		#minor.pack_start(cell,True)
		#minor.add_attribute(cell,'text',1)

		#
		#	init list
		#
		self.dev_liststore = gtk.ListStore(str,str,str,bool,int)
					# path, address, label, autosave, status (-1 non present, 0 off, 1 on)
		cell = gtk.CellRendererText()
		bt_devices_cb = self['bt_devices_cb']
		bt_devices_cb.set_model(self.dev_liststore)
		bt_devices_cb.pack_start(cell,True)
		bt_devices_cb.add_attribute(cell,'text',2)

		#
		#	load hcid configuration
		#
		self.config = BluezConfig(HCID_CONF)
		opts = self.config.options

		#
		#	load default settings
		#
		self.curr_row = self.dev_liststore.append(['','','Any device',True,False])
		self.curr_dev_cfg = None
		self.curr_dev = None

		self.updating = True
		
		self['bt_autoinit_cb'].set_active(opts.has_key('autoinit') and opts['autoinit'][0] == 'yes')

		self.load_radio(opts,'pairing')
		self.load_radio(opts,'security')

		self.updating = False
		
		#
		#	load dbus wrapper
		#
		self.manager = BluetoolManagerProxy(self)

		#
		#	on start, select the default device
		#
		bt_devices_cb.set_active(0)

	def on_device_added(self,dev):
		addr = dev['address']
		name = dev['local_name']
		self.dev_liststore.append([dev.path, addr, name+"\t("+addr+")",False,False]);

	def on_device_removed(self,dev):
		it = self.dev_liststore.get_iter_first()
		while 1:
			it = self.dev_liststore.iter_next(it)
			if it is None: break
			if self.dev_liststore.get_value(it,0) == dev.path:
				#
				#	also, if this is the device we're displaying
				#	at the moment, show another before removal
				#
				self.dev_liststore.remove(it)
				if self['bt_devices_cb'].get_active() < 0:
					self['bt_devices_cb'].set_active(0)
				break
			
	def on_device_up(self,dev):
		it = self.dev_liststore.get_iter_first()
		while 1:
			it = self.dev_liststore.iter_next(it)
			if it is None: break
			if self.dev_liststore.get_value(it,0) == dev.path:
				self.dev_liststore.set_value(it,4,True)
				break

	def on_device_down(self,dev):
		it = self.dev_liststore.get_iter_first()
		while 1:
			it = self.dev_liststore.iter_next(it)
			if it is None: break
			if self.dev_liststore.get_value(it,0) == dev.path:
				self.dev_liststore.set_value(it,4,False)
				break

	#
	#	error reporting
	#
	def bt_popup_error(self,list):
		dialog = gtk.MessageDialog(
			parent         = None,
			flags          = gtk.DIALOG_DESTROY_WITH_PARENT | gtk.DIALOG_MODAL,
			type           = gtk.MESSAGE_ERROR,
			buttons        = gtk.BUTTONS_OK,
			message_format = list[0]
		)
		dialog.connect('response',lambda x,y:dialog.destroy())
		dialog.show()		

	def bt_print_error(self,list):
		print "error: ",list[0]

	def bt_empty_handler(self,list):
		#print list
		pass

	def on_select_device(self,widget):

		idx = widget.get_active()

		if idx < 0: idx = 0	# the default device is always present

		self.curr_row = self.dev_liststore.iter_nth_child(None,idx)
		devpath = self.dev_liststore.get_value(self.curr_row,0)
		devaddr = self.dev_liststore.get_value(self.curr_row,1)
		autosave = self.dev_liststore.get_value(self.curr_row,3)

		self.updating = True

		self['bt_cfgsave_tgb'].set_active(autosave)

		self.updating = False

		print "stored device configurations: ", self.config.devices

		#
		#	do we have already a saved configuration for this device ?
		#
		if self.config.devices.has_key(devaddr):
			self.curr_dev_cfg = self.config.devices[devaddr]

		elif autosave:
			self.curr_dev_cfg = self.config.devices[devaddr] = {}

		else:
			self.curr_dev_cfg = None

		print "curr_cfg",self.curr_dev_cfg

		#
		#	is the device present right now ?
		#
		if self.manager.devices.has_key(devpath):
			self.curr_dev = self.manager.devices[devpath]
		else:
			self.curr_dev = None

		#if self.updating: return

		if self.curr_dev_cfg:
			self.load_dev_cfg(self.curr_dev_cfg)
		else:
			self.live_dev_cfg(self.curr_dev)

	def on_close(self,widget):
		gtk.main_quit()

	def on_toggle_autosave(self,widget):
		if self.updating: return

		self.updating = True

		if self['bt_devices_cb'].get_active() == 0: # 'Any' device
			widget.set_active(True)

		self.dev_liststore.set_value(self.curr_row,3,widget.get_active())

		devpath = self.dev_liststore.get_value(self.curr_row,0)
		addr = self.dev_liststore.get_value(self.curr_row,1)
		autosave = self.dev_liststore.get_value(self.curr_row,3)

		if self.config.devices.has_key(addr):

			if autosave:
				self.curr_dev_cfg = self.config.devices[addr]
			else:
				self.config.devices.pop(addr)
				self.curr_dev_cfg = None

		else:
			if autosave:
				self.curr_dev_cfg = self.config.devices[addr] = {}
			else:
				self.curr_dev_cfg = None

		self.read_ui_and_store_cfg()

		self.updating = False

	def on_toggle_autoinit(self,widget):
		if self.updating: return

		self.read_ui_and_store_cfg()

	def on_toggle_pairing(self,widget):
		if self.updating: return
		if not widget.get_active(): return

		self.read_ui_and_store_cfg()

	def on_toggle_secmgr(self,widget):
		if self.updating: return
		if not widget.get_active(): return

		self.read_ui_and_store_cfg()

	def on_name_changed(self,widget):
		pass

	def on_popup_devclass(self,widget):
		#self.devclass.re(widget.get_text())
		if widget.get_active():
			self.devclass_wnd.show()
		else:
			self.devclass_wnd.hide()
		pass

	def class_to_ui(self,service_mask,major_id,minor_id):
		print "0x%02X%02X%02X" % (service_mask,major_id,minor_id)
		dcw = self.devclass_widgets

		svc   = dcw.get_widget('bt_devclass_service_tv')

		sel   = svc.get_selection()
		sel.unselect_all()
		for bit in range(0,7):
			if service_mask & (1 << bit):
				sel.select_iter(self.devclass_service_store.iter_nth_child(None,bit))

		major = dcw.get_widget('bt_devclass_major_tv')

		minor = dcw.get_widget('bt_devclass_minor_tv')

		sel = major.get_selection()
		it = self.devclass_major_store.iter_nth_child(None,major_id)
		sel.select_iter(it)

		sel = minor.get_selection()
		print self.devclass_major_store[it][0]
		store = self.devclass_minor_stores[self.devclass_major_store[it][0]]
		sel.select_iter(store.iter_nth_child(None,minor_id))

	def ui_to_class(self):
		return '0'

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
		pass

	def on_change_devclass(self,widget):
		self['bt_devclass_tgb'].set_label(self.ui_to_class())
		self['bt_devclass_tgb'].set_active(False)

	def on_close_devclass(self,widget,data):
		self['bt_devclass_tgb'].set_active(False)
		return True

	def set_dev_auth(self):
		if self.curr_dev is None: return
		auth = self['bt_auth_enable_cb'].get_active()
		self.curr_dev.hci.SetProperty('auth_enable', dbus.Byte(auth),
			reply_handler=self.bt_empty_handler, error_handler=self.bt_popup_error)

	def on_toggle_auth(self,widget):
		if self.updating: return
		
		self.read_ui_and_store_cfg()

	def set_dev_encrypt(self):
		if self.updating: return

		self.read_ui_and_store_cfg()

		if self['bt_encrypt_enable_cb'].get_active():
			enc = 2
		else:
			enc = 0
		self.curr_dev.hci.SetProperty('encrypt_mode', dbus.Byte(enc),
			reply_handler=self.bt_empty_handler, error_handler=self.bt_popup_error)

	def on_toggle_encrypt(self,widget):
		if self.updating: return

		self.read_ui_and_store_cfg()

	def set_dev_scan(self):
		if self.curr_dev is None: return
		scan = 0
		if self['bt_iscan_enable_cb'].get_active():
			scan |= HCI_SCAN_INQUIRY
		if self['bt_pscan_enable_cb'].get_active():
			scan |= HCI_SCAN_PAGE
		self.curr_dev.hci.SetProperty('scan_enable', dbus.Byte(scan),
			reply_handler=self.bt_empty_handler, error_handler=self.bt_popup_error)

	def on_toggle_iscan(self,widget):
		if self.updating: return

		self.set_dev_scan()
		self.read_ui_and_store_cfg()

	def on_toggle_pscan(self,widget):
		if self.updating: return

		self.set_dev_scan()
		self.read_ui_and_store_cfg()

	def set_dev_lm(self):
		if self.curr_dev is None: return
		lm = 0
		if self['bt_lm_accept_cb'].get_active():
			lm |= HCI_LM_ACCEPT
		if self['bt_lm_master_cb'].get_active():
			lm |= HCI_LM_MASTER
		self.curr_dev.hci.SetProperty('link_mode', dbus.UInt32(lm),
			reply_handler=self.bt_empty_handler, error_handler=self.bt_popup_error)

	def on_toggle_lm_accept(self,widget):
		if self.updating: return

		self.set_dev_lm()
		self.read_ui_and_store_cfg()

	def on_toggle_lm_master(self,widget):
		if self.updating: return

		self.set_dev_lm()
		self.read_ui_and_store_cfg()

	def set_dev_lp(self):
		if self.curr_dev is None: return
		lp = 0
		if self['bt_lp_rswitch_cb'].get_active():
			lp |= HCI_LP_RSWITCH
		if self['bt_lp_hold_cb'].get_active():
			lp |= HCI_LP_HOLD
		if self['bt_lp_sniff_cb'].get_active():
			lp |= HCI_LP_SNIFF
		if self['bt_lp_park_cb'].get_active():
			lp |= HCI_LP_PARK
		self.curr_dev.hci.SetProperty('link_policy', dbus.UInt32(lp),
			reply_handler=self.bt_empty_handler, error_handler=self.bt_popup_error)

	def on_toggle_lp_rswitch(self,widget):
		if self.updating: return

		self.set_dev_lp()
		self.read_ui_and_store_cfg()

	def on_toggle_lp_hold(self,widget):
		if self.updating: return

		self.set_dev_lp()
		self.read_ui_and_store_cfg()

	def on_toggle_lp_sniff(self,widget):
		if self.updating: return

		self.set_dev_lp()
		self.read_ui_and_store_cfg()

	def on_toggle_lp_park(self,widget):
		if self.updating: return

		self.set_dev_lp()
		self.read_ui_and_store_cfg()

	def __getitem__(self,key):
		return self.widgets.get_widget(key)


#
#	program entry point
#
def main(argv):
	bt = BluetoolCfgPanel()
	gtk.main()

#
if __name__ == '__main__':
	main(sys.argv)
