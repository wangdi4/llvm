from optparse import OptionParser
import sys
import os
import subprocess


def runTestForAllConfigurations(test, confFolder, outFolder, tempFolder, postScript):
    for fileName in os.listdir(confFolder):
        if False == fileName.endswith(".cfg"):
            continue
        os.putenv("MIC_DEVICE_CFG_FILE", os.path.join(confFolder, fileName))
        print "Going to run " + test + " with " + fileName + " configuration file."
        subprocess.Popen(test, shell=True).wait()
        processNameToOutTraceDict = dict()
        for traceFile in os.listdir(tempFolder):
            index = traceFile.find("_tracer_")
            if index == -1:
                print "Error: Exiting"
                sys.exit(1)
            processName = traceFile[0:index]
            if False == (processName in processNameToOutTraceDict):
                processNameToOutTraceDict[processName] = list()
            processNameToOutTraceDict[processName].append(traceFile)
        for k, l in processNameToOutTraceDict.iteritems():
            if len(l) <> 2:
                print "Error: Exiting"
                sys.exit(1)
            postProcessingCommand = "python " + postScript + " -i" + os.path.join(tempFolder, l[0]) + "," + os.path.join(tempFolder, l[1]) + " -o" + os.path.join(outFolder, k + "_" + fileName[0: fileName.find(".cfg")])
            print "Going to run the following post processing: " + postProcessingCommand
            subprocess.Popen(postProcessingCommand, shell=True).wait()
        for traceFile in os.listdir(tempFolder):
            os.remove(os.path.join(tempFolder, traceFile))
            
        

def main():

    parser = OptionParser(usage = 'uasge: %prog [options]', description = "Run specify test with some configuratoins and create post processing trace report.")
    parser.add_option("-t", "--test", dest = "test_cmd", help = "The command line of the test", default = None)
    parser.add_option("-c", "--configurations", dest = "conf_folder", help = "Path to the configuration(s) file(s) folder", default = None)
    parser.add_option("-o", "--output", dest = "out_folder", help = "Path to output folder", default = '.')
    parser.add_option("-p", "--postScript", dest = "post_script", help = "Path to tracer_post_processor.py script", default = "tracer_post_processor.py")

    (options, args) = parser.parse_args()

    test = options.test_cmd
    if (None == test):
        print "Error: please enter test name."
        sys.exit(1)

    confFolder = options.conf_folder
    if (None == confFolder):
        print "Error: please enter configuration(s) directory."
        sys.exit(1)

    if (False == os.path.isdir(confFolder)):
        print "Error: " + confFolder + " does not exist"
        sys.exit(1)

    outFolder = options.out_folder

    if (False == os.path.isdir(outFolder)):
        print "Error: " + outFolder + " does not exist"
        sys.exit(1)

    postScript = options.post_script

    if (False == os.path.isfile(postScript)):
        print "Error: " + postScript + " does not exist"
        sys.exit(1)

    print "AdirD"
    tempFolder = os.path.join(outFolder, "temp")
    if False == os.path.isdir(tempFolder):
        os.mkdir(tempFolder)

    currentTraceOutFolderEnv = os.getenv("TRACE_OUTPUT_FOLDER")
    currentMicDeviceCfgFileEnv = os.getenv("MIC_DEVICE_CFG_FILE")

    os.putenv("TRACE_OUTPUT_FOLDER", tempFolder)

    runTestForAllConfigurations(test, confFolder, outFolder, tempFolder, postScript)

    os.rmdir(tempFolder)

    if None == currentTraceOutFolderEnv:
        os.unsetenv("TRACE_OUTPUT_FOLDER")
    else:
        os.putenv("TRACE_OUTPUT_FOLDER", currentTraceOutFolderEnv)

    if None == currentMicDeviceCfgFileEnv:
        os.unsetenv("MIC_DEVICE_CFG_FILE")
    else:
        os.putenv("MIC_DEVICE_CFG_FILE", currentMicDeviceCfgFileEnv)

    print "ByeBye"
        

main()
