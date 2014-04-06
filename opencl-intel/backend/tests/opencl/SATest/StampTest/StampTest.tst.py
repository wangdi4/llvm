import os, subprocess
import os.path, time
import sys

from optparse import OptionParser

parser = OptionParser()
parser.add_option("-c", dest="config",    default=None)
(options, args) = parser.parse_args()

pathname = os.path.abspath(os.path.dirname(sys.argv[0]))
configFile = os.path.join(pathname, options.config)

# number of files to be generated: config includes 2 kernels, each kernel has
# ref and neat, so 4 files
numNewFiles = 4
SATest = r'SATest'

# returns differense between two lists of files
def fileListDiff(a, b):
    return list(set(sorted(a)).symmetric_difference(set(sorted(b))))

# returns list of files from the folder
def getListFilesDir(a):
    return list( os.listdir( a ) )

# returns list of files with modification time for each file
def getFilesModTimes(a):
    res = []
    for file in a:
        fileFull = os.path.join(pathname, file)
        res.append([file,time.ctime(os.path.getmtime(fileFull))])
    res.sort()
    return res

#the list of files in the folder before test start
filesList = sorted(getListFilesDir(pathname))

# deletes files not included into filesList
def deleteNeedlessFiles():
    filesEnd = getListFilesDir(pathname)
    filesAdded = fileListDiff(filesEnd,filesList)
    for file in filesAdded:
        os.remove(os.path.join(pathname, file))


SAtestParameters = ' -VAL -neat=1 -tsize=1 -build-iterations=1 -execute-iterations=1';
single_wg_on = ' -single_wg=1';
single_wg_off = ' -single_wg=0';
force_ref_off = ' -force_ref=0';
force_ref_on = ' -force_ref=1';

######################################
# step 1
# new ref and neat files with stamps to be created, number of files created == numNewFiles

execstr = SATest + force_ref_off + SAtestParameters + single_wg_on +' -config=' + configFile
subprocess.call(execstr)

filesAfter = getListFilesDir(pathname)
newFiles = fileListDiff(filesAfter,filesList)

if len(newFiles) != numNewFiles:
    print 'wrong number of files generated, should be %d' % numNewFiles
    deleteNeedlessFiles()
    sys.exit(-1)

# take modification time for all new files
filesModTimes = getFilesModTimes(newFiles)

######################################
# step 2
# files created at the step 1 should be used as ref and neat

# to be sure that if new files generated they have different modification time
time.sleep(2)

execstr = SATest + force_ref_off + SAtestParameters + single_wg_on + ' -config=' + configFile
subprocess.call(execstr)

filesAfter2 = getListFilesDir(pathname)
newFiles2 = fileListDiff(filesAfter2,filesList)

# names of files should be the same
if newFiles != newFiles2:
    print 'error: lists of files should be the same, difference is:'
    print fileListDiff(newFiles,newFiles2)
    deleteNeedlessFiles()
    sys.exit(-1)

# different modification time means files have been regenerated
filesModTimes2 = getFilesModTimes(newFiles2)
if filesModTimes != filesModTimes2:
    print 'error: the modification time should be the same, difference is '
    print fileListDiff(filesModTimes,filesModTimes2)
    deleteNeedlessFiles()
    sys.exit(-1)

######################################
# step 3
# force_ref implemented - ref and neat files should be regenerated

# wait to be sure that file's modification time will be changed
time.sleep(2)

execstr = SATest + force_ref_on + SAtestParameters + single_wg_on + ' -config=' + configFile
subprocess.call(execstr)

filesAfter3 = getListFilesDir(pathname)
newFiles3 = fileListDiff(filesAfter3,filesList)

# names of files should be the same
if newFiles != newFiles3:
    print 'error: lists of files should be the same, difference is:'
    print fileListDiff(newFiles,newFiles3)
    deleteNeedlessFiles()
    sys.exit(-1)

# modification time should be different
filesModTimes3 = getFilesModTimes(newFiles3)
if filesModTimes == filesModTimes3:
    print 'error: the modification time should be different'
    deleteNeedlessFiles()
    sys.exit(-1)

######################################
# step 4
# change input data - ref and neat files should be regenerated
dataFile1 = os.path.join(pathname,r'StampTest.1.dat');
dataFile2 = os.path.join(pathname,r'StampTest.2.dat');
dataFile3 = os.path.join(pathname,r'serv.dat');

os.rename(dataFile1, dataFile3);
os.rename(dataFile2, dataFile1);
os.rename(dataFile3, dataFile2);

# to be sure that if new files generated they have different modification time
time.sleep(2)

execstr = SATest + force_ref_off + SAtestParameters + single_wg_on + ' -config=' + configFile
subprocess.call(execstr)

filesAfter4 = getListFilesDir(pathname)
newFiles4 = fileListDiff(filesAfter4,filesAfter3)

# the number of new files created should be equal to numNewFiles
if len(newFiles4) != numNewFiles:
    print 'wrong number of files generated, should be %d' % numNewFiles
    deleteNeedlessFiles()
    sys.exit(-1)

# names of files should be different, because stamps changed
if newFiles3 == newFiles4:
    print 'error: lists of files should be the different'
    deleteNeedlessFiles()
    sys.exit(-1)

######################################
# step 5
# change work group size - ref and neat files should be regenerated

# to be sure that if new files generated they have different modification time
time.sleep(2)

execstr = SATest + force_ref_off + SAtestParameters + single_wg_off + ' -config=' + configFile
subprocess.call(execstr)

filesAfter5 = getListFilesDir(pathname)
newFiles5 = fileListDiff(filesAfter5,filesAfter4)

# the number of new files created should be equal to numNewFiles
if len(newFiles5) != numNewFiles:
    print 'wrong number of files generated, should be %d' % numNewFiles
    deleteNeedlessFiles()
    sys.exit(-1)

# names of files should be different, because stamps changed
if newFiles4 == newFiles5:
    print 'error: lists of files should be the different'
    deleteNeedlessFiles()
    sys.exit(-1)

######################################
deleteNeedlessFiles()