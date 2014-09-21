from testlib.debuggertestcase import DebuggerTestCase

class BasicStopOnBp(DebuggerTestCase):
    CLNAME = 'simple_buffer_copy.cl'

    def test_run_stop_on_one_bp(self):
        self.client.execute_debuggee(
            hostprog_name='ndrange_inout',
            cl_name=self.CLNAME)
        self.client.connect_to_server()
        self.client.start_session(1, 0, 0)

        bps = [(self.CLNAME, 4)]
        self.assertEqual(self.client.debug_run(bps), bps[0])
        self.client.debug_run_finish(bps)

    def test_run_stop_on_two_bps(self):
        self.client.execute_debuggee(
            hostprog_name='ndrange_inout',
            cl_name=self.CLNAME)
        self.client.connect_to_server()
        self.client.start_session(1, 0, 0)

        bps = [(self.CLNAME, 4), (self.CLNAME, 5)]
        self.assertEqual(self.client.debug_run(bps), bps[0])
        self.assertEqual(self.client.debug_run(bps), bps[1])
        self.client.debug_run_finish(bps)
