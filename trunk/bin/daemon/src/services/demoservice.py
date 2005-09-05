#
#	Bluetool Demo Service
#


class bluetool_demoservice:
	'''This class demonstrates how to extend bluetool
		with your custom python scripts'''

	def __init__(self):
		print "Loading demo service"
		self.settings = {}

	def Start(self):
		print "Starting service"
		pass

	def Stop(self):
		print "Stopping service"
		pass
