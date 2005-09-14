#!/usr/bin/env python
import pygtk
pygtk.require('2.0')
import gtk
import egg.trayicon

def on_show_main(widget,section):
	pass

def on_tray_clicked(widget,event):
	if event.button == 1:	#left button
		pass #todo: show main window
	elif event.button == 3:	#right button
		popup = gtk.Menu()

		browser = gtk.MenuItem('Browser')
		browser.connect('activate', on_show_main, 'browser')
		popup.add(browser)

		remote = gtk.MenuItem('Remote devices')
		remote_menu = gtk.Menu()
		remote.set_submenu(remote_menu)
		popup.add(remote)

		conf = gtk.MenuItem('Settings')
		conf.connect('activate', on_show_main, 'settings')
		popup.add(conf)

		about = gtk.MenuItem('About')
		popup.add(about)

		quit = gtk.MenuItem('Quit')
		quit.connect('activate', on_quit)
		popup.add(quit)

		popup.show_all()
		popup.popup(None, None, None, event.button, event.time)

def on_quit(*args):
	gtk.main_quit()
	

icon = egg.trayicon.TrayIcon('Bluetool')
eventbox = gtk.EventBox()
icon.add(eventbox)
eventbox.add(gtk.image_new_from_icon_name('bt-logo', gtk.ICON_SIZE_SMALL_TOOLBAR))
eventbox.connect('button_press_event',on_tray_clicked)
icon.show_all()

gtk.main()
