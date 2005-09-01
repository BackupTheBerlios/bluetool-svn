#!/usr/bin/env python
import re

#conffile = 'hcid.conf'

class BluezConfig:

	CFG_COMMENT    = re.compile('\s*#')
	CFG_STARTSECT  = re.compile('\s*([a-z]+)\s+'
				    '([0-9A-Fa-f]{1,2}:[0-9A-Fa-f]{1,2}:[0-9A-Fa-f]{1,2}:'
				    '[0-9A-Fa-f]{1,2}:[0-9A-Fa-f]{1,2}:[0-9A-Fa-f]{1,2})?'
				    '\s*{\s*#*')
	CFG_ENDSECT    = re.compile('\s*}$')
	CFG_KEY        = re.compile('\s*([a-z|_]+)\s*')
	CFG_VALUE      = re.compile('\s*([^,|^;]+)\s*([,|;])')

	CFG_SECTIONS   = set(['options','device'])
	CFG_OPTPARAMS  = set(['autoinit','pairing','pin_helper','security'])
	CFG_DEVPARAMS  = set(['name','auth','encrypt','class','pscan','iscan','lm','lp','pkt_type'])

	def __init__(self, filepath):
		
		self.filepath = filepath
		self.load()
		self.store()

	def load(self):

		self.options = {}
		self.devices = {}

		fhandle = open(self.filepath, "r")
		curr_sect = ''

		while 1:
			line = fhandle.readline()
			if len(line) == 0:
				break
			
			m = BluezConfig.CFG_COMMENT.match(line) 
			if m:
			#	print "COMMENT>", line
				continue

			m = BluezConfig.CFG_STARTSECT.match(line)
			if m:
				if len(curr_sect) != 0:
					print "error: sections cannot be nested"
					raise TypeError

				curr_sect = m.groups()[0]
			#	print "NEWSECT>",curr_sect
				curr_spec = m.groups()[1]

				if curr_sect == 'device':
					if curr_spec is None:
						curr_spec = ''
					self.devices[curr_spec] = {}
				elif curr_sect == 'options':
					self.options = {}
					if curr_spec is not None:
						raise TypeError
				continue
			else:	pass #print line

			m = BluezConfig.CFG_ENDSECT.match(line)
			if m:
			#	print "ENDSECT>",curr_sect
				curr_sect = ''
				continue

			m = BluezConfig.CFG_KEY.match(line)
			if m:
				key = m.groups()[0]
				curr_entry = None
				if curr_sect == 'options':
					curr_entry = self.options[key] = []
				elif curr_sect == 'device':
					curr_entry = self.devices[curr_spec][key] = []

			#	print "KEY>", key
				end = BluezConfig.CFG_KEY.match(line).span()[1]
				line = line[end:]
			#	print "now parsing",line
				while 1:
					mv = BluezConfig.CFG_VALUE.match(line)
					if mv is None:
						print "error: unterminated parameter"
						raise TypeError
					value = mv.groups()[0]
					tail = mv.groups()[1]

					curr_entry.append(value)

			#		print "VALUE>", value
					if tail == ';': break

					end = BluezConfig.CFG_KEY.match(line).span()[1]+1
					line = line[end:]
			#		print "now parsing",line

		print self.options
		print self.devices

	def store(self):
		fhandle = open(self.filepath, 'w+')
		#fhandle = open('hcid.out', 'w+')

		def dump_dict(dic):
			for key in dic.keys():
			
				fhandle.write('\t'+key+' ')

				for idx in range(0, len(dic[key])):

					vals = dic[key][idx:]
			#		print vals, idx
					fhandle.write(vals[0])
					if len(vals) == 1:
						fhandle.write(';\n')
						break
					else:	fhandle.write(',')

		#
		#	first write the options
		#
		fhandle.write('options {\n')

		dump_dict(self.options)

		fhandle.write('}\n\n')

		#
		#	then any device section found
		#
		for key in self.devices.keys():

			fhandle.write('device '+key+'{\n')

			dump_dict(self.devices[key])

			fhandle.write('}\n\n')


		fhandle.close()


if __name__ == '__main__':
	cfg = BluezConfig(conffile)
