#
#	BNEP client module
#

panclient_services = [
	0x1115,		# PAN User
	0x1116,		# Network Access Protocol
	0x1117		# Network Gateway
]

PAND_CMD = 'pand'

class bluetool_panclient:
	'''Connects to a Personal Area Network via bluetooth'''

	def __init__(self):
		print "Loading bnep server"
		self.settings = {}

	def connect(self):
		print "Connecting to", self.settings['dst_address']
		pass

	def disconnnect(self):
		print "Disconnecting from", self.settings['dst_address']
		pass
