from testlib.debuggertestcase import DebuggerTestCase


class SimpleFuncCalls(DebuggerTestCase):
    CLNAME = 'simple_func_calls.cl'
    
    def test_stepping(self):
        self.client.execute_debuggee(
            hostprog_name='ndrange_inout',
            cl_name=self.CLNAME)
        self.client.connect_to_server()
        self.client.start_session(0, 0, 0)
        
        bp = (self.CLNAME, 11)
        file, line = self.client.debug_run([bp])
        self.assertEqual((file, line), bp)
        
        # on line 12 - the first foo() call
        self.assertEqual(self.client.debug_step_in(), (self.CLNAME, 12))
        
        # now step into first foo() call
        self.assertEqual(self.client.debug_step_in(), (self.CLNAME, 1))
        self.assertEqual(self.client.debug_step_in(), (self.CLNAME, 3))
        
        # step out of the first foo() call
        self.assertEqual(self.client.debug_step_out(), (self.CLNAME, 13))
        
        # step over to the second call
        self.assertEqual(self.client.debug_step_over(), (self.CLNAME, 14))

        # step over the second call, and to the third call
        self.assertEqual(self.client.debug_step_over(), (self.CLNAME, 15))
        self.assertEqual(self.client.debug_step_over(), (self.CLNAME, 16))
        
        # step into the third call
        self.assertEqual(self.client.debug_step_in(), (self.CLNAME, 1))

        # step through the whole code of foo()
        for i in range(5):
            file, line = self.client.debug_step_over()
        
        self.assertEqual((file, line), (self.CLNAME, 18))
        
        # done
        self.client.debug_run_finish()

    def test_step_over_breakpoint(self):
        # Test stepping over a function call in which a breakpoint is placed,
        # as well as out of a function when there's a breakpoint on the way.
        #
        self.client.execute_debuggee(
            hostprog_name='ndrange_inout',
            cl_name=self.CLNAME)
        self.client.connect_to_server()
        self.client.start_session(0, 0, 0)
        
        # Place a breakpoint on the second line of foo as well as on the 
        # first call to foo from main_kernel
        bps = [(self.CLNAME, 4), (self.CLNAME, 12)]
        self.assertEqual(self.client.debug_run(bps), bps[1])

        # Now stepping over, expect to stop on the breakpoint inside foo
        self.assertEqual(self.client.debug_step_over(), bps[0])
        
        # get to the next call to foo and step in, get to line 3
        self.assertEqual(self.client.debug_step_out(), (self.CLNAME, 13))
        self.client.debug_step_over()
        self.client.debug_step_in()
        self.assertEqual(self.client.debug_step_over(), (self.CLNAME, 3))
        
        # now step out of foo. should still stop on BP on line 4
        self.assertEqual(self.client.debug_step_out(), (self.CLNAME, 4))
        
        # stepping out again now takes us back to main_kernel
        self.assertEqual(self.client.debug_step_out(), (self.CLNAME, 15))

        self.client.debug_run_finish()

