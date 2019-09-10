from testlib.debuggertestcase import DebuggerTestCase, expectedFailureCDB


# Reproducer for CQ issue CSSD100007031
#
class CSSD100007031(DebuggerTestCase):
    @expectedFailureCDB
    def test_CSSD100007031(self):
        CLNAME = 'cssd100007031.cl'

        # use printf_tester to eat stdout but ask it to skip verification
        self.client.execute_debuggee(
            hostprog_name='printf_tester',
            cl_name=CLNAME,
            extra_args=['true']) # do not verify
        self.client.connect_to_server()
        self.client.start_session(0, 0, 0)

        # BP in main kernel
        bp = (CLNAME, 64)
        self.assertEqual(self.client.debug_run([bp]), bp)
        self.assertEqual(self.client.var_query_value('ii3'), '1,2,3')

        # BP in myfooMMM..., mybarBBB..., and blehLLL...
        bps = [(CLNAME, lineno) for lineno in (32, 23, 8)]

        # first we stop in myfooMMM...
        self.assertEqual(self.client.debug_run(bps), bps[0])
        # then in mybarBBB...
        self.assertEqual(self.client.debug_run(bps), bps[1])
        # then in blehLLL...
        self.assertEqual(self.client.debug_run(bps), bps[2])

        # then again in blehLLL since... it's called from main_kernel too
        self.assertEqual(self.client.debug_run(bps), bps[2])

        # enough...

        # done
        self.client.debug_run_finish()
