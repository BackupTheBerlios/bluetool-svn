import sys
import os
import re
import time
import gnomevfs
import gobject
import dbus

try: import dbus.glib
except:	pass

import traceback
import pdb

sys.path.append(os.path.abspath(os.getcwd()+'/../../common'))	#TODO
import bluetool

sys_bus = dbus.SystemBus()

#
#	the idea of faking .desktop files comes from gnome-vfs in Edd Dumbill's gnome-bluetooth
#

DEVICE_FILE_PROTOTYPE = \
'''
[Desktop Entry]
Version=1.0
Encoding=UTF-8
Type=Link
Name=Bluetooth on %s
Icon=%s
URL=bluetooth:///%s/
'''

REMDEV_FILE_PROTOTYPE = \
'''
[Desktop Entry]
Version=1.0
Encoding=UTF-8
Type=Link
Name=%s
Icon=%s
URL=bluetooth:///%s/%s/
'''

REMSVC_FILE_PROTOTYPE = \
'''
[Desktop Entry]
Version=1.0
Encoding=UTF-8
Type=Link
Name=%s
Icon=%s
'''

HWADDR_RE = re.compile('^([0-9a-fA-F][0-9a-fA-F]:){5}([0-9a-fA-F][0-9a-fA-F])$')

class VfsDesktopFileBase:

	def __init__(self):

		self.contents = ''
		self.bread = 0

	def get_file_info(self, file_info):

		file_info.name = self.basename + '.desktop'
		file_info.mime_type = 'application/x-gnome-app-info'
		file_info.type = gnomevfs.FILE_TYPE_REGULAR
		file_info.permissions = 0555
		file_info.size = len(self.contents)
		file_info.mtime = long(time.time())
		file_info.ctime = file_info.mtime
		file_info.atime = file_info.mtime

	def read(self, buff, num_bytes, context):

		l = len(self.contents)

		if self.bread >= l:
			raise gnomevfs.EOFError
			#return 0

		buff[:l] = self.contents

		self.bread += l

		return l

class VfsDirectoryBase:
	
	def __init__(self):
		self.index = 0

	def get_file_info(self, file_info):

		file_info.name = self.basename
		file_info.mime_type = 'x-directory/normal'
		file_info.type = gnomevfs.FILE_TYPE_DIRECTORY
		file_info.permissions = 0555
		file_info.mtime = long(time.time())
		file_info.ctime = file_info.mtime
		file_info.atime = file_info.mtime

def get_root():
	return RootDirectoryHandle()

class RootDirectoryHandle(VfsDirectoryBase):

	def __init__(self):
		VfsDirectoryBase.__init__(self)

		print 'RootDirectoryHandle()'

		self.uri = gnomevfs.URI('bluetooth:///')

		self.basename = self.uri.path

		self.mgr = bluetool.get_manager(sys_bus)

		self.devs = self.mgr.ListDevices()

	def read_dir(self, file_info):

		try:
			dev_path = self.devs[self.index]
			DeviceFileHandle(dev_path).get_file_info(file_info)

			self.index += 1

		except IndexError:
			os.environ[r'BLUETOOTH_DIR_LAST_OPEN'] = self.uri.path
			raise gnomevfs.EOFError

	def open_device_file(self, uri):

		address = uri.short_name[:-8]

		#print 'open_device_file: address =',address, 'self =', self

		print 'devs=',self.devs

		for dev_path in self.devs:
			hci = bluetool.get_local_hci(sys_bus, dev_path)
			if hci.GetProperty('address').replace(':','') == address:
				return DeviceFileHandle(dev_path)

		raise gnomevfs.NotFoundError

	def open_device_dir(self, uri):

		for dev_path in self.devs:
			hci = bluetool.get_local_hci(sys_bus, dev_path)
			if hci.GetProperty('address') == uri.short_name:
				return DeviceDirectoryHandle(uri, dev_path)

		raise gnomevfs.NotFoundError

class DeviceFileHandle(VfsDesktopFileBase):

	def __init__(self, dev_path):
		VfsDesktopFileBase.__init__(self)

		print 'DeviceFileHandle(',dev_path,')'

		self.hci = bluetool.get_local_hci(sys_bus, dev_path)
		
		self.name = self.hci.GetProperty('name')
		self.address = self.hci.GetProperty('address')
		if len(self.name) == 0:
			self.name = self.address

		self.contents = DEVICE_FILE_PROTOTYPE % (
					self.name,
					'file:///usr/share/pixmaps/bt-logo.png',
					self.address
				)
		self.basename = self.address.replace(':','')
		self.bread = 0

