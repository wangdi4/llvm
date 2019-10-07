
# Native debugger test type CDB client
import traceback

import os
import sys
import atexit

import re
import types
import string
import subprocess
import time
import pexpect
from pexpect import popen_spawn

from common import os_is_windows, find_on_path, GDBContinueError, StopReason, loge, logd, logw, TestClient

from clientsimulator import SimulatorError
from distutils.spawn import find_executable

RPC_TIMEOUT = 30
RPC_RETRY_DELAY = 10
CDB_PROMPT_TIMEOUT = 100
USE_GWORKITEM = False
USE_BREAKPOINTS2 = True

class ClientError(SimulatorError): pass
class ClientTimeout(SimulatorError): pass

class ClientCDB(TestClient):
    """ Implements the CDB-side protocol for communicating with
        testcases.
    """
    REGEX_EVAL_VARIABLE_SIZE = re.compile(',d +: ([\-.\d]+)')

    REGEX_EVAL_VARIABLE_NUM = re.compile(',d +: ([\-.\dtruefals]+) \[Type: ([_ A-Za-z0-9<>-]+)\]')
    REGEX_EVAL_VARIABLE_TYPE = re.compile('\[Type: (.+)\]')

    # example: &bb,d            : 0x1f5a5f520f : false [Type: bool *]
    REGEX_EVAL_VARIABLE_ADDR = re.compile(',d +: (0x[a-f0-9]+) : .+ \[Type: .*\]')

    # example: bb,d             : false [Type: bool]
    REGEX_EVAL_VARIABLE_BOOL = re.compile(',d +: ([truefals]+) \[Type: .*bool.*\]')

    # example: r3,d             : 0x43 : Unable to read memory at Address 0x43 [Type: int *]
    REGEX_EVAL_VARIABLE_PTR = re.compile(',d +: (.+) \[Type: ([_ A-Za-z0-9<>-]+ .*\*.*)\]')

    #array output example:
    # local_arr,d      [Type: int [4]]
    # [0]              : 32 [Type: int]
    # [1]              : 1 [Type: int]
    # [2]              : 1 [Type: int]
    # [3]              : 1 [Type: int]
    REGEX_EVAL_VARIABLE_ARR_HEADER = re.compile(',d +(: ".*" ){0,1}\[Type: ([_ A-Za-z0-9<>-]+) \[(\d+)\]\]')
    REGEX_EVAL_VARIABLE_ARR_ITEM = re.compile('\[(\d+)\] +: ([\-.\d]+) \[Type: ([_ A-Za-z0-9<>-]+)\]')

    REGEX_EVAL_VAR_LIST_ITEM = re.compile(' +([_A-Za-z0-9]+) = ')

    # CDB prompt describes the ThreadID.
    REGEX_CDB_PROMPT = "\d+:\d{3}>"
    REGEX_PROGRAM_DONE = re.compile("Host program finished")
    REGEX_CDB_ERROR = 'gdb.error' # TODO

    #Child-SP          RetAddr           Call Site
    #00000068`aa62cce0 00000068`ad4a61c0 u6kc!foo+0x44 [c:\shared\dev\xmain_repo\build_debug_llvm\tests\debugger_test_type_wip\cl_kernels\simple_func_calls.cl @ 4]
    REGEX_SOURCE_LOCATION=re.compile('!([^+ ]+)[ +].*\[(.+)\ @\ (\d+)\]')
    REGEX_EVAL_PID = re.compile('0n(\d+) [._A-Za-z0-9]+\.exe')

    tempfiles = []

    def __init__(self,
                 debuggee_exe_path,
                 device_type,
                 cl_dir_path,
                 server_port,
                 logfile):

        self.started = False
        self.debuggee_exe_path = debuggee_exe_path
        if os.path.split(self.debuggee_exe_path)[0] == '':
            self.debuggee_exe_path = './' + self.debuggee_exe_path
        self.cl_dir_path = cl_dir_path

        self.logfile = logfile
        self.device_type = device_type
        # Using -1 (an invalid id) as a flag for uninitialized global IDs
        self.gid_x = -1
        self.gid_y = -1
        self.gid_z = -1

        self.breakcondition=""

        self.child = None
        self.existing_breakpoints = []
        self.cleared_breakpoints = []

    def __del__(self):
        """ Cleanup (and kill subprocess)
        """

        self.terminate_debuggee()

    def terminate_debuggee(self):
        """ Force debuggee termination
        """

        if self.child is not None:
            self._shutdown_gdb()
        self.deleteTempFiles()

    def reset(self):
        self.terminate_debuggee()
        self.exitcode = None
        self.child = None
        self.started = False
        self.existing_breakpoints = []
        self.cleared_breakpoints = []

    def execute_debuggee(self, hostprog_name, cl_name, options={}, extra_args=[]):
        """ Loads the debuggee as a subprocess, under CDB
        """

        if self.child is not None:
            self.reset()

        env = os.environ.copy()
        env["CL_CONFIG_USE_NATIVE_DEBUGGER"] = "1"
        exe_dir, exe_name = os.path.split(self.debuggee_exe_path)
        cwd = os.path.abspath(exe_dir)

        # setup debug server arguments
        cl_file_fullpath = self.cl_abs_filename(cl_name)
        # CDB environment variable should be set to cdb path before start of test
        cdb_command = '"' + env["CDB"] + '"'
        options_str = ""
        if self.device_type:
            options_str = 'device=' + self.device_type + ','
        options_str += ','.join('%s=%s' % (k, v) for k, v in options.iteritems())
        if not options_str:
            options_str = 'none'
        args = [cdb_command,
                self.debuggee_exe_path,
                hostprog_name,
                options_str,
                "\"" + cl_file_fullpath + "\""]
        args += map(str, extra_args)
        cmd = " ".join(args)
        logd("Testing debugger command: " + cmd)
        self.child = pexpect.popen_spawn.PopenSpawn(cmd,
                                   logfile=self.logfile,
                                   env=env)
        logd("Process started, pid=" + str(self.child.pid))

        try:
            # Give CDB up to 100 seconds to display a prompt. More tests
            # running in parallel cause CDB to take longer to display this
            # first prompt. In manual testing, 10s is sufficient for up to
            # 16 tests running concurrently.
            self._expect_prompt(CDB_PROMPT_TIMEOUT)
        except pexpect.TIMEOUT:
            # We should not be timing out, but warn if we do. Things may be OK
            # and CDB is just a bit slow to start...
            logw("Timeout waiting for CDB prompt after startup")
            pass

        # Enable debugging with source line information
        self._command(".lines")

        # Switch from assembly mode to source mode. (For example step will progress a source line instead of a single instruction)
        self._command("L+t")

        # Disable child debugging. This seems to fix random breaks at loader events (ntdll.dll!_LdrpDoDebuggerBreak@0).
        # https://stackoverflow.com/questions/14376523/vs2012-breakpoint-in-ntdll-dll-at-debugger-launch-with-no-more-info
        #self._command(".childdbg 0")

        # This may improve deterministic test behavior, need to verify.
        self._command(".bpsync 1")

        # Don't download symbols from network.
        #self._command(".netsyms 0")

        #self._command(".Settings set EngineInitialization.SendTelemetry=false")
        #self._command(".Settings set EngineInitialization.AllowNetworkPaths=false")
        #self._command(".Settings set EngineInitialization.AllowShellCommands=false")
        #self._command(".Settings set EngineInitialization.VerifyFunctionTableCallbacks=false")
        #self._command(".Settings set Sources.SourceCodeDebugging=true")
        #self._command(".Settings set Symbols.SymbolOptions.IgnoreCVRecord=true")
        #self._command(".Settings set Symbols.SymbolOptions.SuppressErrors=true")

        # Disable CDB paging, line wrapping and python scripts autoloading

    def deleteTempFiles(self):
        for file in self.tempfiles:
            if os.path.isfile(file):
                os.remove(file)
        self.tempfiles = []

    def writeTempFile(self, filename, filecontent):
        f = open(filename, "w")
        self.tempfiles.append(filename)
        f.write(filecontent)
        f.close()

    def _command(self, command, timeout=100):
        """ Issue a command to CDB and returns the output.
        """

        logd("DEBUGGER: " + command)
        self.child.sendline(command)
        output = ""
        try:
            output = self._expect_prompt(timeout)
        except ClientTimeout as e:
            loge("Detected CDB timeout issuing command: " + command)
            raise e

        return output

    def send_message_to_server_wrong_size(self, message, size):
        pass

    def _expect_prompt(self, timeout):
        """
        Reads CDB's output stream until a prompt (or question or error)
        and returns it. Raises pexpect.TIMEOUT after timeout seconds.
        """
        index = self.child.expect([self.REGEX_CDB_PROMPT,
                                   pexpect.TIMEOUT],
                                   timeout=timeout)
        output = self.child.before
        if type(self.child.after) in types.StringTypes:
            output += self.child.after

        if index == 0:
            pass
            # Uncomment the line below to see all output from CDB.
            logd("CDB expected output: \n" + output)
        elif index == 1:
            logw("CDB TIMOUT output=" + output)
        else:
            loge("UNHANDLED OUTPUT: index=" + str(index) + ":\n" + output)

        return output

    def connect_to_server(self, retry=True, timeout=150):
        """ No-op for CDB """

    def get_debuggee_processid(self):
        output = self._command(".tlist -c")
        s = re.search(self.REGEX_EVAL_PID, output)
        if s is not None:
            return s.group(1)

    def start_session(self, gid_x, gid_y, gid_z, timeout=10):
        """ Send 'start' command to CDB """

        self.gid_x = gid_x
        self.gid_y = gid_y
        self.gid_z = gid_z

        if USE_GWORKITEM:
            # get memory address of gworkitem buffer

            gworkitem = "(" + str(gid_x) + "," + str(gid_y) + "," + str(gid_z) + ")"
            filename = os.path.dirname(self.debuggee_exe_path) + "/workitemfocus_inject_" + str(self.get_debuggee_processid()) + ".tmp"
            self.writeTempFile(filename, gworkitem)

            # inject gworkitem into cdbGWorkitemInjectionBuffer global buffer
            #cmd = "eb debugger_test_type!cdbGWorkitemInjectionBuffer " + self.stringToCdbArray(gworkitem)
            #output = self._command(cmd, timeout=200);
            #logd("result of gworkitem injection command: \n" + output)
        else:
            self.breakcondition = (" \"j (poi(__ocl_dbg_gid0)==0n%d & poi(__ocl_dbg_gid1)==0n%d & poi(__ocl_dbg_gid2)==0n%d) '.echo Breakpoint'; 'gc'\"" % (gid_x, gid_y, gid_z))

        self.started = True

    def stringToCdbArray(self, text):
        return "'" + "' '".join(list(text)) + "' 0"

    def debug_run(self, breakpoints, timeout=900):
        """ Set the given breakpoints and issue a 'continue' command in CDB.
                Returns the breakpoint that was hit - (file, line) pair.

                Note: this method DOES NOT expect the subprocess to finish running,
                but expects to get a BP_HIT message. Use debug_run_finish to end
                a run.
        """
        if not self.started:
            raise ClientError("Debug session has not been started")

        self._set_breakpoints(breakpoints)
        logd("continue (to breakpoint, timeout=" + str(timeout) + ")")

        try:
            (status, details) = self._continue(timeout)
            if status == StopReason.BREAKPOINT_HIT:
                return details
            elif status == StopReason.PROGRAM_EXITED:
                raise ClientError("Expected breakpoint but inferior exited " \
                                  "with code " + str(details))
            else:
                raise ClientError("Inferior stopped unexpectedly: " + str(details))
        except ClientTimeout as e:
            loge("Expecting breakpoint but 'continue' timed out")
            raise

    def debug_run_finish(self, breakpoints=[], timeout=900):
        """ Set the given breakpoints and issue a 'run' command in CDB.
                Returns nothing when the subprocess finishes with code 0.
                Otherwise, raises ClientError.
        """
        if not self.started:
            raise ClientError("Debug session has not been started")

        self._set_breakpoints(breakpoints)
        logd("continue (expect program exit, timeout=" + str(timeout) + ")")

        try:
            (status, details) = self._continue(timeout)
            if status == StopReason.PROGRAM_EXITED:
                self.exitcode = details
                if details != 0:
                    raise ClientError("Program exited with error code: " \
                      + str(details))
                else:
                    return
            elif status == StopReason.BREAKPOINT_HIT:
                raise ClientError("Expecting exit but hit breakpoint at " \
                  + str(details))
            elif status == StopReason.NOT_RUNNING:
                raise ClientError("Expecting exit but program not is not being run: " \
                  + str(details))
            else:
                raise ClientError("Inferior stopped unexpectedly: " + str(details))
        except ClientTimeout as e:
            loge("Expecting exit but 'continue' timed out")
            raise

    def wait_for_debuggee_exit(self, timeout=10):
        """ Wait for the debuggee to finish running. Return the pair (rc, log)
            where rc is the subprocess return code and log is its stderr output.
        """

        self.stderr = "FIXME: get stderr from process"

        if self.exitcode is not None:
            return (self.exitcode, self.stderr)

        if self.child is None:
            logw("wait_for_debuggee_exit() called after debuggee has been terminated (assuming -1 ec)")
            return (self.exitcode, self.stderr)

        return (self.exitcode, self.stderr)

    def debug_step_in(self, timeout=100):
        # this should test an OCL-specific step command
        self._command("~. T")
        return self._frame_location()

    def debug_step_over(self, timeout=100):
        # this should test an OCL-specific step command
        self._command("~. P")
        return self._frame_location()

    def debug_step_out(self, timeout=100):
        # this should test an OCL-specific step command
        self._command("~. GU")
        return self._frame_location()

    def debug_continue(self, timeout=100):
        # this should test an OCL-specific step command
        self._continue(timeout)
        return self._frame_location()

    def var_list(self, stackframe=None):
        """ Return a list of visible variable names.

        If stackframe=None, looks at the currently stopped location in the
        kernel code. Otherwise, stackframe specifies the number of stack
        frame (0 - current, 1 - one up, etc.) to take the list of variables
        from.

        Expects RUN to have run.
        The returned list is sorted by name.
        """
        if stackframe is not None and stackframe != 0:
            self._frame(stackframe)

        output = self._command("dv")
        vars = []
        for match in re.finditer(self.REGEX_EVAL_VAR_LIST_ITEM, output):
            vars.append(match.group(1))
        return vars

    def var_query_type(self, varname, stackframe=None):
        if stackframe is not None and stackframe != 0:
            self._frame(stackframe)
        return self._print_type(varname)

    def var_query_size(self, varname, stackframe=None):
        if stackframe is not None and stackframe != 0:
            self._frame(stackframe)
        return int(self._print("(int)sizeof(" + varname + ")"))

    def var_query_value(self, varname, stackframe=None):
        if stackframe is not None and stackframe != 0:
            self._frame(stackframe)
        return self._print(varname)

    def var_query_range(self, start_addr, end_addr, stackframe=None):
        if stackframe is not None and stackframe != 0:
            self._frame(stackframe)
        return self._print("*"+str(start_addr))+self._print("*"+str(start_addr))

    def var_set_value(self, varname, value, stackframe=None):
        if stackframe is not None and stackframe != 0:
            self._frame(stackframe)
        output = self._assign(varname, value)
        logd("result of set command: " + output)
        return output

    def get_stack_trace(self):
        # Not needed in CDB...users should call stack_query_* directly
        return 0

    def stack_query_func_name(self, stackframe):
        """ Query the function name of the stackframe number.
            stackframe is the number of frame in the stack of call frames
            (0 - current, 1 - one up, etc.)
        """
        (loc, func) = self._frame(stackframe if not None else 0)
        return func

    def stack_query_call_location(self, stackframe):
        """ Query the call location of the stackframe number. Return
                file, line pair.
        """
        return self._frame_location(stackframe + 1 if not None else 1)

    def set_server_port(self, port):
        """ No-op as CDB client does not require a server port. """
        pass

    def get_server_port(self):
        """ No-op as CDB client does not require a server port. """
        return None

    def _shutdown_gdb(self, timeout=30):
        """ Graceful shutdown of the CDB subprocess (issue 'quit' command).
            May block for maximum 2 * timeout seconds waiting for exit.
        """
        if not self.child._read_reached_eof:
            try:
                self.child.sendline('q') # quit

                # Wait for stream termination (EOF)
                index = self.child.expect([pexpect.EOF,
                                   pexpect.TIMEOUT],
                                   timeout=100)
                if index != 0:
                    logd("Failed to send 'quit' command to CDB. Timeout detected.")
            except:
                logd ("Failed to send 'quit' command to CDB. May have exited?")

        # Print the final state of the child. Normally isalive() should be FALSE.
        if self.child._read_reached_eof:
            logd('Child exited gracefully.')
        else:
            try:
                self.child.kill(0)
            except:
                logd ("Unable to kill child process. May have exited?")
            logd('Child did not exit gracefully.')

    def _set_breakpoints(self, breakpoints, timeout=100):
        """ Updates OpenCL breakpoints set in CDB """

        if USE_BREAKPOINTS2:
             self._set_breakpoints2(breakpoints, timeout)
             return

        if self.gid_x == -1 or self.gid_y == -1 or self.gid_z == -1:
            raise ClientError("Uninitialized breakpoint global ID")

        # Create list of location strings "filname:lineno"
        breakpoint_locations = ["`" + b[0] + ":" + str(b[1]) + "`" for b in breakpoints]
        isEqual = self.existing_breakpoints == breakpoints
        if isEqual:
            return
        self.existing_breakpoints = breakpoints[:]

        # Clear all previous breakpoints
        output = self._command("bc *")

        # Set new breakpoints
        for loc in breakpoint_locations:
            output = self._command("bu " + loc + self.breakcondition)

    def _set_breakpoints2(self, breakpoints, timeout=100):
        """ Updates OpenCL breakpoints set in CDB in a way that can improve determinism"""
        """ Multi-threaded applications execute in a non-deterministic manner.  This makes creating a
            scripted test for debugging of multi-threaded applications very difficult.  This routine
            attempts to minimize the impact of one issue, that being multiple threads hitting the
            same breakpoint at the same time.  One such scenario believed to be seen in the CDB tests
            is the following:
                - Two threads hit a conditional bp at the same time.
                - Thread #1 meets the condition and should cause execution to stop while thread #2
                  does not meet the condition.
                - The debugger processes thread #1 first and halts execution.
                - The event associated with thread #2 becomes pending and remains in the queue to be
                  processed.
                - The test then instructs the debugger to remove all breakpoints and continue execution.
                - The continue then allows the bp event for thread #2 to be seen again.
                - Since the bp and associated condition that triggered the event have been removed, the
                  debugger halts execution again.
                - This second halt of execution is not expected by the test and causes a failure.
            Setting ".bpsync 1" in CDB does not prevent this. I tried numerous ways to deal with this
            (e.g. ~* gh, bc *, bd *) but the approach below is the only one that seemed to work.

             The approach used below is to replace any deleted breakpoints with a breakpoint that will
             instruct the debugger to continue execution.  This should allow any pending breakpoint
             events to simply continue on unnoticed by the test.  This is not a preferred solution and
             will cause tests to run a bit slower.
        """

        if self.gid_x == -1 or self.gid_y == -1 or self.gid_z == -1:
            raise ClientError("Uninitialized breakpoint global ID")

        isEqual = self.existing_breakpoints == breakpoints
        if isEqual:
            return

        # Create list of new location strings "filname:lineno"
        new_breakpoint_locations = ["`" + b[0] + ":" + str(b[1]) + "`" for b in breakpoints]

        # Create list of old location strings "filname:lineno"
        old_breakpoint_locations = \
            ["`" + b[0] + ":" + str(b[1]) + "`" for b in self.existing_breakpoints]

        # Create list of previously cleared location strings "filname:lineno"
        cleared_breakpoint_locations = \
            ["`" + b[0] + ":" + str(b[1]) + "`" for b in self.cleared_breakpoints]

        # Delete all previous breakpoints
        output = self._command("bc *")

        # Reset breakpoints at previously cleared locations that stop but immediately continue
        for old_loc in old_breakpoint_locations:
            output = self._command("bu " + old_loc + " \"gc\"")

        # Set new breakpoints at the old locations that stop but immediately continue
        for old_loc in old_breakpoint_locations:
            output = self._command("bu " + old_loc + " \"gc\"")

        self.cleared_breakpoints.extend(self.existing_breakpoints)
        self.existing_breakpoints = breakpoints[:]

        # Set new breakpoints
        for new_loc in new_breakpoint_locations:
            output = self._command("bu " + new_loc + self.breakcondition)

    def _continue(self, timeout):
        """ Issues a 'continue' command in CDB and waits up to timeout * 2 seconds for the
            inferior to stop (breakpoint hit, program exited/crashed)
        """
        output = self._command("g", timeout)

        if "Breakpoint" in output:
            (source_file, line_number) = self._frame_location(0)
            source_file = os.path.basename(source_file).lstrip("\"").rstrip("\"")
            logd("breakpoint hit at location: " + source_file + ":" + str(line_number))
            return (StopReason.BREAKPOINT_HIT, (source_file, line_number))

        elif re.search(self.REGEX_PROGRAM_DONE, output) is not None:
            self.exitcode = 0
            return (StopReason.PROGRAM_EXITED, self.exitcode)
        #elif "internal-error" in output \
        #  or "gdb.error" in output \
        #  or "Couldn't get registers: No such process." in output:
        #    raise GDBContinueError(output)
        #elif "The program is not being run." in output:
        #    return (StopReason.NOT_RUNNING, output)
        else:
            raise ClientError("Unhandled stop reason:\n'" + output + "'")

    def _fixRounding(self, strnum):
        if strnum.find(".") != -1:
            floatnum = ("%g" % float(strnum))
            if floatnum.find(".") == -1:
                floatnum = floatnum + ".0"
            return floatnum
        else:
            return strnum

    def _print(self, expression):
        """ Executes the CDB 'print' command and processes the output
            to be closer to the expected formatting in the testcases.
            Note: To improve formatting, this function also invokes
                  the 'whatis' command.
        """

        # command: dx -n -r0 __ocl_dbg_gid0,d
        # returns: __ocl_dbg_gid0,d : 16 [Type: unsigned __int64]
        output = self._command("dx -n -r0 " + expression + ",d")

        s = re.search(self.REGEX_EVAL_VARIABLE_NUM, output)

        if s is not None:
            #logd(str(s.groups()))
            num_value = self._fixRounding(s.group(1))
            #num_type = s.group(2)
            ret = num_value
        else:
            s = re.search(self.REGEX_EVAL_VARIABLE_BOOL, output)
            if s is not None:
                logd(str(s.groups()))
                bool_value = s.group(1)
                ret = str(bool_value)
            else:
                s = re.search(self.REGEX_EVAL_VARIABLE_ARR_HEADER, output)
                if s is not None:
                    logd(str(s.groups()))
                    if len(s.groups()) == 3:
                        arr_size = int(s.group(3))
                        arr_type = s.group(2)
                    else:
                        arr_size = int(s.group(2))
                        arr_type = s.group(1)
                    cast_type = ""
                    if arr_type == "char":
                        cast_type = "(unsigned char[" + str(arr_size) + "])"
                    output = self._command("dx -n -r1 " + cast_type + expression + ",d")

                    myarr = []
                    for match in re.finditer(self.REGEX_EVAL_VARIABLE_ARR_ITEM, output):
                        myarr.append(self._fixRounding(match.group(2)))

                    if arr_size != len(myarr):
                        raise ClientError("Unable to parse array: " + output)
                    ret = ','.join(myarr)
                else:
                    s = re.search(self.REGEX_EVAL_VARIABLE_PTR, output)
                    if s is not None:
                        logd(str(s.groups()))
                        # if the variable is a pointer, then we need to return the address of the pointer in decimal encoding
                        output = self._command("dx -n -r0 (size_t)" + expression + ",d")
                        s = re.search(self.REGEX_EVAL_VARIABLE_NUM, output)
                        if s is not None:
                            num_value = int(s.group(1))
                            ret = str(num_value)
                        else:
                            raise ClientError("Unable to parse number: " + output)

        logd("result of print command: " + ret)
        return ret

    def _assign(self, varname, value):
        """ Executes the CDB 'print' command and processes the output
            to determine the proper 'set' command for the var type,
            then assigned the given value to the variable.
            Note: this only supports the few types tested by
            test_variable_setting.py.
        """

        # operating on a global var takes extra time in CDB
        timeout = 300
        value_str = str(value)
        output = self._print_type(varname)
        if output == "bool":
            """ Should be able to use the varname with the eb command but this
                did not work, so use the address of the var instead for now.
            """
            addr_output = self._command("dx -n -r0 &" + str(varname) + ",d")
            s = re.search(self.REGEX_EVAL_VARIABLE_ADDR, addr_output)
            if s is not None:
                var_address = s.group(1)
            else:
                raise ClientError("Unable to get address from: " + addr_output)
            if value_str == "\"true\"":
                num_value = 1
            else:
                if value_str == "\"false\"":
                    num_value = 0
                else:
                    raise ClientError("Unrecognized bool value for assign: " + value)
            ret = self._command("eb " + str(var_address) + " " + str(num_value), timeout)
        else:
            if output == "double":
                ret = self._command("eD " + str(varname) + " " + str(value), timeout)
            else:
                if output == "float":
                    ret = self._command("ef " + str(varname) + " " + str(value), timeout)
                else:
                    if output == "int":
                        ret = self._command("ed " + str(varname) + " " + str(value), timeout)
                    else:
                        raise ClientError("Assign to unsupported type: " + output)
        return ret

    def _must_fix_float(self, v, t):
        """ Returns true if the value v being printed of type t must have a ".0" appended. """
        return "." not in v and "*" not in t and ("float" in t or "double" in t)

    def _print_type(self, expression):
        output = self._command("dx -n -r0 " + expression)

        s = re.search(self.REGEX_EVAL_VARIABLE_TYPE, output)
        if s is not None:
            return s.group(1)
        else:
            raise ClientError("ERROR Parsing type from CDB output: no match for " \
              + self.REGEX_TYPE + " in: \n" + output )

    def _frame(self, frame_number):
        """
        Returns (source_location, function_name) tuple from specified frame number
        """
        output = self._command(".frame " + str(frame_number))
        s = re.search(self.REGEX_SOURCE_LOCATION, output)
        #logd(s.groups())
        if s is not None:
            try:
                fnname = s.group(1)
                loc = s.group(2)
                ln = s.group(3)
                return (loc + ":" + ln, fnname)
            except (ValueError, IndexError) as e:
                raise ClientError("Unable to parse breakpoint location " \
                  + "from CDB output location list: " + output)
        raise ClientError("Unable to parse breakpoint location from CDB output:\n" + output)

    def _frame_location(self, stackframe=0):
        """
        Returns a tuple (filename, lineno) representing the current source location
        """
        (source_location, function_name) = self._frame(stackframe)
        if source_location is not None:
            (filename, lineno) = source_location.rsplit(":", 1)
            return (os.path.basename(filename), int(lineno))
        return ("Unknown source location in frame " + str(stackframe), 0)
