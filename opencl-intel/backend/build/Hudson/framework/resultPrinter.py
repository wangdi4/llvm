from core import TaskVisitor, VolcanoTestTask, VolcanoTestSuite, TestTaskResult
import logger



class ResultPrinter(TaskVisitor):
    """ Prints the test  run results for the given task hierarchy """
    PRINT_ALL = 1
    PRINT_NOT_PASSED = 2
    PRINT_FAILED = 3
    
    HEADER_MSG = {
                  PRINT_ALL: "Test suite results:\n",
                  PRINT_NOT_PASSED: "Failed and Ignored tests:\n",
                  PRINT_FAILED: "Failed tests:\n"
                 }
    
    def __init__(self):
        TaskVisitor.__init__(self)
        self.root_task = None
        self.task_count = 0
        self.task_failed = 0
        self.task_passed = 0
        self.task_timedout =0
        self.suite_count = 0
        self.suite_failed = 0
        self.suite_passed = 0
        self.suite_timedout = 0
        self.mode = self.PRINT_ALL

    def PrintTask(self, task):
        printResult = True
        
        if self.PRINT_NOT_PASSED == self.mode and TestTaskResult.Passed == task.result:
            printResult = False
            
        if self.PRINT_FAILED == self.mode and TestTaskResult.Failed != task.result:
            printResult = False 
        
        if printResult:
            self.root_task.logAndPrint("%(style)s[%(result)8s ]%(stylenorm)s %(testname)s" % { "style":TestTaskResult.resultStyle(task.result), "result":TestTaskResult.resultName(task.result), "stylenorm": logger.STYLE.NORMAL, "testname":task.fullName() }) 
                
    def OnVisitTask(self, task):
        self.task_count += 1
        
        if TestTaskResult.Failed == task.result:
            self.task_failed += 1
        
        if TestTaskResult.Passed == task.result:
            self.task_passed += 1
            
        if TestTaskResult.TimedOut == task.result:
            self.task_timedout += 1

        self.PrintTask(task)

    def OnVisitSuite(self, task):
        self.suite_count += 1
        
        if TestTaskResult.Failed == task.result:
            self.suite_failed += 1
        
        if TestTaskResult.Passed == task.result:
            self.suite_passed += 1
            
        if TestTaskResult.TimedOut == task.result:
            self.suite_timedout += 1

        self.PrintTask(task)

    def ClearStats(self):
        self.root_task = None
        self.task_count = 0
        self.suite_count = 0
        self.task_failed = 0
        self.task_passed = 0
        self.task_timedout =0
        self.suite_failed = 0
        self.suite_passed = 0
        self.suite_timedout = 0
        self.mode = self.PRINT_ALL

    def PrintResults(self, task, mode = PRINT_NOT_PASSED):
        self.root_task = task
        
        if mode in self.HEADER_MSG:
            self.mode = mode
        else: 
            self.mode = self.PRINT_ALL
            
        task.logAndPrint(logger.STYLE.INFO + "==============================================================================" + logger.STYLE.NORMAL)
        task.logAndPrint(self.HEADER_MSG[self.mode])
        task.visit(self)
        task.logAndPrint(logger.STYLE.HIGH + "\nSummary: Failed: %(failed)d test(s), Passed: %(passed)d test(s), Skipped/NotRun: %(skipped)d test(s), Total: %(total)d test(s)" % { "failed":self.task_failed + self.task_timedout, "passed":self.task_passed, "skipped": self.task_count - (self.task_passed + self.task_failed + self.task_timedout), "total":self.task_count })
        task.logAndPrint(logger.STYLE.HIGH + "         Failed: %(failed)d suite(s), Passed: %(passed)d suite(s), Skipped/NotRun: %(skipped)d suite(s), Total: %(total)d suite(s)" % { "failed":self.suite_failed + self.suite_timedout, "passed":self.suite_passed, "skipped": self.suite_count - (self.suite_passed + self.suite_failed + self.suite_timedout), "total":self.suite_count })
        
        