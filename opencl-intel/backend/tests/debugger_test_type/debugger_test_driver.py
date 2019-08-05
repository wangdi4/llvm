import os
import imp
import inspect
import platform
import sys
import unittest
import time
import Queue
import logging
from optparse import OptionParser
from multiprocessing import cpu_count, JoinableQueue, Queue, Process

testlib_dir = os.path.abspath(os.path.join(os.path.dirname(sys.argv[0]), 'testlib'))
if not os.path.exists(testlib_dir):
  raise Exception("Unable to find debugger_test_type library directory " + testlib_dir)
sys.path.insert(0, testlib_dir)

for fname in ['pexpect_2_4.zip', 'protobuf_lib.zip']:
    path = os.path.abspath(os.path.join(os.path.dirname(sys.argv[0]), 'testlib', fname))
    if not os.path.exists(path):
        raise Exception("Unable to find python library archive at " + path)

    sys.path.insert(0, path)

from testlib.common import find_on_path, os_is_windows, TestSuiteNotFoundException, logd, logw, loge, loglevel
from testlib.debuggertestcase import DebuggerTestCase
from testlib.runner_process import RunnerProcess, RunnerParams

# Maximum number of seconds to wait (per thread) for exit after receiving
# a Ctrl-C event before sending a SIGTERM signal.
MAX_CTRL_C_TIMEOUT = 15

def parse_options():
    """ Parses commandline options and returns a tuple (options, positional_args)
    """
    parser = OptionParser()
    parser.add_option("-v", "--verbose",
                      dest="verbose", action="store_true",
                      default=False,
                      help="Print additional (debug) information during test runs")
    parser.add_option("-l", "--log",
                      dest="logfile", action="store_true",
                      default=False,
                      help="Dump debugger client output to logfile (dtt_log.txt). " \
                           "Not recommended with parallel test execution.")
    parser.add_option("-t", "--test_client",
                      dest="test_client", default="gdb",
                      help="Run the specified test type {gdb|cdb|simulator}")
    parser.add_option("-j", type="int",
                      dest="num_jobs",
                      default=cpu_count(),
                      help="number of tests to run in parallel")
    parser.add_option("-d", "--device", action="store", type="string",
                      dest="test_device",
                      default="",
                      help="Run on the specified device type")

    (opts, positionals) = parser.parse_args()

    # Check for invalid options
    if opts.num_jobs < 1:
        loge("Error: need more than 0 jobs (-j) to run tests")
        sys.exit(1)
    if opts.test_client != "gdb" and opts.test_client != "simulator" and opts.test_client != "cdb":
        loge("Error: please specify --test_client=[gdb|cdb|simulator]")
        sys.exit(1)

    return (opts, positionals)

def get_testcase_filenames(dir):
    """ Yield all testcase filenames from given directory.
    """
    for filename in os.listdir(dir):
        if not filename.startswith('__'):
            root, ext = os.path.splitext(filename)
            if ext == '.py':
                yield filename

def load_testcase_classes(filename):
    """ Load all testcase classes (children of DebuggerTestCase) from the given
        filename. Yield them one by one.
    """
    module = imp.new_module('testmodule')
    if not os.path.exists(filename):
        raise TestSuiteNotFoundException(filename)

    codeobj = compile(open(filename, 'rU').read(), filename, 'exec')
    exec codeobj in module.__dict__

    for name, klass in inspect.getmembers(module, inspect.isclass):
        if klass.__bases__[0] == DebuggerTestCase:
            yield klass

def run_tests(names = None, options=None):
    """ Collect test cases and run them. If 'names' is passed, it's taken as
        a list of test suites (file names relative to the testcases/ dir) to
        run. Otherwise, auto-discovers and runs all test cases.

        Return a Queue of TestResult objects (one per suite)
    """
    SERVER_START_PORT = 56203

    # Queues for synchronization
    test_queue = JoinableQueue()
    result_queue = Queue()

    # Initialize OCL parameters
    DTT_EXE_NAME = 'debugger_test_type'
    if os_is_windows():
        DTT_EXE_NAME += '.exe'

    CL_KERNELS_DIR = 'cl_kernels'

    my_dir = os.path.dirname(sys.argv[0])
    testcases_dir = os.path.join(my_dir, 'testcases')
    dtt_exe_log_filename = os.path.join(my_dir, 'dtt_exe_log.txt')

    search_path = my_dir
    if not os_is_windows():
        search_path += ":" + os.environ['LD_LIBRARY_PATH']
    else:
        search_path += ":" + os.environ['PATH']

    exe_path=find_on_path(DTT_EXE_NAME, search_path)
    if exe_path is None:
        exe_path=find_on_path(os.path.join('validation', 'debugger_test_type',
            DTT_EXE_NAME), search_path)
        if exe_path is None:
            loge("Unable to find executable " + DTT_EXE_NAME )
            sys.exit(1)

    cl_dir_path=os.path.join(os.path.dirname(exe_path), CL_KERNELS_DIR)

    # Build a list of test suites
    if len(names) > 0:
        for x in names:
            test_queue.put(x)
    else:
        for x in get_testcase_filenames(testcases_dir):
            test_queue.put(x)

    worker_processes = []
    logfile = None
    if options.logfile:
        logfile = open("dtt_log.txt", "w")

    #### Run!
    try:
        for i in range(options.num_jobs):
            p = RunnerProcess(RunnerParams(
                             test_client=options.test_client,
                             test_device=options.test_device,
                             test_dir=testcases_dir,
                             exe_path=exe_path,
                             cl_kernels_dir=cl_dir_path,
                             port=(SERVER_START_PORT + i),
                             logfile=logfile,
                             load_func=load_testcase_classes,
                             testcase_queue=test_queue,
                             result_queue=result_queue))
            p.start()
            worker_processes.append(p)

        # wait for tests to execute
        test_queue.join()

        # release thread(s) used to manage the work queue
        test_queue.close()
    except KeyboardInterrupt as e:
        logw("\nCtrl-C detected, test results may be incomplete. " \
            + "Shutting down child processes...")

        # Cleanup child processes
        for p in worker_processes:
            p.join(MAX_CTRL_C_TIMEOUT)
            if p.is_alive():
                logw("terminating worker process pid=" + str(p.pid))
                p.terminate()

    if options.logfile:
        logfile.close()
    return result_queue


