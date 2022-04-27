##
# Don't call this script directly. Use the CMAke-generated wrapper:
# valRun.py
# See it's content at CMakeLists.txt
#
#
# Example invocation:
# python ./valRun.py -x -p /nfs/iil/home/shohaml/build/trunk/install/SLES64/Debug/bin -p /nfs/iil/disks/opencl_vlan/shohaml/trunk/src -t framework_TBB
##
import os
import subprocess
import sys
import difflib
import optparse
import re
import shutil
from multiprocessing import Array

pwd    = "."
srcDir = "."
batchMode = False

BYTES_DIFF_THREASHHOLD = 100

##########################################################
# Container for a test.
#
# command - string array of the actual command to invoke. Each argument, including
#           the name of the command is an array element. It will run from workdir.
# suppression - list of suppression files,
# workdir - path to run the test from.
# len     - test run length:
#           * short (default - up to 5 minutes)
#           * medium (up to 10 minutes)
#           * long
##########################################################
class TestParams:
    def __init__(self, comm, supp, wrkdir=".", len="short"):
        self.command = comm      # command as a string array, each string is argv.
        self.suppression = supp  # array of strings, suppression file names.
        self.workdir = wrkdir    # work directory to launch the test.
        self.testLen=len         # Length of test runtime, for general knowledge. 

##########################################################
# FrameworkTestType wrapper over TestParams.
# It hides most of the standard params, such as test command,
# and suppression file - which are uniform to all framework tests.
#
# tname  - framework test name. Will be padded with '*', You can use partial names,
# supp   - [optional] list of suppression files,
# runLen - test run length:
#           * short (default - up to 5 minuts)
#           * medium (up to 10 minutes)
#           * long
##########################################################
class FrameworkTestParams(TestParams):
    def __init__(self, tname, supp=["runtime.supp"], runLen="short"):
        self.frameworkTestName = tname
        rtest = ["./framework_test_type", "--gtest_filter=*%s*" % (self.frameworkTestName)]
        TestParams.__init__(self, rtest, supp, wrkdir="../framework_test_type", len=runLen)

##########################################################
# Conformance12 wrapper over TestParams.
# It hides most of the standard params, such as test command,
# and suppression file - which are uniform to all framework tests.
#
# command  - string array of the actual command to invoke. Each argument, including
#            the name of the command is an array element. It will run from workdir.
# supp     - [optional] list of suppression files,
# workdir  - [optional] path to run the test from.
# runLen   - test run length:
#           * short (default - up to 5 minuts)
#           * medium (up to 10 minutes)
#           * long
##########################################################
class Conf12TestParams(TestParams):
    def __init__(self, command, supp=["runtime.supp", "conform12.supp"], 
                 workdir="../conform12", runLen="short"):
        TestParams.__init__(self, command, supp, workdir, len=runLen)


# dict from test name, to execution line
listOfTests = {
    "framework_TBB": FrameworkTestParams ("Test_TBB", runLen="short"),
    "framework_clCreateContextTest": FrameworkTestParams ("clCreateContext", runLen="short"),
    "framework_clExecutionTest": FrameworkTestParams ("clCreateContext", runLen="short"),
    "framework_clBuildProgramMaxArgsTest": FrameworkTestParams ("clBuildProgramMaxArgs", runLen="short"),
    "framework_clCreateKernelTest": FrameworkTestParams ("clCreateKernel", runLen="short"),
    "framework_memset": FrameworkTestParams ("memset", runLen="medium"),
    "conf12_basic_readimage": Conf12TestParams(["./basic/test_basic", "readimage"], runLen="short"),
    "conf12_basic_loop": Conf12TestParams(["./basic/test_basic", "loop"], runLen="medium"),
    "conf12_basic_barrier": Conf12TestParams(["./basic/test_basic", "barrier"], runLen="medium"),
}

#===============================================================================
# Candidates:
# Test_clCreateContextTest
# Test_clCreateKernelTest
# Test_clExecutionTest
# Test_clBuildProgramMaxArgsTest
# Test_EventDependenciesTest - BAD test.
#
# Test_MT_execution
# Test_ImmediateExecutionTest
#
# Test_CreateReleaseOOOQueueTest
# Test_ShutdownFromChildThread
#
# Test_clOutOfOrderTest
# Test_clEnqueueRWBuffer
#===============================================================================

