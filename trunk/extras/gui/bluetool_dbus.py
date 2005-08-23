import dbus
import gtk

if getattr(dbus, 'version', (0,0,0)) >= (0,41,0):
    import dbus.glib

class BluetoolDeviceProxy:
	def __init__(self,dbus_path):
		self.bus = dbus.SystemBus()
		self.obj = self.bus.get_object('org.bluetool',dbus_path)
		self.hci = dbus.Interface(self.obj, 'org.bluetool.device.hci')
		self.path = dbus_path
		self.up = False

	def __getitem__(self,key):
		proplist = self.hci.GetProperty(key)
		if len(proplist) == 2:
			return proplist[1]
		else:
			return proplist[1:]

class BluetoolManagerProxy:
	def __init__(self,view):
		try:
			#
			#	create proxy for remote object (the manager)
			#
			self.bus = dbus.SystemBus()
			self.obj = self.bus.get_object('org.bluetool','/org/bluetool/manager')
			self.iface = dbus.Interface(self.obj,'org.bluetool.manager')

			#
			#	bind signals to local hook functions
			#
			self.iface.connect_to_signal('DeviceAdded',self.DeviceAdded);
			self.iface.connect_to_signal('DeviceRemoved',self.DeviceRemoved);
			self.iface.connect_to_signal('DeviceUp',self.DeviceUp);
			self.iface.connect_to_signal('DeviceDown',self.DeviceUp);

			#
			#	get a list of devices
			#
			self.view = view

			self.devices = {}
			devlist = self.iface.ListDevices()
			for devpath in devlist:
				self.DeviceAdded(devpath) 	
				# WARNING: it's not nice to call an event handler explicitly

		except dbus.dbus_bindings.DBusException:
			dialog = gtk.MessageDialog(
				parent         = None,
				flags          = gtk.DIALOG_DESTROY_WITH_PARENT | gtk.DIALOG_MODAL,
				type           = gtk.MESSAGE_ERROR,
				buttons        = gtk.BUTTONS_OK,
				message_format = "Unable to connect to Bluetool daemon"
			)
			dialog.connect('response',lambda x,y: gtk.main_quit())
			dialog.show()
			
	#
	#	signal handlers
	#
	def DeviceAdded(self,devpath):
		print "-> added device @",devpath
		dev = BluetoolDeviceProxy(devpath)
		self.devices[devpath] = dev

		self.view.on_device_added(dev)

	def DeviceRemoved(self,devpath):
		if self.devices.has_key(devpath):
			print "-> removed device @",devpath
			self.view.on_device_removed(self.devices[devpath])
			self.devices.pop(devpath)

	def DeviceUp(self,devpath):
		print "-> enabled device @",devpath
		if self.devices.has_key(devpath):
			self.devices[devpath].up = True
			self.view.on_device_up(self.devices[devpath])
		else: print "error: device not listed"

	def DeviceDown(self,devpath):
		print "-> disabled device @",devpath
		if self.devices.has_key(devpath):
			self.devices[devpath].up = False
			self.view.on_device_down(self.devices[devpath])
		else: print "error: device not listed"
	#
	#	signal hooks
	#
#	def on_device_added(self): pass
#	def on_device_removed(self): pass

	#
	#	method proxies
	#
#	def ListDevices(self):
#		print self.iface.ListDevices()

#	def method(self,name):
#		return self.obj.ProxyMethod\
#		(self.bus,self.service,self.path,self.iface,name)
