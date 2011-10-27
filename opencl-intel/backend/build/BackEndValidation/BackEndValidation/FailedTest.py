'''
Created on Nov 23, 2010

@author: myatsina
'''
class FailedTest(object):
    '''
    classdocs
    '''


    def __init__(self, command, expectFail, returncode, timedOut, matchedContainsCase, matchedExceptCase, matchedDoesntContainNoCase, matchedExceptMultiLineCase):
        '''
        Constructor
        Receives fail information
        
        @param command: The command line that was executed
        @type command: String
        
        @param expectFail: True denotest the test is expectewd to fail, false otherwise
        @type expectFail: Boolean
        
        @param returncode: The return code of the executed test
        @type returncode: Integer
        
        @param timedOut: True denotes test timed out, False otherwise
        @type timedOut: Boolean
        
        @param matchedContainsCase: Patterns that should have been found but weren't found, case sensative (i.e. 'Test succeeded')
        @type matchedContainsCase: FailedLine List
        
        @param matchedExceptCase: Expected failure that were found, case sensative (i.e. 'gauss test failed')
        @type matchedExceptCase: FailedLine List
        
        @param matchedDoesntContainNoCase: Patterns that shouldn't appear in the log file but were found, case insenstive (i.e. 'error', 'fail')
        @type matchedDoesntContainNoCase: FailedLine List
        
        @param matchedExceptMultiLineCase: Expected multi line failure that were found, case sensative (i.e. 'gauss test failed\n FAILURE!!!!\n')
        @type matchedExceptMultiLineCase: FailedLine List
        '''
        
        self.command = command
        self.expectFail = expectFail
        self. returncode = returncode
        self.timedOut = timedOut
        self.matchedContainsCase = matchedContainsCase
        self.matchedExceptCase = matchedExceptCase
        self.matchedDoesntContainNoCase = matchedDoesntContainNoCase
        self.matchedExceptMultiLineCase = matchedExceptMultiLineCase
        
    
    def __str__(self):
        myStr = 'Command: ' + self.command + '\n'
        if self.expectFail:
            myStr += 'Test was expected to fail\n'
        if self.returncode < 0:
            myStr += 'Bad return code: ' + str(self.returncode) + '\n'
        if self.timedOut:
            myStr += 'Test timed out' + '\n'
        if len(self.matchedContainsCase) > 0:
            myStr += 'Could not find (case sensative): \n' + self.stringFailedLines(self.matchedContainsCase)
        if len(self.matchedExceptCase) > 0:
            myStr += 'Found expected failures (case sensative): \n' + self.stringFailedLines(self.matchedExceptCase)
        if len(self.matchedDoesntContainNoCase) > 0:
            myStr += 'Found unexpected failures (case insensative): \n' + self.stringFailedLines(self.matchedDoesntContainNoCase)
        if len(self.matchedExceptMultiLineCase) > 0:
            myStr += 'Found expected multi line failures (case sensative) at line ranges: \n' + self.stringFailedLines(self.matchedExceptMultiLineCase)
                
        return myStr
    
    def stringFailedLines(self, failedLines):
        '''
        String
        
        @param failedLines: The fauiled lines
        @type failedLines: FailedLine List
        
        @return: String representation of the failed lines
        @rtype: String
        '''
        myStr = ''
        for failedLine in failedLines:
                        myStr = myStr + '\t' + str(failedLine) + '\n'
        return myStr
        
    
class FailedLine(object):
    
    def __init__(self, pattern, lineNum, line):
        '''
        Constructor
        Receives fail information
        
        @param pattern: 
        @type pattern: String
        
        ''' 
        self.failInfo = 'Pattern: ' + pattern + ', Line num: ' + str(lineNum) + ', Line: ' + line[0: len(line) - 1] #removed \n of the line
        
    def __str__(self):
        return self.failInfo

        