# global junk_PID
junk_PPID = re.compile(r'(Parent PID:).*')
junk_heap_usage = re.compile(r'^.*total heap usage.*$')
junk_PID  = re.compile(r'(--|==)\d+(--|==) ')
#junk_path = re.compile(r'^(.*/)?([\w/.]+)')
junk_path = re.compile(r'([\w./]+)?/([\w.]+)')
misleading_byte_count = re.compile(r'in loss record (\d+) of (\d+)')

start_of_leak_summary = re.compile(r'LEAK SUMMARY:')
definitely_lost_summary = re.compile(r'definitely lost: ([\d,]+) bytes in ([\d,]+) blocks')
indirectly_lost_summary = re.compile(r'indirectly lost: ([\d,]+) bytes in ([\d,]+) blocks')
possibly_lost_summary = re.compile(r'possibly lost: ([\d,]+) bytes in ([\d,]+) blocks')
suppressed_summary = re.compile(r'suppressed: ([\d,]+) bytes in ([\d,]+) blocks')
used_suppression   = re.compile(r'used_suppression: *([\d]+) +(.+)')

remove_fullpath_cmd = re.compile(r'--fullpath-after=')

# Debug condition:
debugIsOn = False
def isDebug():
    global debugIsOn
    return debugIsOn

##
# return True if the line is to be ignored.
##
def valIgnoreLines(line):
    global junk_PID
    if None != junk_PID.search(line):
        if isDebug():
            print "Found Junk %s</br>" % (line)
        return True

    return False


def clearValgrindLog(logfile):
    global junk_PPID
    global junk_heap_usage
    global junk_PID
    global junk_path
    global misleading_byte_count

    global remove_fullpath_cmd

    retLines = []

    for l in open(logfile).readlines():
        if None != remove_fullpath_cmd.search(l):
            continue

        nl = junk_PPID.sub(r'\1***', l)
        nl = junk_heap_usage.sub(r'total heap usage: ...', nl)
        nl = junk_PID.sub(r'', nl);
        nl = junk_path.sub(r'.../\2', nl)
        nl = misleading_byte_count.sub(r'in loss record XXX of YYY', nl)
        retLines.append(nl)

    return retLines


def printValErrorReport(logfile, rgxp, xin, header, newl):
    retVal = header
    leaks = {}
    newlIndx = 0
    for res in rgxp.finditer(xin):
        while newl[newlIndx] < res.start():
            newlIndx += 1
        retVal += '''
\n
Found in Line %d
-------------------
%s
''' % (newlIndx+1, res.groups(1)[0])
    
    return retVal


def printValErrorReportSorted(logfile, rgxp, xin, header, newl):
    retVal = ""
    leaks = {}
    newlIndx = 0
    for res in rgxp.finditer(xin):
        while newl[newlIndx] < res.start():
            newlIndx += 1
        
        k = int(res.groups(1)[1])
        if k not in leaks.keys():
            leaks[k] = {}
        #leaks[k][newlIndx+1] = '\n'.join(res.groups(2)[0])
        leaks[k][newlIndx + 1] = res.groups(2)[0]
    
    retVal += header
    for sz in sorted(leaks.keys(), reverse=True):
        for line, txt in leaks[sz].iteritems():
            retVal += '''
Found in Line %d (%d byts)
-----------------------------
%s
''' % (line, sz, txt)
    
    return retVal


