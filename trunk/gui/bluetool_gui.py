#!/usr/bin/env python
import sys
import os
import pygtk
pygtk.require('2.0')
import gtk
import gtk.glade
import gobject
import dbus

from bluetool_dbus import BluetoolManagerProxy

#
#	main window
#
class BluetoolWindow:
	''' '''
	def __init__(self):
		#
		#	load gui
		#
		self.widgets = gtk.glade.XML('bluetool-gui.glade', 'bt_main_wnd')
		handlers = {
			"on_bt_close"			: self.on_close,
			"on_bt_status_exp_activate"	: self.on_property_expand,
			"on_bt_host_exp_activate"	: self.on_property_expand,
			"on_bt_devices_cb_changed"	: self.on_select_device,
			"on_bt_setname_btn_clicked"	: self.on_change_name,
			"on_bt_devclass_cb_changed"	: self.on_change_class,
			"on_bt_iscan_chk_toggled"	: self.on_change_scan_enable,
			"on_bt_pscan_chk_toggled"	: self.on_change_scan_enable,
			"on_bt_inquiry_btn_clicked"	: self.on_inquiry_start,
			"on_bt_inqcancel_btn_clicked"	: self.on_inquiry_cancel
		}
		self.widgets.signal_autoconnect(handlers)
	#	self.updater=gobject.timeout_add(2000,self._update_stats)

		#
		#	load dbus wrapper
		#
		self.manager = BluetoolManagerProxy(self)

		#
		#	init browser panel
		#		
		self.inquiry_runner = None
		self.inquiry_progress = gtk.ProgressBar()
		bt_progress_box = self['bt_progress_box']
		bt_progress_box.pack_start(self.inquiry_progress,True,True)
		bt_progress_box.show_all()

		#
		#	init properties panel
		#
		self.close_property_expanders()

		#
		#	load device list
		#
		self.devlist = gtk.ListStore (str)
		cell = gtk.CellRendererText()
		bt_devices_cb = self['bt_devices_cb']
		bt_devices_cb.set_model(self.devlist)
		bt_devices_cb.pack_start(cell,True)
		bt_devices_cb.add_attribute(cell,'text',0)

		self.frozen = False

		for dev in self.manager.devices:
			self.on_device_added(dev)
			#bt_devices_cb.append_text ( dev['local_name']+"\t("+dev['address']+")" )

		#bt_devices_cb.set_active(0)

	def __getitem__(self,key):
		return self.widgets.get_widget(key)


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
		dialog.connect('response',lambda a,b:dialog.destroy())
		dialog.show()		

	def bt_print_error(self,list):
		print "error: ",list[0]

	#
	#	dbus signals
	#


	#
	#	gui handlers
	#
	#	some notes about the naming convention:
	#
	#	on_* is for GUI callbacks
	#	cb_* is for BluetoolManager callbacks
	#	*_hdl is for DBus reply handlers
	#
	def on_close(self,widget):
		gtk.main_quit()

	def get_dev(self):
		index = self['bt_devices_cb'].get_active()
		if index < 0 or index >= len(self.manager.devices): 
			return None
		else:
			return self.manager.devices[index]

	def close_property_expanders(self):
		for ename in ( 'bt_status_exp','bt_host_exp' ):
			self[ename].set_expanded(False)

	def on_property_expand(self,widget):
		dev = self.get_dev()
		if dev is None: widget.set_expanded(True)

	def update_stats_hdl(self,*args):
		active, rx_bytes, rx_errors, tx_bytes, tx_errors = args[1:]
		if active:
			self['bt_status_lab'].set_text("Radio ON")
			self['bt_rxbytes_lab'].set_text(repr(rx_bytes))
			#if stats[4] : t_string += repr(stats[4])
			self['bt_txbytes_lab'].set_text(repr(tx_bytes))
		else:
			self['bt_status_lab'].set_text("Radio OFF")
			self['bt_rxbytes_lab'].set_text("")
			self['bt_txbytes_lab'].set_text("")

	def update_stats(self):
		dev = self.get_dev()
		if dev is not None:
			dev.hci.GetProperty('stats',
				reply_handler=self.update_stats_hdl,error_handler=self.bt_print_error)
		return True

	def on_change_name(self,widget):
		dev = self.get_dev()
		if dev is None: return
		if self.frozen: return
	
		self.frozen = True

		bt_name_txt = self['bt_name_txt']

		name = bt_name_txt.get_text()
		if len(name) > 248: name = name[:248]
		dev.hci.SetProperty('local_name',dbus.String(name))
		name = dev['local_name']
		bt_name_txt.set_text(name)

		bt_devices_cb = self['bt_devices_cb']
		index = bt_devices_cb.get_active()

		bt_devices_cb.remove_text(index)
		bt_devices_cb.insert_text (index,name+"\t("+dev['address']+")")
		bt_devices_cb.set_active(index)

		self.frozen = False	
	#
	#	translates class numbers to readable strings
	#
	def _classid_to_string(major,minor):
		table = \
		[
			["Unknown"],
			# computer
			["Unknown", "Workstation", "Server", "Laptop", "Handheld", "Palm" "Wearable"],
			# phone
			["Unknown", "Cellular", "Cordless", "Smartphone", 
			"Wired modem or voice gateway", "ISDN access", "Sim card reader"],
			# lan
			["Unknown", "1-17% available", "17-33% available", "33-50% available",
			"50-67 available%", "67-83 available%", "83-99 available%", "Not available"],
			# audio/video
			["Unknown", "Headset", "Hands-free", "Microphone", "Loudspeaker", "Headphones",
			"Portable audio", "Car audio", "Set-top box", "HIFI audio", "VCR", "Videocamera",
			"Camcorder", "Video monitor", "Video display and loudspeaker", "Video conferencing",
			"", "Gaming" ],
			# peripherial
			[],
			# imaging
			["Unknown","Display","Camera","Scanner","Printer"]
			#            2^2 -->
		]
		try:
			return table[major][minor]
		except IndexError:
			return "Unknown"

	def on_change_class(self,widget):
		dev = self.get_dev()
		if dev is None: return
		if self.frozen: return
		bt_class_cb = self['bt_devclass_cb']

		major = 1 #computer
		selected = widget.get_active_text()
		if   selected == "computer":	minor = 1
		elif selected == "laptop":	minor = 3
		elif selected == "handheld":	minor = 4
		else:				minor = 0
		dev.hci.SetProperty(
			"class", dbus.Byte(major), dbus.Byte(minor), dbus.Byte(0)
		)

	def update_scan_enable(self):
		dev = self.get_dev()
		if dev is None: return

		self.frozen = True

		enable = dev['scan_enable']

		bt_pscan_chk = self['bt_pscan_chk']
		bt_iscan_chk = self['bt_iscan_chk']

		b_iscan = enable & 1 != 0
		bt_iscan_chk.set_active(b_iscan)

		b_pscan = enable & 2 != 0
		bt_pscan_chk.set_active(b_pscan)

		self.frozen = False

	def change_scan_enable_hdl(self,enable):
		self.update_scan_enable()

	def on_change_scan_enable(self,widget):
		dev = self.get_dev()
		if dev is None: return
		if self.frozen: return

		bt_pscan_chk = self['bt_pscan_chk']
		bt_iscan_chk = self['bt_iscan_chk']

		flags = 0
		if bt_iscan_chk.get_active() : flags |= 1
		if bt_pscan_chk.get_active() : flags |= 2

		dev.hci.SetProperty('scan_enable',dbus.Byte(flags),
			reply_handler=self.change_scan_enable_hdl, error_handler=self.bt_popup_error)

	def on_device_added(self,dev):
		bt_devices_cb = self['bt_devices_cb']
		bt_devices_cb.append_text( dev['local_name']+"\t("+dev['address']+")" )
		aindex = bt_devices_cb.get_active()
		if aindex == -1:
			bt_devices_cb.set_active(len(self.manager.devices)-1) #ugly!
		#self.on_select_device(None) # the parameter is ignored anyway

	def on_device_removed(self,index):
		bt_devices_cb = self['bt_devices_cb']
		aindex = bt_devices_cb.get_active()
		bt_devices_cb.remove_text(index)
		if index == aindex:
			bt_devices_cb.set_active(0)
		elif aindex == -1:
			self.close_property_expanders()
		#self.on_select_device(None) # the parameter is ignored anyway

	def on_select_device(self,widget):
		dev = self.get_dev()
		if dev is None: return
		if self.frozen: return

		self.frozen = True
		#
		#	fill the 'properties' panel
		#
		#try:
		#	gobject.source_remove(self.stats_refresh)
		#except AttributeError:
		#	pass

		#self.stats_refresh=gobject.timeout_add(5000,lambda: self._update_stats)
		if not self.update_stats(): #device is active
			return
			
		self['bt_name_txt'].set_text(dev['local_name'])

		#
		#	device class
		#
		dclass = dev['class']
		cidx = 0
		if dclass[0] == 1:
			if   dclass[1] == 1: cidx = 1 #computer
			elif dclass[1] == 3: cidx = 2 #laptop
			elif dclass[1] == 4: cidx = 4 #handheld

		self['bt_devclass_cb'].set_active(cidx)		

		self['bt_hwaddr_lab'].set_text(dev['address'])
		version_info = dev['version_info']
		self['bt_manuf_lab'].set_text(version_info[4])
		self['bt_version_lab'].set_text(version_info[0]+" / "+version_info[2]+" / ---")#+dev['firmware'])

		#feat_list = gtk.ListStore (str)
		#cell = gtk.CellRendererText ()
		#bt_feats_cb = self['bt_feats_cb']
		#bt_feats_cb.clear()
		#bt_feats_cb.set_model(feat_list)
		#bt_feats_cb.pack_start(cell,True)
		#bt_feats_cb.add_attribute (cell,'text',0)

		#feat_strings = dev['features'].replace("> <","|").strip("<>").split("|")
		#for feat in feat_strings:
		#	bt_feats_cb.append_text(feat)

		self.update_scan_enable()

		self.frozen = False

	#
	#	device inquiry
	#
	def cb_update_inquiry_progress(self):
		f = self.inquiry_progress.get_fraction()+0.005
		if f > 1.0: f = 1.0
		self.inquiry_progress.set_fraction(f)
		self.inquiry_progress.set_text("Scanning...")
		return f != 1.0

	def inquiry_complete_hdl(self,list):
		self.inquiry_progress.set_text("Complete")
		gobject.source_remove(self.inquiry_runner)

	def on_inquiry_start(self,widget):
		dev = self.get_dev()
		if dev is None: return

		self.inquiry_progress.set_fraction(0)
		if self.inquiry_runner is not None:
			gobject.source_remove(self.inquiry_runner)
		self.inquiry_runner = gobject.timeout_add(100,self.cb_update_inquiry_progress)
		#todo: remove runner when not needed or is this automatic ?
		print dev.hci.StartInquiry(
			reply_handler=self.inquiry_complete_hdl, error_handler=self.bt_popup_error)

	def inquiry_canceled_hdl(self,list):
		gobject.source_remove(self.inquiry_runner)		
		self.inquiry_progress.set_text("Canceled")
		self.inquiry_progress.set_fraction(0)

	def on_inquiry_cancel(self,widget):
		dev = self.get_dev()
		if dev is None: return

		dev.hci.CancelInquiry(
			reply_handler=self.inquiry_canceled_hdl, error_handler=self.bt_popup_error)

#
#	program entry point
#
def main(argv):
	bt = BluetoolWindow()
	gtk.main()

#
if __name__ == '__main__':
	main(sys.argv)