class DeviceDirectoryHandle(VfsDirectoryBase):

	def __init__(self, uri, dev_path):
		VfsDirectoryBase.__init__(self)

		print 'DeviceDirectoryHandle(',dev_path,')'

		self.uri = uri
		self.dev_path = dev_path

		self.basename = uri.short_name

		self.hci = bluetool.get_local_hci(sys_bus, dev_path)
		
		#self.name = self.hci.GetProperty('name')
		self.address = self.hci.GetProperty('address')
		
		self.devs = self.hci.InquiryCache()

	def read_dir(self, file_info):

		try:

			if self.index == 0 and os.getenv(r'BLUETOOTH_DIR_LAST_OPEN') == self.uri.path:
				print 'reloading URI', self.uri

				#	this should not be needed, but fixes a strange
				#	behaviour in nautilus, which would cause the
				#	external program (see below) to be run more than once
				#
				os.environ[r'BLUETOOTH_DIR_LAST_OPEN'] = ''

				#
				#	XXX: calling an external program sucks
				#
				os.system('./bluetool_rescan.py '+self.dev_path)
				print 'done reloading'
				
				self.devs = self.hci.InquiryCache()

			rem_path = self.devs[self.index]
			RemDevFileHandle(rem_path, self).get_file_info(file_info)

			self.index += 1

		except IndexError:
			os.environ[r'BLUETOOTH_DIR_LAST_OPEN'] = self.uri.path
			raise gnomevfs.EOFError

	def open_remdev_file(self, uri):

		print 'open_remdev_file(', uri, ')'

		address = uri.short_name[:-8]

		for rem_path in self.devs:
			hci = bluetool.get_remote_hci(sys_bus, rem_path)
			if hci.GetProperty('address').replace(':','') == address:
				return RemDevFileHandle(rem_path, self)

		raise gnomevfs.NotFoundError

	def open_remdev_dir(self, uri):

		for rem_path in self.devs:
			hci = bluetool.get_remote_hci(sys_bus, rem_path)
			if hci.GetProperty('address') == uri.short_name:
				return RemDevDirectoryHandle(uri, rem_path)

		raise gnomevfs.NotFoundError

class RemDevFileHandle(VfsDesktopFileBase):

	def __init__(self, rem_path, dir_handle):
		VfsDesktopFileBase.__init__(self)

		print 'RemDevFileHandle(',rem_path,')'

		self.hci = bluetool.get_remote_hci(sys_bus, rem_path)
		#self.sdp = bluetool.get_remote_sdp(sys_bus, rem_path)
		
		self.name = self.hci.GetProperty('name')
		self.address = self.hci.GetProperty('address')
		if len(self.name) == 0:
			self.name = self.address

		svc, ma, mi = self.hci.GetProperty('class')

		icon_path = 'file://'+bluetool.devclass2icon(ma)

		print icon_path

		self.contents = REMDEV_FILE_PROTOTYPE % (
					self.name,
					icon_path,
					dir_handle.address,
					self.address
				)
		self.basename = self.address.replace(':','')
		self.bread = 0

class RemDevDirectoryHandle(VfsDirectoryBase):

	def __init__(self, uri, rem_path):
		VfsDirectoryBase.__init__(self)

		print 'RemDevDirectoryHandle(',rem_path,')'

		self.uri = uri
		self.dev_path = rem_path

		self.basename = uri.short_name

		self.hci = bluetool.get_remote_hci(sys_bus, rem_path)
		self.sdp = bluetool.get_remote_sdp(sys_bus, rem_path)

		self.records = self.sdp.SearchAllRecordsCached()[1]

	def read_dir(self, file_info):

		try:
			rec_path = self.records[self.index]

			RemSvcFileHandle(rec_path, self).get_file_info(file_info)

			self.index += 1

		except IndexError:
			os.environ[r'BLUETOOTH_DIR_LAST_OPEN'] = self.uri.path
			raise gnomevfs.EOFError

	def open_remsvc_file(self, uri):

		handle = uri.short_name[:-8]

		for rec_path in self.records:
			rec = bluetool.get_record(sys_bus, rec_path)
			if str(rec.GetHandle()) == handle:
				return RemSvcFileHandle(rec_path, self)

		raise gnomevfs.NotFoundError

class RemSvcFileHandle(VfsDesktopFileBase):

	def __init__(self, rec_path, dir_handle):
		VfsDesktopFileBase.__init__(self)

		print 'RemSvcFileHandle(',rec_path,')'

		self.record = bluetool.get_record(sys_bus, rec_path)

		self.basename = str(self.record.GetHandle())

		ids = self.record.GetClassIdList()
		name = bluetool.SDP_SVCLASS_IDS[ids[0]]

		self.contents = REMSVC_FILE_PROTOTYPE % (
					name,
					'file:///usr/share/pixmaps/bt-logo.png'
				)

