import os
import platform
import re
import socket
import subprocess
import sys
import time

from timelimited import timelimited, TimeLimitExpired
from debugservermessages_pb2 import ClientToServerMessage, ServerToClientMessage
from common import logi, os_is_windows, TestClient
import protocol
import debuginfo

class SimulatorError(Exception): pass

class ClientSimulator(TestClient):
    """ Simulates a debugger client. Allows invocation of a debuggee and
        communication with the debug server embedded in it.

        This class provides both high and low-level APIs.

        Note: ClientSimulator is intended to serve for a single invocation of
        the server and communication with it. Call .reset() if you need it to
        be ready for another invocation.
    """
    def __init__(self,
                 debuggee_exe_path,
                 cl_dir_path,
                 logfile,
                 device_type=None,
                 server_port=56203):
        self.debuggee_exe_path = debuggee_exe_path
        if os.path.split(self.debuggee_exe_path)[0] == '':
            self.debuggee_exe_path = './' + self.debuggee_exe_path
        self.cl_dir_path = cl_dir_path
        self.logfile = logfile
        self.server_port = server_port
        self.subproc = None

        self.reset()

    def reset(self):
        """ Reset the simulator.
        """
        if self.subproc is not None:
            self.terminate_debuggee()
            self.wait_for_debuggee_exit()

        self.subproc = None
        self.socket = None

        # filled in when we get START_SESSION_ACK
        self.server_sizeof_size_t = None

        # keeps track of the amount of RUN commands sent to the server
        self.num_run_commands = 0

        # Vars information received in the last BP_HIT reply from the server
        self.last_vars_info = None

        # Stack trace info received in the last STACK_TRACE_INFO reply from
        # the server
        self.last_stack_trace_info = None

        # The saved stderr output of the debuggee process
        self.saved_stderr = None

    def set_server_port(self, port):
        """ Set the server port.

            Note: to be effective, this must be called before execute_debuggee
            and connect_to_server.
        """
        self.server_port = port

    def get_server_port(self):
        """ Retrieve the server port """
        return self.server_port

    def execute_debuggee(self, hostprog_name, cl_name, options={}, extra_args=[]):
        """ Loads the debuggee as a subprocess.

            hostprog_name: the host program to run
            cl_name: the cl file run
            options: options to pass to the debuggee
            extra_args: extra arguments to pass to the debuggee
        """
        if self.subproc is not None:
            raise SimulatorError('Subprocess already running')

        # setup environment for debug server execution
        env = os.environ.copy()
        env['CL_CONFIG_DBG_ENABLE'] = '1'
        env['CL_CONFIG_DBG_PORT_NUMBER'] = str(self.server_port)

        if os_is_windows():
            import _winreg as winreg
            # put an enabling flag into the registry
            hkey = winreg.CreateKey(
                winreg.HKEY_CURRENT_USER,
                'Software\\Intel\\OpenCL\\Debugger\\__global')
            winreg.SetValueEx(hkey, 'CL_CONFIG_DBG_ENABLE', 0, winreg.REG_SZ, '1')

        exe_dir, exe_name = os.path.split(self.debuggee_exe_path)
        cwd = os.path.abspath(exe_dir)

        # setup debug server arguments
        cl_file_fullpath = self.cl_abs_filename(cl_name)
        options_str = ','.join('%s=%s' % (k, v) for k, v in options.iteritems())
        if not options_str:
            options_str = 'none'

        args = [    self.debuggee_exe_path,
                    hostprog_name,
                    options_str,
                    cl_file_fullpath]
        args += map(str, extra_args)

        # run
        self.subproc = subprocess.Popen(
                            args, cwd=cwd, env=env,
                            stdout=subprocess.PIPE, stderr=subprocess.PIPE)

    def terminate_debuggee(self):
        """ Force debuggee termination.
        """
        if self.subproc:
            self.subproc.poll()
            if self.subproc.returncode is None:
                self.subproc.kill()

    def wait_for_debuggee_exit(self):
        """ Wait for the debuggee to finish running. Return the pair (rc, log)
            where rc is the subprocess return code and log is its stderr output.
        """

        if self.subproc is None:
            # This happens when we use skipNotGDB decorator
            return 0, ''
        rc = self.subproc.poll()
        if rc is None:
            # process is still alive
            stdout, stderr = self.subproc.communicate() # also waits...
            if self.saved_stderr is None:
                self.saved_stderr = stderr + stdout
            return self.subproc.returncode, str(self.saved_stderr)
        else:
            # process has already exited
            return rc, ''

    def connect_to_server(self, retry=True, timeout=150):
        """ Connect to the server with a socket. If retry is True, will try to
            reconnect if the connection fails.

            The timeout (in seconds) specifies how long to wait for the
            connection. If no connection succeeded within the timeout, raise
            SimulatorError.
        """
        def do_connect():
            self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            address = ('localhost', self.server_port)
            logi('connect_to_server(timeout=%s)' % timeout)

            while True:
                try:
                    logi('Client trying to connect...')
                    self.socket.connect(address)
                    return
                except socket.error:
                    if retry:
                        time.sleep(0.8)
                    else:
                        return

        try:
            timelimited(timeout, do_connect)
            logi('... Connection succeeded.')
        except TimeLimitExpired:
            raise SimulatorError('failed to connect within timeout')

    def send_message_to_server(self, message):
        """ Send a ClientToServerMessage to the server.
        """
        protocol.send_message(self.socket, message)

    def send_message_to_server_wrong_size(self, message, size):
        """ Send a ClientToServerMessage to the server.
        """
        protocol.send_message_wrong_size(self.socket, message, size)

    def get_message_from_server(self, timeout=80):
        """ Get a ServerToClientMessage from the server.

            Message receiving will always be done with a timeout, which is
            specified in seconds. If nothing was received within a timeout,
            raise SimulatorError.
        """
        try:
            return timelimited(timeout, protocol.get_message, self.socket)
        except TimeLimitExpired:
            raise SimulatorError('failed to get a message within timeout')

    def start_session(self, gid_x, gid_y, gid_z, timeout = 80):
        """ Send START_SESSION to the server, with the given GID.

            If START_SESSION_ACK is received successfully in reply, return
            peacefully. Otherwise raises an exception.
        """
        msg = ClientToServerMessage()
        msg.type = ClientToServerMessage.START_SESSION
        msg.start_session_msg.global_id_x = gid_x
        msg.start_session_msg.global_id_y = gid_y
        msg.start_session_msg.global_id_z = gid_z
        self.send_message_to_server(msg)

        # expect a fast response to START_MESSAGE, with the default timeout
        reply = self.get_message_from_server(timeout)
        self._expect_reply_type(reply, ServerToClientMessage.START_SESSION_ACK)

        self.sizeof_size_t = reply.start_session_ack_msg.sizeof_size_t
        debuginfo.set_sizeof_size_t(self.sizeof_size_t)

    def debug_run(self, breakpoints, timeout=100):
        """ Issue a RUN command to the server, with the given breakpoints - list
            of (cl_file_name, line_num) pairs. A timeout for receiving a reply
            can be specified optionally. For the first RUN sent, the timeout
            is automatically increased to account for the DLL load times.

            Return the breakpoint that was hit - (file, line) pair. The file
            is only the cl_name (not the full absolute path).
            Save the variable information internally for later enquiries with
            var_query_* methods.

            Note: this method DOES NOT expect the subprocess to finish running,
            but expects to get a BP_HIT message. Use debug_run_finish to end
            a run.
        """
        self._send_run_message(breakpoints)
        if self.num_run_commands == 1:
            timeout += 400

        try:
            reply = self.get_message_from_server(timeout)
            #~ print reply
            self._expect_reply_type(reply, ServerToClientMessage.BP_HIT)
        except socket.error:
            raise SimulatorError('Server exited unexpectedly')

        bp_info = reply.bphit_msg.breakpoint
        self.last_vars_info = reply.bphit_msg.vars
        self.last_stack_trace_info = None # irrelevant now
        return str(os.path.split(bp_info.file)[1]), bp_info.lineno

    def debug_run_finish(self, breakpoints=[], timeout=100):
        """ Issue a RUN command to the server, with the given breakpoints.
            Expects the subprocess to finish running with return code 0 (exit
            cleanly). Otherwise, raises SimulatorError.
        """
        self._send_run_message(breakpoints)
        if self.num_run_commands == 1:
            timeout += 400

        try:
            rc, stderr = timelimited(timeout, self.wait_for_debuggee_exit)
        except TimeLimitExpired:
            raise SimulatorError('run did not finish within timeout')

        if rc != 0:
            raise SimulatorError('Server exited with rc = %s. Stderr: \n%s' %
                    (rc, str(stderr)))

    def debug_step_in(self, timeout=100):
        """ Issue a SINGLE_STEP_IN command to the server. Return the file, line
            where execution stopped after the step.
        """
        return self._debug_step(ClientToServerMessage.SINGLE_STEP_IN, timeout)

    def debug_step_over(self, timeout=100):
        """ Issue a SINGLE_STEP_OVER command to the server. Return the file, line
            where execution stopped after the step.
        """
        return self._debug_step(ClientToServerMessage.SINGLE_STEP_OVER, timeout)

    def debug_step_out(self, timeout=100):
        """ Issue a SINGLE_STEP_OUT command to the server. Return the file, line
            where execution stopped after the step.
        """
        return self._debug_step(ClientToServerMessage.SINGLE_STEP_OUT, timeout)

    def var_list(self, stackframe=None):
        """ Return a list of visible variable names.

            If stackframe=None, looks at the currently stopped location in the
            kernel code. Otherwise, stackframe specifies the number of stack
            frame (0 - current, 1 - one up, etc.) to take the list of variables
            from. Note that stackframe=0 has no variable info.

            Expects RUN to have run.
            The returned list is sorted by name.
        """
        self._expect_did_run()
        if stackframe is None:
            vars_list = self.last_vars_info
        else:
            self.get_stack_trace()
            self._expect_have_stack_info()
            vars_list = self.last_stack_trace_info.frames[stackframe].vars
        return sorted([var.name for var in vars_list])

    def var_query_type(self, varname, stackframe=None):
        """ Return the type of the variable named 'varname'.
            See doc of var_list for explanation of stackframe.
        """
        var_info = self._find_var_info(varname, stackframe)
        return debuginfo.var_info_type_as_string(var_info)

    def var_query_size(self, varname, stackframe=None):
        """ Return the size (in bytes) of the variable named 'varname'.
            See doc of var_list for explanation of stackframe.
        """
        var_info = self._find_var_info(varname, stackframe)
        return debuginfo.var_info_type_size(var_info)

    def var_query_value(self, varname, stackframe=None):
        """ Return the value (as a string) of the variable named 'varname'.
            See doc of var_list for explanation of stackframe.
        """
        # make error message more comprehensible for a common mistake
        if (stackframe is not None) and (not isinstance(stackframe, int)):
            raise SimulatorError('integer expected for stacktrace argument')
        var_info = self._find_var_info(varname, stackframe)
        start_addr, end_addr = debuginfo.var_info_memory_range(var_info)
        self._get_memory_range(start_addr, end_addr)

        reply = self.get_message_from_server()
        #~ print reply
        self._expect_reply_type(reply, ServerToClientMessage.MEMORY_RANGE_INFO)
        return debuginfo.var_info_value(var_info, reply.memory_range_info_msg.buf)

    def var_query_range(self, start_addr, end_addr, stackframe=None):
        """ Return the value (as a string) of the variable named 'varname'.
            See doc of var_list for explanation of stackframe.
        """
        # make error message more comprehensible for a common mistake
        self._get_memory_range(start_addr, end_addr)

        reply = self.get_message_from_server()
        #~ print reply
        self._expect_reply_type(reply, ServerToClientMessage.MEMORY_RANGE_INFO)
        return reply

    def get_stack_trace(self):
        """ Get a stack trace from the server and store it locally for later
            enquiries with var_query_* and stack_query_*.
        """
        self._expect_did_run()
        msg = ClientToServerMessage()
        msg.type = ClientToServerMessage.GET_STACK_TRACE
        self.send_message_to_server(msg)

        reply = self.get_message_from_server()
        #~ print reply.stack_trace_info_msg
        self._expect_reply_type(reply, ServerToClientMessage.STACK_TRACE_INFO)
        self.last_stack_trace_info = reply.stack_trace_info_msg

    def stack_query_func_name(self, stackframe):
        """ Query the function name of the stackframe number.
            stackframe is the number of frame in the stack of call frames
            (0 - current, 1 - one up, etc.)
        """
        self._expect_have_stack_info()
        return self.last_stack_trace_info.frames[stackframe].func_name

    def stack_query_call_location(self, stackframe):
        """ Query the call location of the stackframe number. Return
            file, line pair.
        """
        lineinfo = self.last_stack_trace_info.frames[stackframe].call_line
        return str(os.path.split(lineinfo.file)[1]), lineinfo.lineno

    #-------------------------------  PRIVATE  -------------------------------#

    def _send_run_message(self, breakpoints):
        """ Send a RUN message to server.
        """
        msg = ClientToServerMessage()
        msg.type = ClientToServerMessage.RUN
        msg.run_msg.info = ''
        if not isinstance(breakpoints, list):
            raise SimulatorError("'run' command expected a list of breakpoints")
        for bp in breakpoints:
            bpinfo = msg.run_msg.breakpoints.add()
            bpinfo.file = self.cl_abs_filename(bp[0])
            bpinfo.lineno = bp[1]
        self.send_message_to_server(msg)
        self.num_run_commands += 1

    def _debug_step(self, kind, timeout=100):
        """ Issue a SINGLE_STEP_* command to the server. Return the file, line
            where execution stopped after the step.

            'kind' must be one of the valid ClientToServerMessage.SINGLE_STEP_*
            message types.
        """
        msg = ClientToServerMessage()
        msg.type = kind
        self.send_message_to_server(msg)

        try:
            reply = self.get_message_from_server(timeout)
            self._expect_reply_type(reply, ServerToClientMessage.BP_HIT)

            bp_info = reply.bphit_msg.breakpoint
            self.last_vars_info = reply.bphit_msg.vars
            self.last_stack_trace_info = None # irrelevant now
            return str(os.path.split(bp_info.file)[1]), bp_info.lineno
        except socket.error:
            raise SimulatorError('Server exited unexpectedly')

    def _get_memory_range(self, start_addr, end_addr):
        """ Send a GET_MEMORY_RANGE message to the server and receive the reply.
            Return the memory buffer read from the server.
        """
        msg = ClientToServerMessage()
        msg.type = ClientToServerMessage.GET_MEMORY_RANGE
        msg.get_memory_range_msg.start_addr = start_addr
        msg.get_memory_range_msg.end_addr = end_addr
        self.send_message_to_server(msg)

    def _expect_reply_type(self, msg, expected_type):
        """ Expect the type of msg to by expected_type. Otherwise raise
            SimulatorError.
        """
        if msg.type != expected_type:
            raise SimulatorError('Expected %s reply. Got msg:\n%s' % (
                        expected_type, msg))

    def _expect(self, cond, err_msg):
        """ Expect the condition to be true. Otherwise raise SimulatorError with
            the given error message.
        """
        if not cond:
            raise SimulatorError(err_msg)

    def _expect_did_run(self):
        """ Expect that at least a single RUN command was executed. Otherwise
            raise a SimulatorError.
        """
        if self.num_run_commands == 0:
            raise SimulatorError('Expected RUN to have been executed at least once')

    def _expect_have_stack_info(self):
        """ Expect to have stack trace info.
        """
        self._expect(self.last_stack_trace_info is not None,
                     'Up-to-date stack frame info missing')

    def _find_var_info(self, varname, stackframe=None):
        """ Find the VarInfo entry for this variable.
            If stackframe is None, looks in last_vars_info (the vars of the
            current function). Otherwise, looks at the corresponding stack
            frame.
        """
        if stackframe is None:
            vars_list = self.last_vars_info
        else:
            self._expect_have_stack_info()
            vars_list = self.last_stack_trace_info.frames[stackframe].vars

        for var in vars_list:
            if var.name == varname:
                return var
        raise SimulatorError('Looking for inexistent variable "%s"' % varname)

    def __del__(self):
        try:
            self._shutdown()
        except:
            pass # ignore errors here

    def _shutdown(self):
        if self.subproc is not None:
            if self.subproc.returncode is None:
                self.subproc.kill()
            self.subproc.wait()
            self.subproc = None
