from testlib.debuggertestcase import DebuggerTestCase


class NoDebugBuild(DebuggerTestCase):
    CLNAME = 'simple_buffer_copy.cl'

    def test_stop_with_debug_build(self):
        # Sanity-check test. Without passing the debug_build=off flag, should
        # stop normally
        self.client.execute_debuggee(
            hostprog_name='ndrange_inout',
            cl_name=self.CLNAME)
        self.client.connect_to_server()
        self.client.start_session(0, 0, 0)

        bp = (self.CLNAME, 4)
        self.assertEqual(self.client.debug_run([bp]), bp)
        self.client.debug_run_finish()

    def test_no_stop_with_debug_build_off(self):
        # Pass debug_build=off - expect the test *not* to stop on a breakpoint.
        # debug_run_finish will finish successfully, even with the breakpoint
        # passed.
        self.client.execute_debuggee(
            hostprog_name='ndrange_inout',
            options={'debug_build': 'off'},
            cl_name=self.CLNAME)
        self.client.connect_to_server()
        self.client.start_session(0, 0, 0)

        bp = (self.CLNAME, 4)
        self.client.debug_run_finish([bp])
