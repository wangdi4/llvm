from testlib.debuggertestcase import DebuggerTestCase

class JitReloadTest(DebuggerTestCase):
    CLNAME = 'simple_buffer_copy.cl'

    def test_run_stop_on_one_bp(self):
        self.client.execute_debuggee(
            hostprog_name='jit_reload',
            cl_name=self.CLNAME)
        self.client.connect_to_server()
        self.client.start_session(1, 0, 0)

        bps = [(self.CLNAME, 4)]
        self.assertEqual(self.client.debug_run(bps), bps[0])
        self.client.debug_run_finish(bps)
