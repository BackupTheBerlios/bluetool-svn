import sys
import re
import time
import gnomevfs
import dbus
import bluetool
import os

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
Name=Bluetooth neighborood on %s
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

HWADDR_RE = re.compile('^([0-9a-fA-F][0-9a-fA-F]:){5}([0-9a-fA-F][0-9a-fA-F])$')

DIR_LAST_OPEN = None

def get_root():
	return RootDirectoryHandle()

class RootDirectoryHandle(object):

	def __init__(self):

		print 'RootDirectoryHandle()'

		self.mgr = bluetool.get_manager(sys_bus)

		self.devs = self.mgr.ListDevices()
		self.index = 0

	def read_dir(self, file_info):

		try:
			dev_path = self.devs[self.index]
			DeviceFileHandle(dev_path).get_file_info(file_info)

			self.index += 1

		except IndexError:
			self.index = 0
			raise gnomevfs.EOFError

	def get_file_info(self, file_info):

		file_info.name = '/'
		file_info.mime_type = 'x-directory/normal'
		file_info.type = gnomevfs.FILE_TYPE_DIRECTORY
		file_info.permissions = 0555
		file_info.mtime = long(time.time())
		file_info.ctime = file_info.mtime
		file_info.atime = file_info.mtime

	def open_device_file(self, uri):

		if not uri.short_name.endswith('.desktop'):
			return

		address = uri.short_name[:-8]

		#print 'open_device_file: address =',address, 'self =', self

		print 'devs=',self.devs

		for dev_path in self.devs:
			hci = bluetool.get_local_hci(sys_bus, dev_path)
			if hci.GetProperty('address').replace(':','') == address:
				return DeviceFileHandle(dev_path)

		raise gnomevfs.NotFoundError

	def open_device_dir(self, uri):

		# uri was already validated

		for dev_path in self.devs:
			hci = bluetool.get_local_hci(sys_bus, dev_path)
			if hci.GetProperty('address') == uri.short_name:
				return DeviceDirectoryHandle(uri, dev_path)

		raise gnomevfs.NotFoundError

class DeviceFileHandle(object):

	def __init__(self, dev_path):

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
		self.bread = 0

	def get_file_info(self, file_info):

		dskname = self.address.replace(':','')

		file_info.name = dskname + '.desktop'
		file_info.mime_type = 'application/x-gnome-app-info'
		file_info.type = gnomevfs.FILE_TYPE_REGULAR
		file_info.permissions = 0555
		file_info.size = len(self.contents)
		#file_info.uid = os.getuid()
		#file_info.gid = os.getgid()
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

class DeviceDirectoryHandle(object):

	def __init__(self, uri, dev_path):
		print 'DeviceDirectoryHandle(',dev_path,')'

		self.hci = bluetool.get_local_hci(sys_bus, dev_path)
		
		self.name = self.hci.GetProperty('name')
		self.address = self.hci.GetProperty('address')

		print 'dir_last_open=',DIR_LAST_OPEN

		if uri == DIR_LAST_OPEN:
			print 'reloading URI', uri 
			self.devs = self.hci.StartInquiry()
		else:
			self.devs = self.hci.InquiryCache()
		self.index = 0

	def get_file_info(self, file_info):

		file_info.name = self.address
		file_info.mime_type = 'x-directory/normal'
		file_info.type = gnomevfs.FILE_TYPE_DIRECTORY
		file_info.permissions = 0555
		file_info.mtime = long(time.time())
		file_info.ctime = file_info.mtime
		file_info.atime = file_info.mtime

	def read_dir(self, file_info):

		try:
			rem_path = self.devs[self.index]
			RemDevFileHandle(rem_path, self).get_file_info(file_info)

			self.index += 1

		except IndexError:
			self.index = 0
			raise gnomevfs.EOFError

	def open_remdev_file(self, uri):

		print 'open_remdev_file(', uri, ')'

		if not uri.short_name.endswith('.desktop'):
			return

		address = uri.short_name[:-8]

		for rem_path in self.devs:
			hci = bluetool.get_remote_hci(sys_bus, rem_path)
			if hci.GetProperty('address').replace(':','') == address:
				return RemDevFileHandle(rem_path, self)

		raise gnomevfs.NotFoundError

	def open_remdev_dir(self, uri):
		pass

class RemDevFileHandle:

	def __init__(self, rem_path, dir_handle):

		print 'RemDevFileHandle(',rem_path,')'

		self.hci = bluetool.get_remote_hci(sys_bus, rem_path)
		self.sdp = bluetool.get_remote_sdp(sys_bus, rem_path)
		
		self.name = self.hci.GetProperty('name')
		self.address = self.hci.GetProperty('address')
		if len(self.name) == 0:
			self.name = self.address

		svc, ma, mi = self.hci.GetProperty('class')

		self.contents = REMDEV_FILE_PROTOTYPE % (
					self.name,
					'file:///usr/share/pixmaps/bt-logo.png',
					dir_handle.address,
					self.address
				)
		self.bread = 0

	def get_file_info(self, file_info):

		dskname = self.address.replace(':','')

		file_info.name = dskname + '.desktop'
		file_info.mime_type = 'application/x-gnome-app-info'
		file_info.type = gnomevfs.FILE_TYPE_REGULAR
		file_info.permissions = 0555
		file_info.size = len(self.contents)
		#file_info.uid = os.getuid()
		#file_info.gid = os.getgid()
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

class InquiryDirectoryHandle:
	def __init__(self, uri):
		pass

class ServiceDirectoryHandle:
	def __init__(self, uri):
		pass

def uri2handle(uri):

	print 'uri2handle:',uri

	if uri.short_name == '/':
		return get_root()

	parent = uri.parent

	if uri.short_name.endswith('.desktop'):

		if HWADDR_RE.match(parent.path[1:]):
			return get_root().open_device_dir(parent).open_remdev_file(uri)
		else:
			return get_root().open_device_file(uri)

	else:
		if HWADDR_RE.match(parent.path[1:]):
			return get_root().open_device_dir(parent).open_remdev_dir(uri)
		else:
			return get_root().open_device_dir(uri)

	raise gnomevfs.NotFoundError

class bluetooth_method:

	def __init__(self, method_name, args):
		print "__init__", method_name, args

	def vfs_open_directory(self, uri, open_mode, context):
		print "vfs_open_directory", uri, open_mode, context
		#raise gnomevfs.NotSupportedError

		handle = uri2handle(uri)

		DIR_LAST_OPEN = uri

		return handle

	def vfs_read_directory(self, handle, file_info, context):
		print "vfs_read_directory", handle, context
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
