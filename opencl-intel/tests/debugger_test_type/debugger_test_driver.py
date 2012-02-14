import os
import imp
import inspect
import platform
import sys
import unittest
import time

# Note: DO NOT import modules that need (even indirectly) protobuf at module
# level, without calling setup_path first. 
# setup_path makes sure the protobuf library will be found
from testlib.debuggertestcase import DebuggerTestCase



def get_testcase_filenames(dir):
    """ Yield all testcase filesnames from given directory.
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
    codeobj = compile(open(filename, 'rU').read(), filename, 'exec')
    exec codeobj in module.__dict__
    
    for name, klass in inspect.getmembers(module, inspect.isclass):
        if klass.__bases__[0] == DebuggerTestCase:
            yield klass


def setup_path():
    """ Sets up the Python module import path to find protobuf.
    """
    sys.path.insert(0, os.path.join(os.path.dirname(sys.argv[0]), 
                                    'protobuf_lib.zip'))


def run_tests(names=None):
    """ Collect test cases and run them. If 'names' is passed, it's taken as 
        a list of testcases (file names relative to the testcases/ dir) to 
        run. Otherwise, auto-discovers and runs all test cases.
        
        Return the unittest.TestResult
    """
    from testlib.clientsimulator import ClientSimulator, os_is_windows
        
    DTT_EXE_NAME = 'debugger_test_type'
    if os_is_windows():
        DTT_EXE_NAME += '.exe'

    CL_KERNELS_DIR = 'cl_kernels'
    SERVER_PORT = 56203

    my_dir = os.path.dirname(sys.argv[0])
    testcases_dir = os.path.join(my_dir, 'testcases')
    dtt_exe_log_filename = os.path.join(my_dir, 'dtt_exe_log.txt')
    dtt_exe_logfile = open(dtt_exe_log_filename, 'w')

    suite = unittest.TestSuite()

    if names is not None:
        testcases_iterator = iter(names)
    else:
        testcases_iterator = get_testcase_filenames(testcases_dir)

    for testcase_filename in testcases_iterator:
        testcase_path = os.path.join(my_dir, 'testcases', testcase_filename)
        for klass in load_testcase_classes(testcase_path):
            client = ClientSimulator(
                        debuggee_exe_path=os.path.join(my_dir, DTT_EXE_NAME),
                        cl_dir_path=os.path.join(my_dir, CL_KERNELS_DIR),
                        server_port=SERVER_PORT)
            suite.addTest(DebuggerTestCase.create(
                            klass, 
                            client=client,
                            dtt_exe_logfile=dtt_exe_logfile))
    
    return unittest.TextTestRunner(verbosity=2).run(suite)


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
        
        sim.connect_to_server(timeout=1.6)
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


#---------------------------------------------------------------------------
if __name__ == "__main__":
    import logging
    logging.basicConfig(stream=sys.stderr, level=logging.ERROR)
    
    setup_path()
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

    #### Run!
    if len(sys.argv) > 1:
        testresult = run_tests(sys.argv[1:])
    else:
        testresult = run_tests()
    
    if testresult.wasSuccessful():
        print 'TEST SUCCEEDED'
        sys.exit(0)
    else:
        print 'TEST FAILED'
        sys.exit(1)

    #### ZZZ: this runs when run_tests & sys.exit are commented out
    run_standalone()
