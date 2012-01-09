import unittest


class DebuggerTestCase(unittest.TestCase):
    """ All debugger test cases should inerit from this class.
        They will then have a 'self.client' attribute with the 
        ClientSimulator object.
    """
    def __init__(self, methodName='runTest', client=None, dtt_exe_logfile=None):
        """ dtt_exe_logfile is a file object into which to write the return
            code and stderr of the debuggee invocation.
        """
        super(DebuggerTestCase, self).__init__(methodName)
        self.client = client
        self.dtt_exe_logfile = dtt_exe_logfile

    def setUp(self):
        """ Each test should have a fresh client object ready for connection.
        """
        self.client.reset()

    @staticmethod
    def create(testcase_klass, **kwargs):
        """ Create a suite containing all tests taken from the given
            subclass, passing them the extra parameters in kwargs.
        """
        testloader = unittest.TestLoader()
        testnames = testloader.getTestCaseNames(testcase_klass)        
        suite = unittest.TestSuite()
        for name in testnames:
            suite.addTest(testcase_klass(name, **kwargs))
        return suite

    def tearDown(self):
        """ dtt_exe_logfile is a file object into which to write the return
            code and stderr of the debuggee invocation.
        """
        self.client.terminate_debuggee()
        rc, stderr = self.client.wait_for_debuggee_exit()
        
        if self.dtt_exe_logfile is not None:
            self.dtt_exe_logfile.write('\n\nrc = %s -->' % rc)
            self.dtt_exe_logfile.write(_fix_stream_newlines(stderr))


def _fix_stream_newlines(s):
    return '\n'.join(s.splitlines())


#--------------------------------------------------------------------------
if __name__ == "__main__":    
    class TestOne(DebuggerTestCase):
        def test_something(self):
            print 'client =', self.client
            self.assertEqual(1, 1)
        
        def test_something_else(self):
            self.assertEqual(2, 2)

    suite = unittest.TestSuite()
    suite.addTest(DebuggerTestCase.create(TestOne, client=42))
    suite.addTest(DebuggerTestCase.create(TestOne, client=42234))
    unittest.TextTestRunner(verbosity=2).run(suite)