def run_standalone():
    """ Run a session stand-alone, not as a test. Used for debugging.
    """
    from testlib.debugservermessages_pb2 import ClientToServerMessage, ServerToClientMessage
    from testlib.clientsimulator import ClientSimulator, SimulatorError

    CLNAME = 'nested_calls1.cl'
    LINE = 16

    my_dir = os.path.dirname(sys.argv[0])

    try:
        sim = ClientSimulator(
                    debuggee_exe_path=os.path.join(my_dir, 'debugger_test_type.exe'),
                    cl_dir_path=os.path.join(my_dir, 'cl_kernels'),
                    server_port=56203)

        sim.execute_debuggee(
                hostprog_name='1d_inout',
                cl_name=CLNAME)

        time.sleep(0.34)

        sim.connect_to_server(timeout=16)
        sim.start_session(1, 0, 0)

        print sim.debug_run([(
                    CLNAME, LINE)
                ])

        print sim.var_list()

        for varname in sim.var_list():
            print 'variable "%s"' % varname
            print '  type = %s' % sim.var_query_type(varname)
            print '  size = %s' % sim.var_query_size(varname)
            if sim.var_query_size(varname) == 0: continue
            print '  value = %s' % sim.var_query_value(varname)

        # and now one stack frame up!
        sim.get_stack_trace()

        print sim.var_list(stackframe=1)
        for varname in sim.var_list(stackframe=1):
            print 'variable "%s"' % varname
            print '  type = %s' % sim.var_query_type(varname, stackframe=1)
            print '  size = %s' % sim.var_query_size(varname, stackframe=1)
            if sim.var_query_size(varname, stackframe=1) == 0: continue
            print '  value = %s' % sim.var_query_value(varname, stackframe=1)

        sim.debug_run_finish([])
    except SimulatorError, e:
        print 'got SimulatorError:'
        print e

    time.sleep(1)

    sim.terminate_debuggee()
    rc, s = sim.wait_for_debuggee_exit()
    print 'rc =', rc
    print str(s)

def process_results(result_queue):
    """ Prints results summary and returns the exit code """
    passed = []
    unexpected_passed = []
    failed = []
    expected_failed = []
    timed_out = False
    total = result_queue.qsize()

    while not result_queue.empty():
      (name, details, success, expected) = result_queue.get()
      if success and expected:
          passed.append((name, details))
      elif success and not expected:
          unexpected_passed.append((name, details))
      elif not success and expected:
          expected_failed.append((name, details))
      else:
          failed.append((name, details))

      if "ClientTimeout" in details:
          timed_out = True

    print "\n===== Test Results Summary (" + str(total) + " Suites) ====="
    for (listname, l) in [("Failed", failed),
                          ("Failed (Expected)", expected_failed),
                          ("Passed (Unexpected)", unexpected_passed),
                          ("Passed", passed)]:
        if len(l) > 0:
            print listname + ": " + str(len(l))
            for (f, d) in l:
                print "\t" + f + " " + d

    # Should be TIMEOUT_RETCODE imported from framework/cmdtool.py
    if timed_out and platform.system() == 'Windows':
        return 127
    elif timed_out:
        return -9
    elif len(failed) == 0:
        # success
        return 0
    else:
        # fail
        return 1

#---------------------------------------------------------------------------
if __name__ == "__main__":
    #import logging

    os.environ['PATH'] = os.getcwd() + ';' + os.environ['PATH']

    dlls_path = None

    # ZZZ: Hack for running from inside source dir - for easier debugging.
    # Uncomment the relevant line. For production use this setup is not
    # necessary (it's the responsibility of validation scripts)

    #~ dlls_path = r'C:\work\branch_ebenders_dev\win32\__bin\bin.win32\Release'
    #~ dlls_path = r'C:\work\trunk_clean2\win_32\__bin\bin.win32\Debug'
    #~ dlls_path = r'C:\work\1.5-gold\win32\__bin\bin.win32\Debug'
    #~ dlls_path = r'C:\work\1.5-gold\win32\__bin\bin.win32\Release'
    #~ dlls_path = r'C:\work\sdk-2012\win64debug\__bin\bin.win64\Debug'
    #~ dlls_path = r'C:\work\debugger_merge_sdk2013\install\win64\Debug\bin'
    #~ dlls_path = r'C:\work\trunk\install\win64\Release\bin'
    #~ dlls_path = r'C:\work\sdk-2012\win64\__bin\bin.win64\Release'

    # Add extra DLL path if needed
    if dlls_path:
        os.environ['PATH'] = dlls_path + ';' + os.environ['PATH']

    #~ sys.argv.extend(['basic_stop_on_bp1.py'])
    #~ sys.argv.extend(['test_variable_types.py', 'test_variable_types_in_several_kernels.py'])

    (options, args) = parse_options()

    if options.verbose:
        loglevel(logging.DEBUG)
    else:
        loglevel(logging.INFO)

    result_queue = run_tests(options=options, names=args)

    retcode = process_results(result_queue)

    # Release thread(s) used to manage the result queue
    result_queue.close()

    sys.exit(retcode)

    #### ZZZ: this runs when run_tests & sys.exit are commented out
    run_standalone()
