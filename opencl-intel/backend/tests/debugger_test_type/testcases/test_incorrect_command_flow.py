from testlib.debuggertestcase import DebuggerTestCase

class TestIncorrectCommandFlow(DebuggerTestCase):
    CLNAME = 'simple_program.cl'
    ERROR_MSG_SIM = 'Expected 3 reply. Got msg:\ntype: CMD_ERROR\ncmd_error_msg {\n  description: "Expected a START_SESSION command"\n}\n'
    ERROR_MSG_GDB = "Debug session has not been started"
    AFTER_LOOP_ROW = 7
    def test_no_start_session_message(self):
    #
    #  Test - test that the Debug-Server respond with the right error after sending commands without first sending a start session message
    #  TC-2
        self.client.execute_debuggee(
            hostprog_name='ndrange_inout',
            cl_name=self.CLNAME)
        self.client.connect_to_server()
        bp = (self.CLNAME, self.AFTER_LOOP_ROW)
        try:
            self.client.debug_run([bp])
        except Exception as e:
            if self.use_gdb or self.use_cdb:
                # GDB error
                self.assertEqual(self.ERROR_MSG_GDB, str(e))
            else:
                # Simulator error
                self.assertEqual(self.ERROR_MSG_SIM, str(e))
        else:
            self.assertEqual('this should not happen', '!!!')
