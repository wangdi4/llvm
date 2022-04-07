
OpenCL debugger_test_type Integration Tests
===========================================

This file contains a high-level overview of the Intel OpenCL debugger
test scripts.

The main test utility (debugger_test_driver.py) evaluates tests defined in the
'testcases' directory. Tests contain commands which are issued to a debugger
client as well as assertions to verify the state of the debugger. Tests use
OpenCL source files in the 'cl_kernels' directory.

To invoke the tests, use the debugger_test_driver.py script, which has the
following usage information:

Usage: debugger_test_driver.py [options] <testcases>

Options:
  -h, --help            show this help message and exit
  -v, --verbose         Print additional (debug) information during test runs
  -l, --log             Dump debugger client output to logfile (dtt_log.txt).
                        Not recommended with parallel test execution.
  -t TEST_CLIENT, --test_client=TEST_CLIENT
                        Run the specified test client {gdb|simulator}
  -j NUM_JOBS           number of tests to run in parallel
  -d DEVICE, --device DEVICE
                        Run on the specified device type {cpu|fpga_fast_emu}
                        If the options is not specified,
                        tests will be run on a default device

Testcases:
  Optional list of files from the 'testcases' directory to run.


Parallel Execution
==================
In order to support parallel test execution, the driver utility creates up to
num_jobs child worker processes (RunnerProcess) which communicate with the
parent using two queues of type multiprocessing.Queue. The first queue,
'testcase_queue' is initialized by the parent process (debugger_test_driver.py)
and contains test names which are used to initialize DebuggerTestCase objects.
The second queue, 'result_queue' is used by the RunnerProcess worker to report
test results to the parent. A RunnerProcess object runs tests from 'test_queue'
and pushes results to the 'result_queue' until the 'test_queue' is empty, at
which point the worker process stops. If there are any resources which must be
shared (i.e. available TCP/IP port numbers) it is the responsibility of the
driver to assign the shared resource to each RunnerProcess.


Test Clients
============
Currently, there are two debugger clients supported by DebuggerTestCase; the
simulator debugger and GDB. The debugger client to use is specified with the
'-t' option to debugger_test_driver.py, which is propagated to the
DebuggerTestCase object at initialization. The DebuggerTestCase is responsible
for creating the correct Client class (i.e. ClientSimulator or ClientGdb) to
execute the tests.

Simulator Tests
---------------
The simulator client interfaces with the OpenCL debugger server defined in
src/backend/ocl_cpu_debugging through a socket. In order to avoid socket
collisions, each RunnerProcess is assigned a unique port number. This port
number is propagated to ClientSimulator objects through the DebuggerTestCase
that spawns it.

GDB Tests
---------
The GDB client interfaces with GDB via stdin/stdout using the pexpect library;
a pure Python implementation of the classic 'expect'. The ClientGDB is
responsible for translating function calls issued by test cases into GDB
commands. An example of such a translation is:

    <in testcase>
    client.var_query_type('myvariable')

    <to GDB stdin>
    (gdb) whatis myvariable

In addition, the ClientGDB is responsible for doing the reverse; translating GDB
output into return values in the python test cases:

    <from GDB stdout>
    (gdb) whatis myvariable
    type = int

    <in testcase, return value from 'var_query_type' is 'int'>
    self.assertEqual('int', client.var_query_type('myvariable'))

In addition to driving GDB through stdin/stdout, tests require the ability to
break on a specific Global ID in a work item. In order to have this
functionality, it is necessary for GDB to have the OCL Plugin loaded.
The plugin is implemented in Python and is called:

    libintelocl.so-gdb.py

The name is important because GDB will auto-load python files that have the same
name as a loaded shared library (in this case, libintelocl.so).

Additonal Debugger Client Tests
-------------------------------
In order to add support for an additional debugger, create a class that
implements the following methods:

  terminate_debuggee(s):
    Force debuggee termination

  reset(s):
    Set client state so it is ready to execute a new testcase

  execute_debuggee(s, prog, cl, options, extra_args):
    Execute 'prog' <cl_kernels> <options> <extra_args> under a debugger.
    Where options are named options of type 'name=value' and extra_args
    are positional options.

  start_session(s, x, y, z):
    Start debugging the inferior program (which has already been started with
    execute_debuggee) and set the work-item that is being debugged to (x, y, z)

  debug_run(s, breakpoints, timeout):
    Set breakpoints specified by the 'breakpoints' array and continue the
    inferior program until either a breakpoint is hit, or timeout seconds have
    expired. Throw an exception if the program terminates instead of hitting a
    breakpoint. Returns a tuple (file, line) corresponding to the breakpoint
    that was hit.

  debug_run_finish(s, breakpoints, timeout):
    Set breakpoints specified by the 'breakpoints' array and continue the
    inferior program until either the program exits or timeout seconds have
    expired. Throw an exception if the program hits a breakpoint or exits with
    a non-0 return code. Returns nothing.

  wait_for_debuggee_exit(s, timeout):
    Wait up to timeout seconds for the debuggee to exit.
    Return a pair (exitcode, stderr) corresponding to the integer exit code of
    the inferior and any output the inferior may have printed to stderr.

  debug_step_[in|over|out] (s, timeout):
    Issue the specified step command. Throw an exception if timeout seconds
    expire before the debugger responds.

  var_list(s, stackframe):
    Return a list of local variables in the frame specified by stackframe, where
    0 corresponds to the bottom-most frame.

  var_query_[type|size|value] (s, varname, stackframe):
    Return the specified property (type, size, value) of the variable named
    'varname' in frame 'stackframe'.

  var_set_value(s, varname, value, stackframe):
    Assign 'value' to the variable 'varname' in frame 'stackframe'.

  stack_query_func_name(self, stackframe):
    Return the function name in frame 'stackframe'.

  stack_query_call_location(self, stackframe):
    Return a pair (location, function_name) corresponding to the call-site which
    resulted in frame 'stackframe' being invoked.

  set_server_port(s, port):
    If the client requires a TCP/IP socket, use port number 'port'.

  get_server_port(s):
    Return the port number used for TCP/IP communication with the debuggee.


Bugs
====
There are known differences between the ClientSimulator and ClientGDB in how
user-defined types and arrays of vectors are printed.

If you suspect a bug, run the failing test case with the "-v" or "-l" flags to
print verbose output or generate a log file, respectively. The ClientGDB will
then print the command-line used to invoke GDB, as well as all the commands
issued to GDB and all the GDB output received.
