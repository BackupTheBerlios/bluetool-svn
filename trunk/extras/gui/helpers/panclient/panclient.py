import pygtk
pygtk.require('2.0')
import gtk
import gtk.glade
import dbus

try:	import dbus.glib
except ImportError:
	pass

class helper_panclient:

	def __init__(self,instance):

		self.widgets = gtk.glade.XML('panclient.glade', 'bt_panclient_dlg')
		handlers = {
		}
		self.widgets.signal_autoconnect(handlers)


		
