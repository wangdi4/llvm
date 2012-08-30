import re
import shutil
import os
import sys
from optparse import OptionParser

def createEmptyDir(dir) :
	if not os.path.exists(dir):
		os.makedirs(dir)
	else :
		for root, dirs, files in os.walk(dir):
			for f in files:
				os.unlink(os.path.join(root, f))
			for d in dirs:
				shutil.rmtree(os.path.join(root, d))
				
def removeFile(f) :
	tmp = open(f, 'w')	
	tmp.close()
	os.remove(f)
	
def addLineToFile (path, str):
	f = open(path, 'a')	
	f.write(str)
	f.close();

class RunDesc :
	def __init__(self, cfg, suite, target, cpu, vsize):
		self.cfg = cfg
		self.suite = suite
		self.target = target
		self.cpu = cpu
		self.vsize = vsize
		
	def __init__(self, line, is_workload, sep) :
		list = line.split(sep)
		assert (len(list) >= 5)
		if is_workload : self.cfg = list[0].rpartition(".")[0]
		else : self.cfg = list[0]
		self.suite= list[1];
		self.target = list[2]
		self.cpu = list[3]
		self.vsize = list[4] 
		
	def asString (self, sep) :
		str = self.cfg + sep + self.suite + sep + self.target + sep + self.cpu + sep + self.vsize
		return str
		
	def isEqual(self, other) :
		return self.cfg == other.cfg and self.suite == other.suite and self.target == other.target and self.cpu == other.cpu and self.vsize == other.vsize

class WorkloadDesc :
	def __init__(self, run_desc, workload):
		self.run_desc = run_desc
		self.workload = workload
	
	def asString(self, sep=";") :
		str = self.run_desc.asString(sep) + sep + self.workload
		return str
	
	def isEqual(self, other) :
		return self.run_desc.isEqual(other.run_desc) and self.workload == other.workload
		
	def printVersion (self) :
		ret = self.run_desc.suite
		while (len(ret) < 12) : ret += " " 
		ret += self.run_desc.target
		while (len(ret) < 20) : ret += " " 
		ret += ("v"+self.run_desc.vsize)
		while (len(ret) < 24) : ret += " " 
		ret += self.run_desc.cpu
		while (len(ret) < 40) : ret += " " 
		ret += self.workload
		while (len(ret) < 100) : ret += " " 
		return ret
		
class CompareWorkloadDesc :
 	def __init__(self, workload_desc, time1, time2):
		self.workload_desc = workload_desc
		self.time1 = time1
		self.time2 = time2
		self.ratio = float(time1) / float(time2)
		
	def asString(self) :
		ret = self.workload_desc.printVersion()
		ret += self.time1
		while (len(ret) < 115) : ret += " " 
		ret += self.time2
		while (len(ret) < 130) : ret += " " 
		ret += str(self.ratio)
		return ret
		
		
		
class CompareWorkloadMeasures :
 	def __init__(self, workload_desc):
		self.workload_desc = workload_desc
		self.time1 = []
		self.time2 = []
		self.ratio = []
		
	def addMeasure(self, t1, t2) :
		self.time1.append(t1)
		self.time2.append(t2)
		self.ratio.append(float(t1) / float(t2))
		
	def printVersion(self) :
		ret = self.workload_desc.printVersion()
		mean_ratio = sum(self.ratio) / len(self.ratio)
		ret += str(mean_ratio)[0:6]
		ret += "       data:   "
		
		l = len(ret)
		for i in range(len(self.ratio)) : 
			r  = self.ratio[i]
			t1 = self.time1[i]
			t2 = self.time2[i]
			ret += str(t1)
			while (len(ret) < l + 12) : ret += " "
			ret += str(t2)
			while (len(ret) < l + 24) : ret += " "
			ret += str(r)[0:6]
			while (len(ret) < l + 40) : ret += " "
			l += 40
		
		return ret

