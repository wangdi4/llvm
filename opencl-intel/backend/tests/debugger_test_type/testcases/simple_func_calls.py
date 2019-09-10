from testlib.debuggertestcase import DebuggerTestCase, expectedFailureCDB


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

        # on the starting line of main_kernel
        self.assertEqual(self.client.debug_step_in(), (self.CLNAME, 12))

        # now step into first foo() call
        if self.use_cdb:
          self.assertEqual(self.client.debug_step_in(), (self.CLNAME, 2))
        if not self.use_gdb and not self.use_cdb:
          self.assertEqual(self.client.debug_step_in(), (self.CLNAME, 1))
        self.assertEqual(self.client.debug_step_in(), (self.CLNAME, 3))

        # step out of the first foo() call
        location = self.client.debug_step_out()
        if self.use_gdb or self.use_cdb:
          # Note: step_out works incorrectly with GDB.
          # See test_steps.py tests for details.
          self.assertEqual(location, (self.CLNAME, 12))
          self.client.debug_step_over()
        else:
          self.assertEqual(location, (self.CLNAME, 13))

        # step over to the second call
        self.assertEqual(self.client.debug_step_over(), (self.CLNAME, 14))

        # step over the second call, and to the third call
        self.assertEqual(self.client.debug_step_over(), (self.CLNAME, 15))
        self.assertEqual(self.client.debug_step_over(), (self.CLNAME, 16))

        # step into the third call
        location = self.client.debug_step_in()
        if not self.use_gdb:
            # The simulator stops on the opening brace of a function whereas
            # GDB stops on the first statement. On the simulator, issue another
            # "step" so we end up in the same place.
            location = self.client.debug_step_over()
        self.assertEqual(location, (self.CLNAME, 3))

        # step through the whole code of foo()
        for i in range(4):
            file, line = self.client.debug_step_over()
        if self.use_cdb:
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
        location = self.client.debug_step_out()
        if self.use_gdb or self.use_cdb:
          # Note: step_out works incorrectly with GDB.
          # See test_steps.py tests for details.
          self.assertEqual(location, (self.CLNAME, 12))
          self.client.debug_step_over()
        else:
          self.assertEqual(location, (self.CLNAME, 13))

        self.client.debug_step_over()

        location = self.client.debug_step_in()
        if not self.use_gdb:
            # The simulator stops on the opening brace of a function whereas
            # GDB stops on the first statement. On the simulator, issue another
            # "step" so we end up in the same place.
            location = self.client.debug_step_over()
        self.assertEqual(location, (self.CLNAME, 3))

        # now step out of foo. should still stop on BP on line 4
        self.assertEqual(self.client.debug_step_out(), (self.CLNAME, 4))

        # stepping out again now takes us back to main_kernel
        if self.use_gdb or self.use_cdb:
          # Note: step_out works incorrectly with GDB.
          # See test_steps.py tests for details.
          self.assertEqual(self.client.debug_step_out(), (self.CLNAME, 14))
        else:
          self.assertEqual(self.client.debug_step_out(), (self.CLNAME, 15))

        self.assertEqual(self.client.debug_run(bps), bps[0])
        self.client.debug_run_finish()
