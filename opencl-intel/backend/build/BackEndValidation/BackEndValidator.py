'''
Created on Nov 23, 2010

@author: myatsina

'''
from BackEndValidation.ValidatorError import ValidatorError
import traceback

def printResult(status):
    print ''
    print '============================================='
    print '               TEST ' + status
    print '============================================='
    print ''

from BackEndValidation import ValidatorOptionsParser
from BackEndValidation.Validator import Validator
import sys


if __name__ == '__main__':
    
    if sys.version_info < (2, 6):     
        print 'Error: you must use python 2.6 or greateer'
        sys.exit(-1) 

    try:
        validatorOptionsParser = ValidatorOptionsParser.ValidatorOptionsParser()
        validatorOptionsParser.parse()
                
        validator = Validator(validatorOptionsParser)
        
        failedTests = validator.run()
        
        if len(failedTests) > 0:
            printResult('FAILED')
            for failedTest in failedTests:
                print failedTest
                print ''
            sys.exit(-1)
        else:
            printResult('SUCCEEDED')
            sys.exit(0)
    
    except ValidatorError as error:
        print error
        print ''
        if error.getOrigException() is not None:
            print 'Original exception:'
            traceback.print_exc()
        
        sys.exit(-1)
        
    except Exception as error:
        print 'Unexpected error occured:'
        traceback.print_exc()
        
