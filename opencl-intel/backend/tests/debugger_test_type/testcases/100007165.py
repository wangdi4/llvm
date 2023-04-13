from testlib.debuggertestcase import DebuggerTestCase


class 100007165(DebuggerTestCase):
    def test_100007165(self):
        CLNAME = '100007165.cl'

        self.client.execute_debuggee(
            hostprog_name='ndrange_inout',
            cl_name=CLNAME)
        self.client.connect_to_server()
        self.client.start_session(0, 0, 0)

        # BP on empty return in a function
        bp = (CLNAME, 10)
        self.assertEqual(self.client.debug_run([bp]), bp)

        # BP on a non-empty return in a function
        bp = (CLNAME, 4)
        self.assertEqual(self.client.debug_run([bp]), bp)

        # done
        self.client.debug_run_finish()