def getInterestingErrors(logfile):
    xin = open(logfile).read(-1)
    newl = newLineArr(xin)
    retVal = ""

    header = """\n\n
Memory leaks in descending order
=================================
"""
    regxLost = re.compile("""
(
\s+(\d+) \sbytes \sin \s\d+ \sblocks \sare \s[^}]+ \slost.*
\s+.*at,*\s+.*\s+         # at... line
(?:.*by.*\?{3}\s+)*
(?:.*by.*[^?\s]+\s+[^?\s]+\s+)+
(?:.*by.*\?{3}\s+)*
)
""", re.VERBOSE
)
    retVal += printValErrorReportSorted(logfile, regxLost, xin, header, newl)

    header = """\n\n
Invalid read/writes in descending order
========================================
"""
    regxInvalidRead = re.compile("""
(
\s+Invalid \s(?:read|write) \sof \ssize \s(\d+).*
\s+.*at,*\s+.*\s+         # at... line
(?:.*by.*\?{3}\s+)*
(?:.*by.*[^?\s]+\s+[^?\s]+\s+)+
(?:.*by.*\?{3}\s+)*
)
""", re.VERBOSE
)
    retVal += printValErrorReportSorted(logfile, regxInvalidRead, xin, header, newl)

    header = """\n\n
Jumps on unconditional memory
==============================
    """
    regxJump = re.compile("""
(
\s+Conditional \sjump \sor \smove.*
\s+.*at,*\s+.*\s+         # at... line
(?:.*by.*\?{3}\s+)*
(?:.*by.*[^?\s]+\s+[^?\s]+\s+)+
(?:.*by.*\?{3}\s+)*
)
""", re.VERBOSE
)
    retVal += printValErrorReport(logfile, regxJump, xin, header, newl)

    header = """\n\n
Syscall on uninitialized memory
================================
    """
    regxSyscall = re.compile("""
(
\s+Syscall \sparam.*
\s+.*at,*\s+.*\s+         # at... line
(?:.*by.*\?{3}\s+)*
(?:.*by.*[^?\s]+\s+[^?\s]+\s+)+
(?:.*by.*\?{3}\s+)*
)
""", re.VERBOSE
)
    retVal += printValErrorReport(logfile, regxSyscall, xin, header, newl)

    header = """\n\n
Invalid/Mismatched free
=========================
    """
    regxFree = re.compile("""
(
\s+(?:Invalid|Mismatched) \sfree.*
\s+.*at,*\s+.*\s+         # at... line
(?:.*by.*\?{3}\s+)*
(?:.*by.*[^?\s]+\s+[^?\s]+\s+)+
(?:.*by.*\?{3}\s+)*
)
""", re.VERBOSE
)
    retVal += printValErrorReport(logfile, regxFree, xin, header, newl)    

    header = """\n\n
Overlapping source and destination
===================================
    """
    regxOverlap = re.compile("""
(
\s+Source \sand \sdestination \soverlap.*
\s+.*at,*\s+.*\s+         # at... line
(?:.*by.*\?{3}\s+)*
(?:.*by.*[^?\s]+\s+[^?\s]+\s+)+
(?:.*by.*\?{3}\s+)*
)
""", re.VERBOSE
)
    retVal += printValErrorReport(logfile, regxOverlap, xin, header, newl)    
    
    return retVal



#######################################
# Generate an array of newline char indexes.
#######################################
def newLineArr(openFile):
    regx = re.compile(r'\n')
    retVal = []
    for res in regx.finditer(openFile):
        retVal.append(res.start())
    return retVal


def compare(expectedFile, actualFile, genFormat):
    if genFormat is "html":
        diff = difflib.HtmlDiff(tabsize=4, wrapcolumn=60, linejunk=valIgnoreLines). \
        make_file(clearValgrindLog(expectedFile), clearValgrindLog(actualFile), expectedFile, actualFile, True)
    elif genFormat is "html_full":
        diff = difflib.HtmlDiff(tabsize=4, wrapcolumn=60, linejunk=valIgnoreLines). \
        make_file(clearValgrindLog(expectedFile), clearValgrindLog(actualFile), expectedFile, actualFile, False)
    elif genFormat is "unified":
        diff = difflib.unified_diff(clearValgrindLog(expectedFile), clearValgrindLog(actualFile), expectedFile, actualFile)
    else:
        sys.stderr("Illegal comparison output format!")
        sys.exit(1)

    return diff



