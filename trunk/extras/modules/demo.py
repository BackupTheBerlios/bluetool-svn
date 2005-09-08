#
#	Bluetool Demo Service
#


class bluetool_demo:
	'''Proof of concept plugin'''

	def __init__(self):
		print "Loading demo service"
		self.settings = {}

	def Start(self):
		print "Starting service"

	def Stop(self):
		print "Stopping service"
