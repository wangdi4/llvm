from testlib.debuggertestcase import DebuggerTestCase


class TestWithoutDebug(DebuggerTestCase):
    CLNAME = 'printf_tester1.cl'
    
    def test_execution_without_debug_option(self):
        # Test - test the ability to run a non-debugged program
        # TC-1
        # Just run a trivial kernel
        # The real verification is done by the host program (which returns 1 in
        # case of an error)
        #
        self.client.execute_debuggee(
            hostprog_name='printf_tester',
            options={'debug_build': 'off'},
            cl_name=self.CLNAME)
        self.client.connect_to_server()
        self.client.start_session(3, 0, 0)
        
        # timeout is increased here because I've been seeing intermittent
        # failures on x64 Debug with the default timer. Maybe the stdout 
        # redirection makes the run a bit longer
        self.client.debug_run_finish(timeout=6)
