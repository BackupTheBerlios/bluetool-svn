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

	def __getitem__(self,key):
		proplist = self.hci.GetProperty(key)
		if len(proplist) == 2:
			return proplist[1]
		else:
			return proplist[1:]

class BluetoolManagerProxy:
	def __init__(self,view):
		try:
			self.bus = dbus.SystemBus()
			self.obj = self.bus.get_object('org.bluetool','/org/bluetool/manager')
			self.iface = dbus.Interface(self.obj,'org.bluetool.manager')

			self.devices = []
			devlist = self.iface.ListDevices()
			for devpath in devlist:
				print "-> found device object @", devpath
				self.devices.append( BluetoolDeviceProxy(devpath) )

			self.view = view

		except dbus.dbus_bindings.DBusException:
			dialog = gtk.MessageDialog(
				parent         = None,
				flags          = gtk.DIALOG_DESTROY_WITH_PARENT | gtk.DIALOG_MODAL,
				type           = gtk.MESSAGE_ERROR,
				buttons        = gtk.BUTTONS_OK,
				message_format = "Unable to connect to Bluetool daemon"
			)
			dialog.connect('response',lambda a,b: gtk.main_quit())
			dialog.show()
			
		#
		#	bind signals to local hook functions: TODO
		#
		#self.bus.add_signal_receiver(self.DeviceAdded,self.iname,self.sname,self.pname)
		#self.bus.add_signal_receiver(self.DeviceRemoved,'DeviceRemoved',self.iname)
		self.iface.connect_to_signal('DeviceAdded',self.DeviceAdded);
		self.iface.connect_to_signal('DeviceRemoved',self.DeviceRemoved);

	#
	#	signal handlers
	#
	def DeviceAdded(self,devpath):
		print "-> added device @",devpath
		dev = BluetoolDeviceProxy(devpath)
		self.devices.append(dev)

		self.view.on_device_added(dev)

	def DeviceRemoved(self,devpath):
		for dev in self.devices:
			if dev.path == devpath:
				i = self.devices.index(dev)
				del(self.devices[i])
				print "-> removed device @",dev.path

				self.view.on_device_removed(i)


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