class CompareWorkloadAccum:
	
	def __init__(self) :
		self.map = dict()
	
	def addComparison(self, workload_desc, time1, time2) :
		key = workload_desc.asString();
		if (key in self.map) :
			self.map[key].addMeasure(time1, time2)
		else :
			measure = CompareWorkloadMeasures(workload_desc)
			measure.addMeasure(time1, time2)
			self.map[key] =  measure
	
	def summarizeResults (self ,thr, log):
		above_thr = []
		stable = dict()
		noisy = dict()
		for key in self.map:
			ratios = []
			measures = self.map[key]
			has_speedup = 0
			has_slowdown = 0
			has_stable = 0
			for r in measures.ratio :
				if r > 1 + thr : has_speedup = 1
				elif r < 1 - thr : has_slowdown = 1
				else : has_stable = 1
				
			mean_ratio = sum(measures.ratio) / len(measures.ratio)
			if has_speedup + has_slowdown + has_stable > 1 and max(measures.ratio) - min(measures.ratio) > thr :
				noisy[mean_ratio] = measures
			else :
				stable[mean_ratio] = measures
				
		log.write ("\n\n\nsummary of noisy workloads\n---------------------------------\n")
		for ratio in sorted(noisy.keys()) :
			res = noisy[ratio]
			to_print_str = res.printVersion()
			if ratio < 1 + thr and ratio > 1 -thr :  to_print_str = "*" + to_print_str 
			else : to_print_str = " " + to_print_str 
			log.write (to_print_str + "\n")
			
		log.write ("\n\n\nsummary of stable workloads\n---------------------------------\n")
		for ratio in sorted(stable.keys()) :
			res = stable[ratio]
			to_print_str = res.printVersion()
			if ratio < 1 + thr and ratio > 1 -thr :  to_print_str = "*" + to_print_str 
			else : to_print_str = " " + to_print_str 
			log.write (to_print_str+"\n")
		
		
		
		
	
