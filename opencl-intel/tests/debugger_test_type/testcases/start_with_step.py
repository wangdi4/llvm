from testlib.debuggertestcase import DebuggerTestCase


class StartWithStep(DebuggerTestCase):
    # Test that the debug server will correctly start running when the first
    # command sent to it after START_SESSION is SINGLE_STEP_IN
    #
    CLNAME = 'simple_func_calls.cl'
    
    def test_start_with_step1(self):
        self.client.execute_debuggee(
            hostprog_name='ndrange_inout',
            cl_name=self.CLNAME)
        self.client.connect_to_server()
        self.client.start_session(0, 0, 0)
        
        # First send a SINGLE_STEP_IN
        # Increase timeout since this is the initial run
        self.assertEqual(self.client.debug_step_in(timeout=45), (self.CLNAME, 9))
        
        # Now send a RUN
        bp = (self.CLNAME, 16)
        self.assertEqual(self.client.debug_run([bp]), bp)
        
        # done
        self.client.debug_run_finish()
