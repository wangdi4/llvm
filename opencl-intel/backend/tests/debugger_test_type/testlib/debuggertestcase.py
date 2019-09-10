import unittest
import functools
import sys

from common import loge, logw, GDBContinueError


class DebuggerTestCase(unittest.TestCase):
    """ All debugger test cases should inerit from this class.
        They will then have a 'self.client' attribute with the
        ClientSimulator object.
    """
    def __init__(self, methodName='runTest', use_gdb=True, use_cdb=False, client=None, dtt_exe_logfile=None):
        """ native is a flag to specify running of GDB tests when True.
            dtt_exe_logfile is a file object into which to write the return
            code and stderr of the debuggee invocation.
        """
        super(DebuggerTestCase, self).__init__(methodName)
        self.use_gdb = use_gdb
        self.use_cdb = use_cdb
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

def expectedFailureGDB(func):
    """ This decorator is a modified version of unittest.case.expectedFailure.
        Placing it on a testcase will mark the test as "expected to fail" when
        native (GDB) tests are being run (i.e. DebuggerTestCase.native == True)
        NOTE: this decorator requires python 2.7.
        FIXME: this decorator should call the original unittest.case.expectedFailure
    """
    @functools.wraps(func)
    def wrapper(self, *args, **kwargs):
        if sys.version_info < (2, 7):
            logw("Warning: use Python >= 2.7 (current version is " \
                + ".".join(map(str, sys.version_info)) + ")" \
                + " for 'expected fail' support in testcase " + str(func))
            func(self, *args, **kwargs)
            return
        else:
            if self.use_gdb:
                try:
                    func(self, *args, **kwargs)
                except Exception:
                    raise unittest.case._ExpectedFailure(sys.exc_info())
                raise unittest.case._UnexpectedSuccess
            else:
                func(self, *args, **kwargs)
    return wrapper

def expectedFailureCDB(func):
    """ This decorator is a modified version of unittest.case.expectedFailure.
        Placing it on a testcase will mark the test as "expected to fail" when
        native (GDB) tests are being run (i.e. DebuggerTestCase.native == True)
        NOTE: this decorator requires python 2.7.
        FIXME: this decorator should call the original unittest.case.expectedFailure
    """
    @functools.wraps(func)
    def wrapper(self, *args, **kwargs):
        if sys.version_info < (2, 7):
            logw("Warning: use Python >= 2.7 (current version is " \
                + ".".join(map(str, sys.version_info)) + ")" \
                + " for 'expected fail' support in testcase " + str(func))
            func(self, *args, **kwargs)
            return
        else:
            if self.use_cdb:
                try:
                    func(self, *args, **kwargs)
                except Exception:
                    raise unittest.case._ExpectedFailure(sys.exc_info())
                raise unittest.case._UnexpectedSuccess
            else:
                func(self, *args, **kwargs)
    return wrapper

def skipNotGDB(func):
    @functools.wraps(func)
    def wrapper(self, *args, **kwargs):
        if sys.version_info < (2, 7):
            logw("Warning: use Python >= 2.7 (current version is " \
                + ".".join(map(str, sys.version_info)) + ")" \
                + " for 'expected fail' support in testcase " + str(func))
            func(self, *args, **kwargs)
            return
        else:
            if self.use_gdb:
                func(self, *args, **kwargs)
            else:
                print("Skipping test " + str(func))
                return unittest.skip("Not supported in simulator testing.")
    return wrapper


def _fix_stream_newlines(s):
    return '\n'.join(s.splitlines())


#--------------------------------------------------------------------------
if __name__ == "__main__":
    class TestOne(DebuggerTestCase):
        def test_something(self):
            print('client =', self.client)
            self.assertEqual(1, 1)

        def test_something_else(self):
            self.assertEqual(2, 2)

    suite = unittest.TestSuite()
    suite.addTest(DebuggerTestCase.create(TestOne, client=42))
    suite.addTest(DebuggerTestCase.create(TestOne, client=42234))
    unittest.TextTestRunner(verbosity=2).run(suite)