class PerfRun :

	def __init__(self, options):
		self.base_dir     = options.base_dir
		self.trunk_dir    = os.path.join(options.base_dir, options.srca);
		self.branch_dir   = os.path.join(options.base_dir, options.srcb);
		self.input_csv    = os.path.join(options.base_dir, options.csv);
		self.output_dir   = os.path.join(options.base_dir, options.output_dir);
		self.dumps_dir    = os.path.join(self.output_dir, "dumps");
		self.csv_dump     = os.path.join(self.output_dir, "changed.csv");
		self.log_fn       = os.path.join(self.output_dir, "output.log");
		self.tmp1         = os.path.join(self.output_dir, "tmp1.txt");
		self.tmp2         = os.path.join(self.output_dir, "tmp2.txt");
		self.thr          = float(options.thr)
		self.execute_iter = options.execute_iter
		self.num_iter     = int(options.num_iter)
		self.verbose      = int(options.verbose)
		self.has_nhlm     = int(options.has_nhlm)
		self.has_avx      = int(options.has_avx)
		self.has_hsw      = int(options.has_hsw)
		self.only_diff    = int(options.only_diff)
		self.getSuiteLoc(os.path.join(options.base_dir, options.suite))
		
	def getSuiteLoc (self, suite_path) :
		self.suite_loc = dict() 
		for line in open(suite_path):
			line = line.strip()
			if len(line) < 5  : continue
			list = line.split(";")
			assert(len(list) == 3)
			suite = list[0]
			target = list[1]
			loc = list[2]
			self.suite_loc[suite + ";" + target] = loc

	def getAsmPath (self, run_desc, src_dir) :
		src_name = os.path.split(src_dir)[1]
		return os.path.join(self.dumps_dir, run_desc.asString("_")+"_"+src_name+".asm")

	def get_ll_path (self, run_desc, src_dir) :
		src_name = os.path.split(src_dir)[1]
		return os.path.join(self.dumps_dir, run_desc.asString("_")+"_"+src_name+".ll")

	def executeCmd(self, cmd) :
		if (self.verbose) : print (cmd)
		ret = os.system(cmd)
		if (self.verbose) : print ("")
		return ret
		
	def parse(self):
		base_dict = dict()
		target_dict = dict()
	
		base_parsed = []
		for l in open(self.tmp1):
			l = l.strip()
			if (len(l) > 5) :
				if (l.startswith("description=")) :
					run_desc = RunDesc(l[len("description="):], False, ";")
				else :	
					list = l.split(",")
					base_parsed.append([WorkloadDesc(run_desc, list[0]), list[3]]) 
	
		target_parsed = []	
		for l in open(self.tmp2):
			l = l.strip()
			if (len(l) > 5) :
				if (l.startswith("description=")) :
					run_desc = RunDesc(l[len("description="):], False, ";")
				else :	
					list = l.split(",")
					target_parsed.append([WorkloadDesc(run_desc, list[0]), list[3]]) 
	
		assert(len(base_parsed) == len(target_parsed))
		compares = []
		for i in range(len(base_parsed)) :
			base_workload = base_parsed[i][0]
			target_workload = target_parsed[i][0]
			base_time = base_parsed[i][1]
			target_time = target_parsed[i][1]
			assert(base_workload.isEqual(target_workload))
			cur_compare = CompareWorkloadDesc(base_workload, base_time,  target_time)
			compares.append(cur_compare)
			self.all_compares.addComparison(base_workload, base_time, target_time)
		return compares
	
	def getAboveThr(self, results, thr):
		above_thr = []
		all = dict()
		for res in results:
			if res.ratio > 1 + thr or res.ratio < 1 -thr :
				above_thr.append(res.workload_desc.run_desc);
			all[res.ratio] = res
		
		for ratio in sorted(all.keys()) :
			res = all[ratio]
			to_print_str = res.asString()
			if ratio < 1 + thr and ratio > 1 -thr :
				to_print_str += "  within threshold"
			self.log.write(to_print_str + "\n")
		
		return above_thr
	
	def hasAsmDiff(self, run_desc):
		trunk_asm_path = self.getAsmPath(run_desc, self.trunk_dir)
		if not os.path.exists(trunk_asm_path) :
			return True
			
		branch_asm_path = self.getAsmPath(run_desc, self.branch_dir)
		if not os.path.exists(branch_asm_path) : 
			return True	
			
		trunk_asm_dump = open (trunk_asm_path)
		trunk_lines = trunk_asm_dump.readlines()
		
		branch_asm_dump = open (branch_asm_path)
		branch_lines = branch_asm_dump.readlines()
		
		if len(trunk_lines) != len(branch_lines) : return True
		for i in range(len(trunk_lines)) :
			if trunk_lines[i] != branch_lines[i] : return True
		
		return False
	
	def runSingle(self, run_desc, is_perf) :
		desc = run_desc.asString("   ")
		print ("running: " + desc )
		
		if run_desc.cpu == "corei7" and self.has_nhlm:
			cpu_param = "corei7"
		elif run_desc.cpu == "sandybridge" and self.has_avx :
			cpu_param = "corei7-avx"
		elif run_desc.cpu == "haswell" and self.has_hsw:
			cpu_param = "corei7-avx2"
		else :
			self.unsupported_job.append(run_desc)
			return False;
		
		if run_desc.target == "SLES64" :  run_desc.target = "Win64"
		
		suite_target = run_desc.suite + ";" + run_desc.target
		if suite_target in self.suite_loc :
			suite_base = self.suite_loc[suite_target]
		else :
			self.unsupported_job.append(run_desc)
			return False
		
		
		if is_perf : 
			mode_param = "-PERF "
			execute_iter_param = " -execute-iterations="+self.execute_iter
		else :
			mode_param = "-BUILD "
			execute_iter_param = ""
		params= mode_param + "-OCL -tsize=" +run_desc.vsize+ " -cpuarch=" + cpu_param + " -build-iterations=1" + execute_iter_param
		install_dir= os.path.join("install", run_desc.target , "Release", "bin")
		config = " -config=\"" + os.path.join(suite_base, run_desc.cfg) + ".cfg\""
		description_line = "description=" + run_desc.asString(";") + "\n"
				
		# run SATest on trunk
		addLineToFile(self.tmp1, description_line)
		os.chdir(os.path.join(self.trunk_dir, install_dir))
		ll_dump = " -dump-llvm-file=\""+ self.get_ll_path(run_desc, self.trunk_dir) +"\""
		asm_dump = " -dump-JIT=\""+ self.getAsmPath(run_desc, self.trunk_dir)  +"\""
		cmd = "SATest.exe "+params+ config + ll_dump + asm_dump +" >> " + self.tmp1
		res = self.executeCmd(cmd)
		if res != 0 : 
			self.failed_SATest.append(run_desc)
			return False
			
		# run SATest on branch
		addLineToFile(self.tmp2, description_line)
		os.chdir(os.path.join(self.branch_dir, install_dir))
		ll_dump = " -dump-llvm-file=\""+ self.get_ll_path(run_desc, self.branch_dir) +"\""
		asm_dump = " -dump-JIT=\""+ self.getAsmPath(run_desc, self.branch_dir)  +"\""
		cmd = "SATest.exe "+params+ config + ll_dump + asm_dump +" >> " + self.tmp2
		res = self.executeCmd(cmd)
		if res != 0: 
			self.failed_SATest.append(run_desc)
			return False
		
		return True
	
	def removeDuplicates (self, orig) :
		temp_dict = dict();
		for x in orig :
			temp_dict[x.asString(";")] = 0
		res = []
		for x in temp_dict.keys() :
			res.append(RunDesc(x, False, ";"));
		return res;
	
	def runBatch (self, to_run):
		if (len(to_run) == 0) :
			self.log.write("no workloads to run\n")
			return []
		
		# remove tmp files
		removeFile(self.tmp1)
		removeFile(self.tmp2)
		
		# aviod running twice the same cfg.
		unique_run = self.removeDuplicates(to_run)
		
		# run all jobs
		for job in unique_run : self.runSingle(job, True)

		# parse output
		results = self.parse() 
		
		# get jobs for all workloads above thr
		return self.getAboveThr(results, self.thr)
	
	def screenNoDiff (self, to_run):
		if (len(to_run) == 0) :
			self.log.write("no workloads to screen\n")
			return []
		
		# remove tmp files
		removeFile(self.tmp1)
		removeFile(self.tmp2)
		
		# aviod running twice the same cfg.
		uniqueRun = self.removeDuplicates(to_run)
		
		# run all jobs
		runs_with_asm_diff = []
		for job in uniqueRun :
			if self.runSingle(job, False) :
				if self.hasAsmDiff(job) :
					runs_with_asm_diff.append(job)
				else :
					self.no_diff.append(job)
		return runs_with_asm_diff;
	
	def parsePerfReportCSV (self):
		to_run = []
		for line in open(self.input_csv):
			line = line.strip()
			if line.find("SuiteName") != -1 : continue
			list = line.split(",")
			if (len(list) != 8) : continue;
			
			to_run.append(RunDesc(line, True, ","))
		return to_run
		
	def dumpCSV(self, to_run) :
		csv = open(self.csv_dump, 'w')	
		for res in to_run :
			csv.write(res.cfg + ".x,"  + res.suite + ","  + res.target + "," + res.cpu + "," + res.vsize + ",,,\n")
	
	def createEnvironment(self) :
		createEmptyDir(self.output_dir)
		os.makedirs(self.dumps_dir)
		shutil.copy2(self.input_csv, self.output_dir)
		
	def run(self) :
		self.log = open(self.log_fn , 'w')
		self.log.write("start running...\n")
		self.all_compares = CompareWorkloadAccum()
		self.no_diff = []
		self.failed_SATest = []
		self.unsupported_job = []
		to_run = self.parsePerfReportCSV()
		
		if self.only_diff :
			self.log.write("\n will screen kernels with no assembly diff...\n")
			to_run = self.screenNoDiff(to_run)
			
		
		for i in range(self.num_iter) :
			self.log.write("\n running iteration " + str(i) + "\n")
			to_run = self.runBatch(to_run)
		
		# write summary of results to log file
		self.log.write("\n\n\nUnsupported jobs\n-----------------------------------\n")
		for run_desc in self.unsupported_job : self.log.write(run_desc.asString("   ") +"\n")
		self.log.write("\n\n\nSATest failed\n-----------------------------------\n")
		for run_desc in self.failed_SATest : self.log.write(run_desc.asString("   ") +"\n")
		if (self.only_diff) :
			self.log.write("\n\n\nNo assembly diff\n-----------------------------------\n")
			for run_desc in self.no_diff       : self.log.write(run_desc.asString("   ") +"\n")
		self.all_compares.summarizeResults(self.thr, self.log)
		
		self.dumpCSV(to_run)
		self.log.close()
		removeFile(self.tmp1)
		removeFile(self.tmp2)
	
	