def isDiffMeaningful(expectedFile, actualFile):
    expected = getLogSummary(expectedFile)
    actual = getLogSummary(actualFile)
    meaningful = False
    global BYTES_DIFF_THREASHHOLD

    # in case of more than 100 bytes difference of lost/suppressed bytes, raise a flag.
    for k, i, j in zip(["definitely", "indirectly", "probably", "suppressed"],
                       expected["lostBytes"], actual["lostBytes"]):
        if isDebug():
            print "%s expected: %d    actual: %d" % (k, i, j)
        delta = abs(i - j)
        if delta > BYTES_DIFF_THREASHHOLD:
            print "There is a big difference (%d bytes) in %s lost bytes: %d vs. expected %d\n<br/>\n" % \
            (delta, k, j, i)
            meaningful = True

    # Any difference in types, and count of activated suppressions (+-2). 
    for sup, times in actual["usedSupressions"].iteritems():
        if sup not in expected["usedSupressions"]:
            print "Got a new suppression: %s<br/>\n" % (sup)
            meaningful = True
        else:
            deltaTimes = times - expected["usedSupressions"][sup]
            if deltaTimes > 2 or times != expected["usedSupressions"][sup] < -2:
                print "Suppression %s appears a different number of times, %d instead of %d<br/>\n" % \
                (sup, times, expected["usedSupressions"][sup])
                meaningful = True

    return meaningful



def readLostValues(rx, line):
    x = rx.search(line)
    if x is not None:
        lostBytes = int(x.group(1).replace(',', ''))
        lostBlocks = int(x.group(2).replace(',', ''))
        return True, lostBytes, lostBlocks
    return False, 0, 0

def readUsedSupression(line):
    global used_suppression
    x =  used_suppression.search(line)
    if x is not None:
        howMany = int(x.group(1).replace(',', ''))
        supName = x.group(2)
        return True, supName, howMany
    return False, "", 0 

def getLogSummary(f):
    inSummary = False
    iFound = False
    dFound = False
    pFound = False
    sFound = False

    dLostBytes = 0
    dLostBlocks = 0
    iLostBytes = 0
    iLostBlocks = 0
    pLostBytes = 0
    pLostBlocks = 0
    sLostBytes = 0
    sLostBlocks = 0
    
    supr = {}

    global start_of_leak_summary
    global definitely_lost_summary
    global indirectly_lost_summary
    global possibly_lost_summary
    global suppressed_summary

    fin = open(f,'rb')
    blockRevNum = 0  # first inc will be 1.
    
    while not inSummary:
        blockRevNum += 1
        lines = readFileInReverseBlockOffset(fin, blockRevNum)
        if lines is None:
            # we did not find summary data in the file
            break
        for l in lines:
            if not inSummary:
                if start_of_leak_summary.search(l) is not None:
                    if isDebug():
                        print "getLogSummary Found summary line: %s" % (l)
                    inSummary = True
                else:
                    if isDebug():
                        print "getLogSummary no summary in: %s" % (l)
            else: # in summary
                if isDebug():
                    print "getLogSummary() summary line: %s" % (l)
                     
                if not dFound:
                    dFound, dLostBytes, dLostBlocks = readLostValues(definitely_lost_summary, l)
                    if dFound:
                        continue
                    
                if not iFound:
                    iFound, iLostBytes, iLostBlocks = readLostValues(indirectly_lost_summary, l)
                    if iFound:
                        continue
                    
                if not pFound:
                    pFound, pLostBytes, pLostBlocks = readLostValues(possibly_lost_summary, l)
                    if pFound:
                        continue
                    
                if not sFound:
                    sFound, sLostBytes, sLostBlocks = readLostValues(suppressed_summary, l)
                    if sFound:
                        continue
                    
                foundSupression, supName, howMany = readUsedSupression(l)
                if foundSupression:
                    supr[supName] = howMany

    fin.close()
    return {
        # must keep order: ["definitely", "indirectly", "probably", "suppressed"]
        "lostBytes": (dLostBytes, iLostBytes, pLostBytes, sLostBytes),
        "usedSupressions": supr
    }

