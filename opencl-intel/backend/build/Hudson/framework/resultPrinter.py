from core import TaskVisitor, VolcanoTestTask, VolcanoTestSuite, TestTaskResult

class ResultPrinter(TaskVisitor):
    """ Prints the test  run results for the given task hierarchy """
    def __init__(self):
        TaskVisitor.__init__(self)
        self.root_task = None
        
    def OnVisitTask(self, task):
        if( self.root_task is None ):
            self.root_task = task
            self.root_task.logAndPrint("===================")
            self.root_task.logAndPrint("Test suite results:\n")
        
        self.root_task.logAndPrint("[%(result)8s ] %(testname)s" % { "result":TestTaskResult.resultName(task.result), "testname":task.fullName() }) 

    def OnVisitSuite(self, task):
        self.OnVisitTask(task)
