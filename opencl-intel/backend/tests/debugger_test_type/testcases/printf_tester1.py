from testlib.debuggertestcase import DebuggerTestCase


class PrintfTester1(DebuggerTestCase):
    CLNAME = 'printf_tester1.cl'

    def test_printf_in_simple_kernel(self):
        # Just run a trivial kernel
        # The real verification is done by the host program (which returns 1 in
        # case of an error)
        #
        self.client.execute_debuggee(
            hostprog_name='printf_tester',
            cl_name=self.CLNAME)
        self.client.connect_to_server()
        self.client.start_session(3, 0, 0)

        bps = [(self.CLNAME, 8)]
        self.assertEqual(self.client.debug_run(bps), bps[0])

        self.client.debug_run_finish(bps)