#===============================================================================
# Read the file in a backwards offset of blocks.
# Return list of lines, or None if file size exceeded.
#===============================================================================
def readFileInReverseBlockOffset(f, offset=1, blockSize=4096):
    # File must be opened binary
    if 'b' not in f.mode.lower():
        raise Exception("Cannot read file %s - it has to be opened is binary." % (f.name))
    
    leftover = 0
    sz = os.stat(f.name).st_size
    numBlocks, leftover = divmod(sz, blockSize)
    
    bytesOffset = 0
    # align to file start, not end. The end is a partial block.
    if 0 == sz:
         return None
    elif leftover != 0:
         bytesOffset = leftover
         if offset > 1:       
             bytesOffset += (offset - 1) * blockSize
    else:
         bytesOffset = offset * blockSize

    if isDebug():
        print "readFileInReverseBlockOffset %s  File size: %d   #blocks: %d + %d Bytes  offset %d (%d)" % \
            (f.name, sz, numBlocks, leftover, offset, bytesOffset )
    
    if bytesOffset > sz:
        if isDebug():
            print "readFileInReverseBlockOffset trying to read more than file size!"
        return None

    if isDebug():
        print "readFileInReverseBlockOffset going to return %d bytes splitted to lines." % (bytesOffset)
    
    f.seek(-bytesOffset, 2) # from EOF
    return f.read(bytesOffset).splitlines()


