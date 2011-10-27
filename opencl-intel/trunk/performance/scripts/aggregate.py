import sys, os, csv

class PerfFile(object):
	
	def __init__(self, filename=None):
		self.tests = []
		if not filename: return
		try:
			content = open(filename, 'rb')
			self.reader = csv.reader(content, dialect='excel')
		except:
			return
		rownum = 0
		for row in self.reader:
			if rownum > 0:
				self.tests.append(row[0:5])
			rownum+=1
		
	def join(self, another):
		if len(another.tests) == 0:
			return
		if len(self.tests) == 0:
			self.tests = another.tests
			return
			
		assert(len(self.tests) == len(another.tests))
		for i in xrange(len(self.tests)):
			myTest = self.tests[i]
			otherTest = another.tests[i]
			assert(myTest[0] == otherTest[0])
			self.tests[i] = [myTest[0]]+ myTest[1:]+ otherTest[1:]
	
	def dump_csv(self, title, offset):
		sb = title + ",\n"
		for i in xrange(len(self.tests)):
			myTest = self.tests[i]
			numRuns = len(self.tests[i])-1
			assert(numRuns%4 == 0)
			sb += str(myTest[0]) + ","
			for tp in xrange(4):
				sb += "\"=min("
				exp = []
				for r in xrange(numRuns/4):
					ch = chr(ord('F') + r*4 + tp)
					exp.append(str(ch) + str(i+2+offset))
				sb += ",".join(exp) + ")\","
			sb += ",".join(myTest[1:])
			sb +=",\n"
		print sb
		return len(self.tests) + 2
			
	def __str__(self):
		return str(self.tests)

if __name__ == "__main__":	
	test_families = sys.argv[1:]
	offset = 0
	for test_family in test_families:
		Perf = PerfFile()
		for suffix in ['_0','_1','_2','_3','_4']:
			F = PerfFile(test_family + suffix)
			Perf.join(F)
		offset += Perf.dump_csv(test_family, offset)