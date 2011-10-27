'''
Created on Nov 8, 2010

@author: myatsina
'''

from pydoc import deque
import os
import re
from BackEndValidation.FailedTest import FailedLine, FailedTest
import time
import subprocess
from BackEndValidation.IOManager import IOManager
import platform

class Validator(object):
    '''
    Validator
    
    Responsible for prepareing the environment and running the release criteria tests. 
    '''

    def __init__(self, validatorOptionsParser):
        '''
        Constructor
        
        Creates new Validator.
        
        @param validatorOptionsParser: Validator options parser which contains run options
        @type validatorOptionsParser: ValidatorOptionsParser 
        '''
        
        self.runOnly = validatorOptionsParser.getRunOnly()
        self.validateOnly = validatorOptionsParser.getValidateOnly()
        self.noImport = validatorOptionsParser.getNoImport()
        self.logFile = validatorOptionsParser.getLogFile()
        self.outputDir = validatorOptionsParser.getOutputDir()
        self.importBinSrcDir = validatorOptionsParser.getImportBinSrcDir()
        self.importBinDestDir = validatorOptionsParser.getImportBinDestDir()
        self.workingDir = validatorOptionsParser.getWorkingDir()
        self.oclCpuBackEnd = validatorOptionsParser.getOclCpuBackEnd()
        self.builinFunctions = validatorOptionsParser.getBuilinFunctions()
        self.svml = validatorOptionsParser.getSVML()
        self.tests = validatorOptionsParser.getTests()
        
        self.ioManager = IOManager() 

    def run(self):
        
        '''
        Runs tests.
        
        @return: Failed tests
        @rtype: FailedTest List
        '''
                
        if not self.validateOnly:
            # Delete old OclCpuBackEnd log file
            self.ioManager.delete(self.logFile)
        
            # Delete old test logs
            self.ioManager.delete(self.outputDir)        
            self.ioManager.makedirs(self.outputDir)
        
            if not self.runOnly:
        
                if not self.noImport:
                    # Delete old OpenCL SDK binaries and import new ones
                    self.ioManager.delete(self.importBinDestDir)
                    # Create new directory that will contians OpenCL SDK binaries
                    self.ioManager.makedirs(self.importBinDestDir)
                    self.importBin()
        
                # Copy OclCpuBackEnd DLL
                self.copyPathContent(self.oclCpuBackEnd)
                # Copy built-in functions DLLs and RTLs
                self.copyPathContent(self.builinFunctions)
                # Copy SVML DLLs
                self.copyPathContent(self.svml)
        
            # from now on working in importBinDestDir/relativeWorkingDir        
            os.chdir(self.workingDir)
         
        return self.runTests()

    def importBin(self):    
        '''
        Import OpenCL SDK and WOLF binaries.
        '''
        self.ioManager.delete(self.importBinDestDir)
        self.ioManager.copy(self.importBinSrcDir, self.importBinDestDir)
            
        
    def copyPathContent(self, path):
        '''
        Copies specified path content into the working directory.
        
        @param path: Path which content needs to be copied into the working dirctory.
                        If path is a file then the file will be copied.
                        If path is a directory then all files directly under it will be copied
        @type path: String 
        '''  
        if os.path.isdir(path):
            for file in os.listdir(path):
                fileName = os.path.join(path, file)
                if os.path.isfile(fileName):
                    self.ioManager.copy(fileName, self.workingDir, copyTree = False)
        else:
            self.ioManager.copy(path, self.workingDir, copyTree = False) 

    def executeTest(self, test):
        '''
        Executes test command and saves test output into the appropriate test log file
        
        @param test: Test object that contains all information concerning the test
        @type test: Test
        
        @return: The return code of the executed test
        @rtype: Integer
        
        @return: timedOut: True denotes test timed out, False otherwise
        @rtype: timedOut: Boolean
        '''

        print test.getCommand()
        
        
        # Open test log file
        logFile = self.ioManager.openFile(test.getLogName(), 'w')
        
        
        SLEEP_INTERVAL = 1
        
        timeoutSeconds = test.getTimeout() * 60
        secondsPassed = 0
        
        timedOut = False

        procStratTime = time.time()
        
        isShell=True
        if platform.system() == 'Windows':
            isShell=False
        proc = subprocess.Popen(test.getCommand(), stdout=logFile, stderr=logFile, shell=isShell)
        
        while proc.returncode is None:
            proc.poll()
            secondsPassed = time.time() - procStratTime
            if secondsPassed > timeoutSeconds :
                timedOut = True
                proc.terminate()
                # Wait for process to terminate
                proc.wait()
                break
            time.sleep(SLEEP_INTERVAL)
            
        logFile.close()
        
        return proc.returncode, timedOut
        
        


    def validateOutputLog(self, failedTests, test, returncode, timedOut):
        '''
        Validates test output.
        
        @param failedTests: A list of the currently know failed tests, if the current test also failed it will be added to this list
        @type failedTests: FailedTest List
        
        @param test: The test to validate
        @type test: Test
        
        @param returncode: The return code of the executed test
        @type returncode: Integer
        
        @param timedOut: True denotes test timed out, False otherwise
        @type timedOut: Boolean
        
        '''
        
        # Contains patterns that are expected to be found in the test log file, but were not found yet, case sensative
        matchedContainsCase = test.getContainsCase()
        
        # Contains expected failure patterns that were found, case sensative
        matchedExceptCase = []
        
        # Contains patterns that shouldn't appear in the log file but were found, case insenstive
        matchedDoesntContainNoCase = []
        
        # Contains expected multi line failure that were found, case sensative
        matchedExceptMultiLineCase = self.getMatchedMultiLineRanges(test.getLogName(), test.getExceptMultiLineCase(), True)
        
        logFile = self.ioManager.openFile(test.getLogName(), 'rb')
        
        lineNum = 0
        
        for line in logFile:
            # Remove patterns that were found in the log file and are expected to be found
            matchedContainsCase = [x for x in matchedContainsCase if x not in self.getMatchPatterns(line, test.getContainsCase(), True)]         
            
            # Check if current line matches any of the expexted failures
            exceptions = self.matchPatterns(line, lineNum, test.getExceptCase(), True)
            matchedExceptCase.extend(exceptions)
            
            # Check if line is not expected one line failure or expected multi line failure
            if len(exceptions) == 0 and not self.isMultiLineException(lineNum, matchedExceptMultiLineCase):
                # Check if line matches failure pattern
                matchedDoesntContainNoCase.extend(self.matchPatterns(line, lineNum, test.getDoesntContainNoCase(), False))
            lineNum += 1
        
        logFile.close()
        
        failedTest = returncode < 0 or timedOut or len(matchedContainsCase) > 0 or len(matchedDoesntContainNoCase) > 0
        
        # Check if expected pattern wasn't found or some lines matched failures
        if (failedTest and not test.getExpectFail()) or (not failedTest and test.getExpectFail()):
                # Add test to failed tests     
                failedTests.append(FailedTest(test.getCommand(), test.getExpectFail(), returncode, timedOut, matchedContainsCase, matchedExceptCase, matchedDoesntContainNoCase, matchedExceptMultiLineCase))

    def runTests(self):
        '''
        Runs tests
        
        @return: Failed tests
        @rtype: FailedTest List
        '''
        failedTests = []
        for test in self.tests:  
            returncode = 0
            timedOut = False   
            if not self.validateOnly:
                # Execute test
                returncode, timedOut = self.executeTest(test)
            
            # Validate test log file 
            self.validateOutputLog(failedTests, test, returncode, timedOut)
        
        return failedTests
    
        
    def matchPattern(self, line, pattern, caseSensative):
        '''
        Checks if line matches pattern with respect tp case sensativy
        
        @param line: Log line to check agains pattern 
        @type line: String
        
        @param pattern: Pattern
        @type pattern: String
        
        @param caseSensative: True denotes case sensative
        @type caseSensative: Boolean 
        
        @return: True in case line matches pattern, Flase otherwise
        @rtype: Boolean
        '''
        lineToMatch = line
        patternToMatch = pattern
        
        if not caseSensative:
            # Transfer line and pattern to lower case
            lineToMatch = lineToMatch.lower()
            patternToMatch = patternToMatch.lower()
        
        if re.match(patternToMatch, lineToMatch) is not None:
            return True
        return False
    
    
    def matchPatterns(self, line, lineNum, patterns, caseSensative):
        '''
        Checks if line matches the specified patterns with respect tp case sensativy
        
        @param line: Log line to check agains pattern 
        @type line: String
        
        @param patterns: Pattern
        @type patterns: String List
        
        @param caseSensative: True denotes case sensative
        @type caseSensative: Boolean
        
        @return: List of all failure information based on all patterns the specified line matched
        @rtype: FailedLine List
        '''
        matches = []
        
        for pattern in patterns:
            
            if self.matchPattern(line, pattern, caseSensative):
                matches.append(FailedLine(pattern, lineNum, line))
        
        return matches
    
    def getMatchPatterns(self, line, patterns, caseSensative):
        '''
        Checks if line matches one of the specified patterns with respect tp case sensativy
        
        @param line: Log line to check agains pattern 
        @type line: String
        
        @param patterns: Pattern
        @type patterns: String List
        
        @param caseSensative: True denotes case sensative
        @type caseSensative: Boolean
        
        @param failedLine: True denotes create FailedLine type
        @type failedLine: Boolean  
        
        @return: List of all patterns the specified line matched
        @rtype: String List
        '''
        matches = []
        
        for pattern in patterns:
            
            if self.matchPattern(line, pattern, caseSensative):
                matches.append(pattern)
        
        return matches
    
    
    def matchMultiLine(self, linesToMatch, patternMultiLine, caseSensative):
        '''
        Checks whether the specified line matches a multi line pattern
        
        @param linesToMatch: Directory path which content needs to be copies to the working dirctory.
        @type linesToMatch: String Lisat
        
        @param patternMultiLine: The multi line pattern against which the lines need to be matches.
        @type patternMultiLine: String List
        
        @param caseSensative: True denotes case sensative
        @type caseSensative: Boolean
        
        @return: True if the specified lines match the multi line pattern 
        @rtype: Boolean
        '''
        if len(linesToMatch) != len(patternMultiLine):
            return False
        
        for i in range(0, len(patternMultiLine)):
            if not self.matchPattern(linesToMatch[i], patternMultiLine[i], caseSensative):
                return False
        
        return True
        
    
    def getMatchedMultiLineRanges(self, logFile, patternMultiLine, caseSensative):
        '''
        Returns ranges of lines in the test log file that match the specified multi line pattern
        
        @param logFile: Name of test log file
        @type logFile: String
        
        @param patternMultiLine: The multi line pattern against which the lines need to be matches.
        @type patternMultiLine: String List
        
        @param caseSensative: True denotes case sensative
        @type caseSensative: Boolean
        
        @return: Ranges of lines in the test log file that match the specified multi line pattern
        @rtype: List of line ranges
        '''
        matches = []
        linesToMatch = deque()
        
        if len(patternMultiLine) == 0:
            return matches
        
        lineNum = 0
        
        logFile = open(logFile, 'r')
        for line in logFile:
            
            if len(linesToMatch) == len(patternMultiLine):
                linesToMatch.popleft()
            linesToMatch.append(line)
            
            # Check if linesToMatch que matches multi line pattern
            if (len(linesToMatch) == len(patternMultiLine)) and self.matchMultiLine(linesToMatch, patternMultiLine, caseSensative):
                # Add line range to matches
                matches.append([lineNum - len(linesToMatch) + 1, lineNum])
            
            lineNum += 1
        
        logFile.close()
        return matches
    
    def isMultiLineException(self, lineNum, matchedExceptMultiLineCase):
        '''
        Checks if the specified line number represnts a line in the log that matches a multi line pattern
        
        @return: True if the specified line number represnts a line in the log that matches a multi line pattern
        @rtype: Boolean 
        '''
        for lineRange in matchedExceptMultiLineCase:
            if  lineRange[0] <= lineNum and lineNum <= lineRange[1]:
                return True
        
        return False
            