def main():
	#args parsing	
	parser = OptionParser()
	parser.add_option("-d", "--base_dir",     dest="base_dir",     help="base directory", default="c:\\work\\rchachic")
	parser.add_option("-a", "--srca",         dest="srca",         help="directory of base branch relative to base directory", default="trunk")
	parser.add_option("-b", "--srcb",         dest="srcb",         help="directory of target branch relative to base directory", default="LoopMask_34459")
	parser.add_option("-c", "--csv",          dest="csv",          help="files with csv of preformance report", default="workloads.txt")
	parser.add_option("-s", "--suite",        dest="suite",        help="suites cfg files destination", default="suite_loc.txt")
	
	parser.add_option("-n", "--num_iter",     dest="num_iter",     help="number of iterations for changed workloads", default=3)
	parser.add_option("-m", "--execute_iter", dest="execute_iter", help="number of iterations to run each workload", default="32")
	parser.add_option("-t", "--thr",          dest="thr",          help="threshold for performance chanfe" , default=0.04)
	
	parser.add_option("-o", "--output_dir",   dest="output_dir",   help="directory for dumping ir jit relative to base", default="perf_output")
	
	parser.add_option("-p", "--nhlm",         dest="has_nhlm",     help="the machine has nhlm support",    default=1)
	parser.add_option("-q", "--avx",          dest="has_avx",      help="the machine has avx support",     default=1)
	parser.add_option("-r", "--hsw",          dest="has_hsw",      help="the machine has haswell support", default=0)
	
	parser.add_option("-x", "--only_diff",    dest="only_diff",    help="screen values", default=1)
	
	parser.add_option("-v", "--verbose",      dest="verbose",      help="verbosity", default=0)
	
	(options, args) = parser.parse_args()

	
	Perf = PerfRun(options)
	Perf.createEnvironment()
	Perf.run()
	
if __name__ == "__main__":
	main_result = main()
	sys.exit(main_result)
	
	
