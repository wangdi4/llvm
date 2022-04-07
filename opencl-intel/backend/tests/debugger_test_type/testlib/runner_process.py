
import os
import inspect
import imp
import sys
import traceback
import unittest
try:
    from StringIO import StringIO # Python 2
except ImportError:
    from io import StringIO # Python 3

from multiprocessing import JoinableQueue, Queue, Process
from Queue import Empty
from testlib.common import TestSuiteNotFoundException, logi
from testlib.debuggertestcase import DebuggerTestCase
from testlib.timelimited import timelimited, TimeLimitExpired
import logging
class RunnerParams:
    """ Parameters used to initialize the testcase runner process """
    def __init__(self,
                 test_client,
                 test_device,
                 test_dir,
                 exe_path,
                 cl_kernels_dir,
                 port,
                 logfile,
                 load_func,
                 testcase_queue,
                 result_queue):

        self.daemon = False
        self.test_client = test_client
        self.test_dir = test_dir
        self.exe_path = exe_path
        self.cl_kernels_dir = cl_kernels_dir
        self.port = port
        self.logfile = logfile
        self.test_device = test_device

        # Function for testsuite discovery
        self.load_func = load_func

        # Queue of work and output
        self.testcase_queue = testcase_queue
        self.result_queue = result_queue


class RunnerProcess(Process):
    """ Threaded debugger testcase runner """

    def __init__(self, configuration):
        Process.__init__(self)
        self.config = configuration
        # TODO: the below is required to not crash on windows, but results in output
        # being printed twice somehow...
        self.stream = StringIO()
        self.runner = unittest.TextTestRunner(verbosity=2, stream=self.stream)

        if self.config.test_client == "gdb":
            from testlib.clientgdb import ClientGDB
            self.CLIENT_CLASS = ClientGDB
        elif self.config.test_client == "cdb":
            from testlib.clientcdb import ClientCDB
            self.CLIENT_CLASS = ClientCDB
        elif self.config.test_client == "simulator":
            from testlib.clientsimulator import ClientSimulator
            self.CLIENT_CLASS = ClientSimulator

    def make_client(self):
        return self.CLIENT_CLASS(
                        debuggee_exe_path=self.config.exe_path,
                        device_type=self.config.test_device,
                        cl_dir_path=self.config.cl_kernels_dir,
                        server_port=self.config.port,
                        logfile=self.config.logfile)


    def get_expected_fail_and_unexpected_passes(self, result):
        """ Extracts the number of expected failures and unexpected passes
            from a TestResult in an old-python-friendly way.
        """
        expected_fail = 0
        unexpected_pass = 0

        # Only the unittest library from Python 2.7 onwards supports expected fail
        if sys.version_info >= (2, 7):
            expected_fail = len(result.expectedFailures)
            unexpected_pass = len(result.unexpectedSuccesses)

        return (expected_fail, unexpected_pass)

    def result_details(self):
        """ Formats a unittest.TestResult object in a string summary
        """
        if not self.result:
            return "(no recorded results)"

        fail = len(self.result.failures)
        error = len(self.result.errors)
        (expected_fail, unexpected_pass) = self.get_expected_fail_and_unexpected_passes(self.result)

        passes = self.result.testsRun - fail - error - expected_fail - unexpected_pass

        details = "("
        details += str(passes) + " ok"
        if error > 0:
            details += ", " + str(error) + " error"
        if fail > 0:
            details += ", " + str(fail) + " fail"
        if expected_fail > 0:
            details += ", " + str(expected_fail) + " expected-fail"
        if unexpected_pass > 0:
            details += ", " + str(unexpected_pass) + " unexpected-pass"

        details += " of " + str(self.result.testsRun) + " run)"
        return details

    def record_result(self, name, details, success, expected):
        """ Record a failure for the current test suite.
            name is the name of the suite
            success is a True/False based on success
            expected is True/False if the result is expected
        """
        self.config.result_queue.put((name, details, success, expected))
        self.config.testcase_queue.task_done()

    def run_test(self):
        """ Runs test suite and saves result in self.result """
        self.result = self.runner.run(self.suite)

    def run(self, timeout=1800):
        """ Test suite runner: retrieves suites from the work queue and runs the tests.
            Allows up to timeout seconds for each suite to run before reporting a timeout.
            Default timeout = 30min
        """

        logging.basicConfig(stream=sys.stderr, level=logging.DEBUG)

        # create a testcase runner client (gdb driver/simulator) for the worker thread
        client = self.make_client()

        while True:
            success = False
            expected = False
            self.test_name = None
            details = "(not run)"

            try:
                if self.config.testcase_queue.empty():
                    return

                # grab the next testcase from the work queue
                self.test_name = self.config.testcase_queue.get(timeout=5)
                test_path = os.path.join(self.config.test_dir, self.test_name)

                # build the suite
                self.suite = unittest.TestSuite()
                for klass in self.config.load_func(test_path):
                    self.suite.addTest(DebuggerTestCase.create(
                                    klass,
                                    use_gdb=self.config.test_client == "gdb",
                                    use_cdb=self.config.test_client == "cdb",
                                    client=client,
                                    dtt_exe_logfile=None))

                # execute the suite for a maximum of timeout seconds
                timelimited(timeout, self.run_test)

            except TestSuiteNotFoundException as e:
                details = "Unable to find test suite: " + str(e)
            except TimeLimitExpired:
                details = "(timed out)"
            except KeyboardInterrupt:
                details = "(aborted)"
                return
            except Empty:
                details = "(empty queue)"
            except Exception as e:
                details = "Exception " + str(e) + "\n" + traceback.format_exc()
            except:
                details = "(Unknown unhandled exception!!!)"
            else:
                logi("Finished suite: " + self.test_name)
                details = self.result_details()
                (expected_fail, unexpected_pass) = self.get_expected_fail_and_unexpected_passes(self.result)
                success = self.result.wasSuccessful() and expected_fail == 0
                expected = unexpected_pass == 0 \
                    and len(self.result.failures) == 0 \
                    and len(self.result.errors) == 0
            finally:
                if client is not None:
                    # Attempt to gracefully (then not so gracefully) shutdown
                    # GDB (if client initialized correctly)
                    client.terminate_debuggee()

                # record_results() must be called for each item from test_queue or driver may hang
                if self.test_name is not None:
                    self.record_result(self.test_name, details, success, expected)
                self.test_name = None
                output = self.stream.getvalue()

                # Limit printing of output to non-empty workers
                if len(output) > 0:
                    logi(output)
