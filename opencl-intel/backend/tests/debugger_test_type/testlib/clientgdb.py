
# Native debugger test type GDB client
import traceback

import os
import sys
from pexpect_2_4 import pexpect
import re
import types
import string
import subprocess

from common import os_is_windows, find_on_path, GDBContinueError, StopReason, loge, logd, logw, TestClient

from clientsimulator import ClientSimulator, SimulatorError
from distutils.spawn import find_executable

RPC_TIMEOUT = 30
RPC_RETRY_DELAY = 10
GDB_PROMPT_TIMEOUT = 100
USE_GWORKITEM = False

class ClientError(SimulatorError): pass
class ClientTimeout(SimulatorError): pass

class ClientGDB(TestClient):
    """ Implements the GDB-side protocol for communicating with
        testcases.
    """

    ### Regular expressions for parsing things GDB prints ###
    # Extracts the type name from the output of the "whatis" GDB command
    # into group 2 of the matchobject returned by re.search().
    REGEX_TYPE = "(type\ =\ )(.*)(\ \(gdb\))"
    REGEX_BREAKPOINT = "Breakpoint\ [\d]+.*\ at\ (.*):([\d]+)"

    # Sometimes, the breakpoint fires at the correct source line, but GDB is unable
    # to parse that source line later on. The OCL plugin still knows the sourceline,
    # and prints it, so it can be read here and we can just warn about this GDB bug,
    # rather than fail the test. The regex below extracts the source information from
    # the output of the OCL plugin.
    REGEX_BREAKPOINT_BACKUP = "Stopping at (.*):([\d]+)"

    # Based on seeming random things, GDB will print out a frame location with or
    # without the address of the PC.
    # In all cases, the frame location is prefixed by a frame number
    REGEX_LOCATION_PREFIX = "#"
    # Handle location format like "func_name () at file_name.txt:4
    REGEX_LOCATION_POSTFIX_1 = "\s+([^\ ]*)\s+\(.*\)\s+at\s+(.*)\n"
    # Handle location format like "0xabcdef in func_name () at file_name.txt:4
    REGEX_LOCATION_POSTFIX_2 = "\s+0x[^\ ]*\s+in\s+([^\ ]*)\s+\(.*\)\s+at\s+(.*)\n"
    # Handle location format like "0xabcdef in func_name ()
    REGEX_LOCATION_POSTFIX_3 = "\s+0x[^\ ]*\s+in\s+([^\ ]*)\s+\(.*\)\s(\d)?"
    # Takes a line from 'info locals' of the form "varname = varvalue" and
    # Groups the varname in matchobject.group(1) when used with re.search()
    REGEX_VARIABLE_NAME = "([^\ ]*)\s+=\s+.*$"

    # To be used with re.match() in order to determine if a type is an array.
    REGEX_IS_ARRAY = ".*\[\d+\].*"

    # Applies to ([u]char) values printed from GDB: groups the value from the ASII
    REGEX_SPLIT_VALUE_FROM_ASCII = "(.*)\ '(.*)'$"

    # Extracts the bp number from the message GDB prints when a bp is set
    REGEX_BREAKPOINT_SET = "Breakpoint\ (\d+)\ "

    # Strings to expect(): possible GDB responses to "continue"
    REGEX_GDB_PROMPT = "\(gdb\)"
    REGEX_GDB_QUESTION = "\(y or n\)"
    REGEX_PROGRAM_DONE = "Host program finished"
    REGEX_GDB_ERROR = "gdb.error"

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
        """
        # FIXME: Get this from some configuration object.
        ci_gdb_location = os.path.join('/opt', 'tools', 'bin', 'gdb')
        if os.path.exists(ci_gdb_location):
            # use GDB from installed location on CI machines
            self.gdb_command = ci_gdb_location
        else:
            # default to system gdb
            self.gdb_command = "gdb"
        """
        # Using -1 (an invalid id) as a flag for uninitialized global IDs
        self.gid_x = -1
        self.gid_y = -1
        self.gid_z = -1

        self.child = None
        self.existing_breakpoints = []

    def __del__(self):
        """ Cleanup (and kill subprocess)
        """

        self.terminate_debuggee()

    def terminate_debuggee(self):
        """ Force debuggee termination
        """

        if self.child is not None:
            self._shutdown_gdb()

    def reset(self):
        self.terminate_debuggee()
        self.exitcode = None
        self.child = None
        self.started = False
        self.existing_breakpoints = []

    def execute_debuggee(self, hostprog_name, cl_name, options={}, extra_args=[]):
        """ Loads the debuggee as a subprocess, under GDB
        """

        if self.child is not None:
            self.reset()

        exe_dir, exe_name = os.path.split(self.debuggee_exe_path)
        cwd = os.path.abspath(exe_dir)

        # setup debug server arguments
        cl_file_fullpath = self.cl_abs_filename(cl_name)
        # GDB environment variable should be set to gdb path before start of test
        gdb_command = os.environ['GDB']+'/gdb'
        options_str = ""
        if self.device_type:
            options_str = 'device=' + self.device_type + ','
        options_str += ','.join('%s=%s' % (k, v) for k, v in options.items())
        if not options_str:
            options_str = 'none'
        args = [gdb_command,
                # On specific gdb versions, the debuggee is not getting LD_LIBRARY_PATH and in some conficutations,
                # that may cause problesm. This hack will pass it to the debuggee explicitly.
                "-iex 'set exec-wrapper env LD_LIBRARY_PATH=" + os.environ.get('LD_LIBRARY_PATH', "") + "'",
#                "localhost:12345",
                "--args",
                self.debuggee_exe_path,
                hostprog_name,
                options_str,
                "'" + cl_file_fullpath + "'"]
        args += map(str, extra_args)
        # env -i will remove the python2 environment for gdb process
        # This is needed when GDB is built with python3, while tests are running in python2
        # The only environment variable, that we will keep for client process is LD_LIBRARY_PATH
        cmd = "env -i " + " ".join(args)

        logd("Testing debugger command: " + cmd)
        self.child = pexpect.spawn(cmd ,
                                   logfile=self.logfile
                                   #,env=os.environ
                                   )
        logd("Process started, pid=" + str(self.child.pid))

        try:
            # Give GDB up to 100 seconds to display a prompt. More tests
            # running in parallel cause GDB to take longer to display this
            # first prompt. In manual testing, 10s is sufficient for up to
            # 16 tests running concurrently.
            self._expect_prompt(GDB_PROMPT_TIMEOUT)
        except pexpect.TIMEOUT:
            # We should not be timing out, but warn if we do. Things may be OK
            # and GDB is just a bit slow to start...
            logw("Timeout waiting for GDB prompt after startup")
            pass

        # Disable GDB's warning about unknown source locations (for JITted code)
        self._command("set breakpoint pending on")

        # Disable GDB paging, line wrapping and python scripts autoloading
        self._command("set pagination off")
        self._command("set width 0")
        self._command("set auto-load python-scripts off")

        # Search for, and attempt to pre-load, the OpenCL GDB plugin.
        # Theoretically it should be loaded automatically when libintelocl.so
        # is loaded by the subprocess, but sometimes we want to set breakpoints
        # before that happens, so we pre-load it here.
        search_path = ""
        if os_is_windows():
            search_path = os.environm["PATH"]
        else:
            search_path = os.environ["LD_LIBRARY_PATH"]
        ocl_plugin_path = find_on_path("libintelocl.so-gdb.py", search_path)

        if ocl_plugin_path is not None:
            self._command("source " + ocl_plugin_path)
        else:
            logw("Unable to find OpenCL GDB plugin on  path " + str(search_path) + "." \
              + " Test may still succeed if GDB auto-loads plugin correctly.")

    def _command(self, command, timeout=100):
        """ Issue a command to GDB and returns the output.
        """

        logd("DEBUGGER: " + command)
        self.child.sendline(command)
        output = ""
        try:
            output = self._expect_prompt(timeout)
        except ClientTimeout as e:
            loge("Detected GDB timeout issuing command: " + command)
            raise e

        return output

    def send_message_to_server_wrong_size(self, message, size):
        pass

    def _expect_prompt(self, timeout):
        """
        Reads GDB's output stream until a prompt (or question or error)
        and returns it. Raises pexpect.TIMEOUT after timeout seconds.
        """
        index = self.child.expect([ClientGDB.REGEX_GDB_PROMPT,
                                   ClientGDB.REGEX_GDB_QUESTION,
                                   ClientGDB.REGEX_PROGRAM_DONE,
                                   ClientGDB.REGEX_GDB_ERROR,
                                   pexpect.TIMEOUT],
                                   timeout=timeout)
        output = self.child.before
        if self.child.after != None:
            output += self.child.after
        if index < 3:
            pass
            # Uncomment the line below to see all output from GDB.
            logd("GDB expected output: \n" + output)
        elif index == 3:
            logw("GDB INTERNAL ERROR output=" + output)
        elif index == 4:
            raise ClientTimeout("Timed out after " + str(timeout) + " seconds:" + output)
        else:
            loge("UNHANDLED OUTPUT: index=" + str(index) + ":\n" + output)

        return output

    def connect_to_server(self, retry=True, timeout=150):
        """ No-op for GDB """

    def start_session(self, gid_x, gid_y, gid_z, timeout=10):
        """ Send 'start' command to GDB """
        self.gid_x = gid_x
        self.gid_y = gid_y
        self.gid_z = gid_z

        if USE_GWORKITEM:
            output = self._command("set env GWORKITEM=(" + str(gid_x) + ","
                                   + str(gid_y) + "," + str(gid_z) + ")");
        # When multiple tests are running at the same time, GDB may take
        # longer to return from the "start" command.
        output = self._command("start", timeout=200);
        logd("result of start command: " + output)
        self.started = True

    def debug_run(self, breakpoints, timeout=900):
        """ Set the given breakpoints and issue a 'continue' command in GDB.
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
        """ Set the given breakpoints and issue a 'run' command in GDB.
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
        self._command("step")
        return self._frame_location()

    def debug_step_over(self, timeout=100):
        # this should test an OCL-specific step command
        self._command("next")
        return self._frame_location()

    def debug_step_out(self, timeout=100):
        # this should test an OCL-specific step command
        self._command("finish")
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

        ret = []
        for output in [self._command("info args"), self._command("info locals")]:
            # Remove the first line (which is the 'info locals' command echoed back)
            # and the last line (which is the '(gdb)' prompt)
            ret.extend([line for line in output.split("\r\n")[1:-1] if not re.match('^[ }]', line)])
        return ret

    def var_query_type(self, varname, stackframe=None):
        if stackframe is not None and stackframe != 0:
            self._frame(stackframe)
        return self._print_type(varname)

    def var_query_size(self, varname, stackframe=None):
        if stackframe is not None and stackframe != 0:
            self._frame(stackframe)
        return int(self._print("sizeof(" + varname + ")"))

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
        output = self._command("set var " + str(varname) + "=" + str(value))
        logd("result of set command: " + output)
        return output

    def get_stack_trace(self):
        # Not needed in GDB...users should call stack_query_* directly
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
        """ No-op as GDB client does not require a server port. """
        pass

    def get_server_port(self):
        """ No-op as GDB client does not require a server port. """
        return None

    def _shutdown_gdb(self, timeout=30):
        """ Graceful shutdown of the GDB subprocess (issue 'quit' command).
            May block for maximum 2 * timeout seconds waiting for exit.
        """
        if self.child.isalive():
            try:
                self.child.sendline('quit')
                self.child.close()
            except (pexpect.ExceptionPexpect, OSError) as e:
                logd ("Unable to send 'quit' command to GDB. May have exited?")

        # Print the final state of the child. Normally isalive() should be FALSE.
        if self.child.isalive():
            logd('Child did not exit gracefully.')
        else:
            logd('Child exited gracefully.')

    def _set_breakpoints(self, breakpoints, timeout=100):
        """ Updates OpenCL breakpoints set in GDB """

        if self.gid_x == -1 or self.gid_y == -1 or self.gid_z == -1:
            raise ClientError("Uninitialized breakpoint global ID")

        # Create list of location strings "filname:lineno"
        breakpoint_locations = ["\"" + b[0] + "\":" + str(b[1]) for b in breakpoints]

        # Examine existing breakpoints
        deleted = []
        for (number, location) in self.existing_breakpoints:
            if location in breakpoint_locations:
                # we already have a breakpoint at location; do not set a new breakpoint
                breakpoint_locations.remove(location)
            else:
                # breakpoint (number, location) no longer needed; remove it
                self._command("delete " + str(number))
                deleted.append((number, location))
        for x in deleted:
            self.existing_breakpoints.remove(x)

        # Set new breakpoints
        for loc in breakpoint_locations:
            if USE_GWORKITEM:
                args = ["b", loc]
            else:
                args = ["ocl-break-workitem", loc,
                    str(self.gid_x), str(self.gid_y), str(self.gid_z)]
            output = self._command(" ".join(args))
            s = re.search(ClientGDB.REGEX_BREAKPOINT_SET, output)
            if s is not None:
                breakpoint_number = s.group(1)
                self.existing_breakpoints.append((breakpoint_number, loc))
            else:
                logw("Unable to parse breakpoint number from GDB output:\n" \
                    + output)

    def _continue(self, timeout):
        """ Issues a 'continue' command in GDB and waits up to timeout * 2 seconds for the
            inferior to stop (breakpoint hit, program exited/crashed)
        """

        output = self._command("continue", timeout)

        if "Breakpoint" in output:
            source_file, line_number = "unknown", 0
            location_list = []

            s = re.search(ClientGDB.REGEX_BREAKPOINT, output)
            if s is None:
                s = re.search(ClientGDB.REGEX_BREAKPOINT_BACKUP, output)
                if s is not None:
                    logw("GDB is unable to parse source location anymore, but breakpoint from " \
                      + s.group(1) + ":" + s.group(2) + " has fired; testcase continuing.")
                else:
                    raise ClientError("Unable to parse breakpoint location from GDB output: " + output)

            try:
                line_number = int(s.group(2))
                source_file = os.path.basename(s.group(1)).lstrip("\"").rstrip("\"")
            except (ValueError, IndexError) as e:
                raise ClientError("Unable to parse breakpoint location line " \
                  + " number from GDB output location list: " + str(location_list))

            logd("breakpoint hit at location: " + source_file + ":" + str(line_number))
            return (StopReason.BREAKPOINT_HIT, (source_file, line_number))

        elif re.search(ClientGDB.REGEX_PROGRAM_DONE, output) is not None:
            self.exitcode = 0
            return (StopReason.PROGRAM_EXITED, self.exitcode)
        elif "internal-error" in output \
          or "gdb.error" in output \
          or "Couldn't get registers: No such process." in output:
            raise GDBContinueError(output)
        elif "The program is not being run." in output:
            return (StopReason.NOT_RUNNING, output)
        else:
            raise ClientError("Unhandled stop reason:\n'" + output + "'")

    def _print(self, expression):
        """ Executes the GDB 'print' command and processes the output
            to be closer to the expected formatting in the testcases.
            Note: To improve formatting, this function also invokes
                  the 'whatis' command.
        """

        gdb_string = self._command("print " + expression)

        # Convert any newlines GDB prints to spaces
        gdb_string = " ".join(gdb_string.split())

        # we need to know the type so we can work around GDB's subtleties
        t = self._print_type(expression)
        if re.match(ClientGDB.REGEX_IS_ARRAY, t):
            element_type = self._print_type(expression + "[0]")
            logd("found array type, element type = " + element_type)

        # remove prompt characters
        gdb_string = gdb_string.rstrip("\ (gdb)")

        # remove "$1 = " string gdb prepends
        gdb_string = re.sub(".*\$\d+\ =\ ", "", gdb_string)
        gdb_string = gdb_string.lstrip()

        # remove type name (in parens) that gdb sometimes prints
        gdb_string = re.sub("\(.*\)\ ", "", gdb_string)

        # remove starting '{' character and ending '}' character
        if gdb_string[0] == "{":
            gdb_string = gdb_string[1:]
        if gdb_string[-1] == "}":
            gdb_string = gdb_string[:-1]

        # split vector components so we can process each individually
        cmps = gdb_string.split(", ")
        for idx, vector_component in enumerate(cmps):
            if self._must_fix_float(vector_component, t):
                # WORKAROUND: non-pointer float value missing a ".0" so we add
                cmps[idx] = vector_component + ".0"

            if vector_component.startswith("0x"):
                # Remove marker of the form "<Address 0x22 out of bounds>" that GDB
                # adds when it cannot dereference a pointer.
                vector_component = re.sub("\ (.*)", "", vector_component)

                # Turn hex values into integers.
                cmps[idx] = str(int(vector_component, 16))

            # GDB sometimes prints ASCII representation too with a value,
            # here we remove it.
            s = re.search(ClientGDB.REGEX_SPLIT_VALUE_FROM_ASCII, vector_component)
            if s is not None:
                cmps[idx] = s.group(1)

        ret = ",".join(cmps)
        ret = ret.replace(' ', '')
        logd("result of print command: " + ret)
        return ret

    def _must_fix_float(self, v, t):
        """ Returns true if the value v being printed of type t must have a ".0" appended. """
        return "." not in v and "*" not in t and ("float" in t or "double" in t)

    def _print_type(self, expression):
        gdb_string = self._command("whatis " + expression)

        # Convert any weird newlines GDB prints to spaces
        gdb_string = " ".join(gdb_string .split())

        # Apply the type-finding regex to extract the type name in group 2
        s = re.search(ClientGDB.REGEX_TYPE, gdb_string)
        if s is not None:
            gdb_string = s.group(2)
        else:
            raise ClientError("ERROR Parsing type from GDB output: no match for " \
              + ClientGDB.REGEX_TYPE + " in: \n" + gdb_string )

        # Update types to match testcases
        gdb_string = re.sub("long int", "long", gdb_string)
        gdb_string = re.sub("long unsigned int", "unsigned long", gdb_string)
        gdb_string = re.sub("struct \{\.\.\.\}", "struct <unnamed>", gdb_string)
        gdb_string = re.sub("\ \*", "*", gdb_string) # 'char *' ==> 'char*'
        gdb_string = re.sub("\ \[", "[", gdb_string) # 'uint [3]' ==> 'uint[3]'

        return gdb_string

    def _frame(self, frame_number):
        """
        Returns (source_location, function_name) tuple from specified frame number
        """

        frame_info = self._command("frame " + str(frame_number)).rstrip()

        source_location = None
        function_name = None

        for gdb_location_format in [ClientGDB.REGEX_LOCATION_POSTFIX_1,
                                    ClientGDB.REGEX_LOCATION_POSTFIX_2,
                                    ClientGDB.REGEX_LOCATION_POSTFIX_3]:

            s = re.search(ClientGDB.REGEX_LOCATION_PREFIX + str(frame_number) \
                        + gdb_location_format, frame_info)
            if s is not None:
                function_name = s.group(1)
                source_location = s.group(2)
                return (source_location, function_name)

        raise ClientError("Unable to parse GDB location format with any regex. " \
                        + "frame number:\n" + str(frame_number) \
                        + "\nOriginal string:\n" + frame_info )

    def _frame_location(self, stackframe=0):
        """
        Returns a tuple (filename, lineno) representing the current source location
        """
        (source_location, function_name) = self._frame(stackframe)
        if source_location is not None:
            (filename, lineno) = source_location.split(":")
            return (os.path.basename(filename), int(lineno))
        return ("Unknown source location in frame " + str(stackframe), 0)