def main():
    global listOfTests
    global srcDir
    
    exitStatus = 0

    usage = '''
    %prog : Run valgrind memory check.
    
    Do not run the script testAndDiff.py directly. Use the valRun.py wrapper.
    More info in the README.TXT file.
    
    # Run 'testName' with optional output, and flow.
    %prog <testName> [-t] [-x] [-d] [-f format] [-p install_path -p src_root ...]
    
    # list existing tests:
    %prog -l
    
    # clear an existing log file - prepare for manual diff.
    %prog -c <log_file> > clean_log_name.txt
    
    # extract interesting leaks/failures from log
    %prog -j <log_file>
    
    # Update 'testName' expected results, use the actual results file.
    %prog <testName> -u

    Each test has a <test>.expected file (regression), and running it
    generates a temporary results file <test>.actual. The two files are
    compared in an intelligent way - to see if there are meaningful changes:
    1) 100 bytes or more difference in number of lost/suppressed bytes.
    2) Any difference in types, and count of activated suppressions (+-2).
    '''
    parser = optparse.OptionParser(usage)
    utilsGroup  = optparse.OptionGroup(parser, "Utils",
       "Use utils to help analyze and update log, and suppression files.")
    flowGroup  = optparse.OptionGroup(parser, "Flow",
       "Control the flow of execution, and Valgrind.")
    
    utilsGroup.add_option("-c", "--clear", action="store", type="string",
                          help='Clear Valgrind log file from run-specific info and print it. Good for external diff tools.')
    parser.add_option("-d", "--debug", action="store_true", default=False,
                      help='Debug script.')
    parser.add_option("-f", "--diff_format", type="choice", action="store", dest="dformat",
          choices=['text', 'html', 'full_html', 'none'], default='text',
          help='Format for the diff report between actual run, and expected. [text(default), html, full_html, none].')
    utilsGroup.add_option("-j", "--leaks_report", action="store", type="string",
                          help='Print meaningful leaks found in a log file.')
    parser.add_option("-l", "--list", action="store_true", default=False,
                      help='List available tests.')
    flowGroup.add_option("-n", "--no_suppression", action="store_true", default=False,
                         help='Do not use suppressions.')
    flowGroup.add_option("-p", "--path_prefix", action="append",
                         help='Path prefix to ignore in logs, can be called multiple times.')
    flowGroup.add_option("-s", "--show_diff", action="store_true", default=False,
                         help='Show diff even if there isn\'t a noticeable memory leak change.')
    utilsGroup.add_option("-u", "--update_expected", action="store_true", default=False,
                          help='Update test expected (regression) results from current "actual" file.')
    flowGroup.add_option("-x", "--no_execute", action="store_true", default=False,
                         help='Do not execute Valgrind, assume current log file already exists.')

    parser.add_option_group(utilsGroup)
    parser.add_option_group(flowGroup)

    (options, args) = parser.parse_args()

    if options.list:
        listAllTests()
        sys.exit(0)

    # Debug flag, must be first check.
    if options.debug:
        global debugIsOn
        debugIsOn = True
        
    if options.clear is not None:
        fname = options.clear
        if not os.path.isfile(fname):
            print "no file '%s' - trying '%s.actual'" % (fname, fname)
            fname += ".actual"
        if os.path.isfile(fname):
            sys.stdout.writelines(clearValgrindLog(fname))
        else:
            print "Can't find file to clear log '%s'" % (fname)
        sys.exit(0)

    if options.leaks_report is not None:
        fname = options.leaks_report
        if not os.path.isfile(fname):
            print "no file '%s' - trying '%s.actual'" % (fname, fname)
            fname += ".actual"
        if os.path.isfile(fname):
            sys.stdout.write(getInterestingErrors(fname))
        else:
            print "Can't find file to extract leaks from '%s'" % (fname)
        sys.exit(0)

    if len(args) == 0:
        print "You must supply a test name, or one of the non-testing flags. See help '-h'."
        sys.exit(1)

    testName = args[0]
    if testName not in listOfTests:
        print "Test %s does not exist.\n\n" %   (testName)
        listAllTests()
        sys.exit(1)

    expectedFile="%s/%s.expected" % (srcDir, testName)

    actualFile="%s.actual" % (testName)

    # update expected (regression)
    if options.update_expected:
        if batchMode:
            print "Sorry, but you cannot update tests in batch (Jenkins) mode."
            sys.exit(0)
        print "Replacing old expected results %s with new one from PWD: %s" % \
            (expectedFile, actualFile)
        shutil.copy(actualFile, expectedFile)
        sys.exit(0)

    valCmd = [
               "valgrind",
               "--leak-check=full",
               "--smc-check=all",
               "--read-var-info=yes",
               "--show-reachable=yes",
               "--track-origins=yes",
               "--log-file=%s/%s" % (pwd, actualFile),
               "--gen-suppressions=all",
               "--num-callers=40",
               "--leak-resolution=high",
               "--partial-loads-ok=yes",
               "--free-fill=99",
               "--malloc-fill=FF",
               "-v",
               ]
    
    if not options.no_suppression:
        for supp in listOfTests[testName].suppression:
            valCmd.append("--suppressions=%s/%s" % (srcDir, supp))
    
    if None != options.path_prefix:
        for p in options.path_prefix:
            valCmd.append("--fullpath-after=%s " % (p))
    
    valCmd.append("--")   

    if options.no_execute:
        print "Not running the test, using existing log files for test '%s'\n" % (testName)
        print "Potential command: %s\n" % ( ' '.join(valCmd + listOfTests[testName].command))
    else:
        os.chdir(listOfTests[testName].workdir)
        #
        print "Going to execute: %s\n" % ( ' '.join(valCmd + listOfTests[testName].command))
        # os.system(valCmd)
        subprocess.call(valCmd + listOfTests[testName].command)
        os.chdir(pwd)

    # check if actual file exists
    if not os.path.isfile(actualFile):
        exitStatus = 1
        print '''
** NOTE:
There is no current (actual) results file for test '%s'.
Please run the test at least once.
''' % (testName)
        exit(exitStatus)

    # Check if expected file exists
    if not os.path.isfile(expectedFile):
        exitStatus = 1
        print '''
** NOTE:
There is no regression (expected) results file for test '%s'. If this is a new test,
than after you verify the actual valgrind log file, you should run:
> %s -u %s
> svn add %s
Otherwise, this is an error.

''' % (testName, sys.argv[0], testName, expectedFile)
        exit(exitStatus)

    print '''
<br/>    
Memory check status report<br/>
===========================<br/>
'''
    if isDiffMeaningful(expectedFile, actualFile):
        exitStatus = 1
    else:
        print "Check is OK, compatible with expected results."

    print '''
End of memory check status report<br/>
==================================<br/>
<br/>
'''

    # find out if we need to show a diff, at all.
    if not options.show_diff and exitStatus == 0: # show diff anyway
        print "\nCheck was OK. No meaningful difference.\n"
        sys.exit(exitStatus)

    if (options.show_diff) or (exitStatus != 0 and options.dformat is not 'none'):
        diff = ""
        if options.dformat is 'html':
            diff = compare(expectedFile, actualFile, "html")
        elif options.dformat is 'full_html':
            diff = compare(expectedFile, actualFile, "html_full")
        elif options.dformat is 'text':
            diff = compare(expectedFile, actualFile, "unified")
        sys.stdout.writelines(diff)
        
    sys.exit(exitStatus)


def listAllTests():
    global listOfTests

    for k in sorted(listOfTests.keys()):
        print "%s (%s)" % (k, listOfTests[k].testLen)


if __name__ == '__main__':
    main()


