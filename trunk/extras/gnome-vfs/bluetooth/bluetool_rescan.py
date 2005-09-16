#! /usr/bin/env python
import os
import sys
import dbus
import dbus.glib
import pygtk
pygtk.require('2.0')
import gobject
import gtk

sys.path.append(os.path.abspath(os.getcwd()+'/../../common'))	#TODO
import bluetool

import pdb

class scanner:

	def __init__(self, dev_path):

        	self.window = gtk.Window(gtk.WINDOW_TOPLEVEL)

        	self.window.connect("destroy", self.on_quit)
        	self.window.set_title("Scanning...")
	        self.window.set_border_width(0)
		self.window.set_position(gtk.WIN_POS_CENTER)

	        vbox = gtk.VBox(False, 3)
        	vbox.set_border_width(10)
        	self.window.add(vbox)
        	vbox.show()

		self.progress = gtk.ProgressBar()
		self.progress.set_text('Searching devices, please wait')
        	vbox.pack_start(self.progress, False, False, 0)
	        self.progress.show()
  
        	separator = gtk.HSeparator()
        	vbox.pack_start(separator, False, False, 0)
	        separator.show()

		bbox = gtk.HButtonBox()
        	vbox.pack_start(bbox, False, False, 0)
		bbox.show()

	        button = gtk.Button(stock='gtk-cancel')
        	button.connect("clicked", self.on_quit)
		bbox.add(button)
		button.show()

		w, h = self.window.get_size()
		self.window.set_default_size(w*2, h)
        	#self.window.set_resizable(False)
		self.window.show()

		sys_bus = dbus.SystemBus()
		self.hci = bluetool.get_local_hci(sys_bus, dev_path)
		self.hci.StartInquiry( reply_handler=self.on_quit, error_handler=self.on_scan_error )

		gobject.timeout_add(100, self.on_redraw_progress)

	def on_redraw_progress(self):

		f = self.progress.get_fraction()+0.005
		if f > 1.0: f = 1.0
		self.progress.set_fraction(f)

		return f <= 1.0

	def on_scan_error(self, message):
		print "error",message

		self.on_quit()

	def on_quit(self,*args):

		if self.progress.get_fraction() < 1.0:
			self.hci.CancelInquiry()

		gtk.main_quit()

if __name__ == '__main__':

	if len(sys.argv) >= 2:

		scanner(sys.argv[1])
		gtk.main()
