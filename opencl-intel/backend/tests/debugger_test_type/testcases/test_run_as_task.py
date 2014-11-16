from testlib.debuggertestcase import DebuggerTestCase


class TestRunAsTask(DebuggerTestCase):
    CLNAME = 'printf_tester2.cl'

    def test_run_as_task(self):
        # Test - test the ability to run a program using enqueTask command
        # TC-65
        # Just run a trivial kernel
        #
        self.client.execute_debuggee(
            hostprog_name='task',
            cl_name=self.CLNAME)
        self.client.connect_to_server()
        self.client.start_session(3, 0, 0)

        self.client.debug_run_finish()