def uri2handle(uri):

	print 'uri2handle:',uri

	if uri.short_name == '/':
		return get_root()

	parent = uri.parent

	pes = uri.path.split('/')

	if len(pes) == 0:
		return None

	handle = None

	uri = gnomevfs.URI('bluetooth://')

	for i, pe in enumerate( pes ):

		uri = uri.append_string(pe)

		if i == 0:
			handle = get_root()

		if i == 1:
			if HWADDR_RE.match(pe):

				handle = handle.open_device_dir(uri)

			elif pe.endswith('.desktop'):

				handle = handle.open_device_file(uri)
				break
			else:	
				raise gnomevfs.NotFoundError

		if i == 2:
			if HWADDR_RE.match(pe):

				handle = handle.open_remdev_dir(uri)

			elif pe.endswith('.desktop'):

				handle = handle.open_remdev_file(uri)
				break
			else:	
				raise gnomevfs.NotFoundError

		if i == 3:
			if pe.endswith('.desktop'):

				handle = handle.open_remsvc_file(uri)
				break
			else:	
				raise gnomevfs.NotFoundError
	return handle

class bluetooth_method:

	def __init__(self, method_name, args):
		print "__init__", method_name, args

	def vfs_open_directory(self, uri, open_mode, context):
		print "vfs_open_directory", uri, open_mode, context
		#raise gnomevfs.NotSupportedError

		handle = uri2handle(uri)

		return handle

	def vfs_read_directory(self, handle, file_info, context):
		print "vfs_read_directory", handle, context
		print 'dir_last_open=', os.getenv(r'BLUETOOTH_DIR_LAST_OPEN')
		handle.read_dir(file_info)

	def vfs_close_directory(self, handle, context):
		print "vfs_close_directory", handle, context
		#raise gnomevfs.NotSupportedError
		del (handle)

	def vfs_get_file_info(self, uri, file_info, options, context):
		print "vfs_get_file_info", uri, options, context

		print uri.short_name

		handle = uri2handle(uri)

		handle.get_file_info(file_info)

	def vfs_get_file_info_from_handle(self,	handle, file_info, options, context):
		print "vfs_get_file_info_from_handle", handle, options, context
		handle.get_file_info(file_info)

	def vfs_check_same_fs(self, uri_a, uri_b, context):
		raise gnomevfs.NotSupportedError

	def vfs_open(self, uri, mode, context):
		print "vfs_open", uri

		try:
			return uri2handle(uri)

		except Exception, e:
			traceback.print_exc()

	def vfs_close(self, handle, context):
		print "vfs_close", handle
		del (handle)

	def vfs_read(self, handle, buffer, num_bytes, context):
		print "vfs_read", handle

		return handle.read(buffer, num_bytes, context)

	def vfs_is_local(self, uri):
		#print "vfs_is_local", uri
		return False

	## Not implemented..
	def vfs_create(self, uri, mode, exclusive, perm, context):
		raise gnomevfs.NotSupportedError
	def vfs_write(self, handle, buffer, num_bytes, context):
		raise gnomevfs.NotSupportedError
	def vfs_seek(self, handle, whence, offset, context):
		raise gnomevfs.NotSupportedError
	def vfs_tell(self, handle):
		raise gnomevfs.NotSupportedError
	def vfs_truncate_handle(self, handle, length, context):
		raise gnomevfs.NotSupportedError
	def vfs_make_directory(self, uri, perm, context):
		raise gnomevfs.NotSupportedError
	def vfs_remove_directory(self, uri, context):
		raise gnomevfs.NotSupportedError
	def vfs_move(self, uri_old, uri_new, force_replace, context):
		raise gnomevfs.NotSupportedError
	def vfs_unlink(self, uri, context):
		raise gnomevfs.NotSupportedError
	def vfs_set_file_info(self, uri, file_info, mask, context):
		raise gnomevfs.NotSupportedError
	def vfs_truncate(self, uri, length, context):
		raise gnomevfs.NotSupportedError
#	def vfs_find_directory(self, uri, kind, create_if_needed, find_if_needed, perm, context):
#		raise gnomevfs.NotSupportedError
	def vfs_create_symbolic_link(self, uri, target_reference, context):
		raise gnomevfs.NotSupportedError
	def vfs_file_control(self, handle, operation, data, context):
		raise gnomevfs.NotSupportedError
