from testlib.debuggertestcase import DebuggerTestCase, expectedFailureGDB, expectedFailureCDB


# Test a simple usage of breakpoints for TC-4-9
#
class TestBreakpoints(DebuggerTestCase):
    CLNAME = 'many_function_calls.cl'
    CLNAME2 = 'printf_tester1.cl'
    MAIN_ROW = 2009
    LOOP_ROW = 2007
    UNNESTED_FIRST_FUNC_ROW = 3
    UNNESTED_JUMP = 10
    NESTED_FIRST_FUNC_ROW = 1993
    NESTED_JUMP = 10
    UNREACHED_ROW = 2115
    RESULT_ROW = 2109

    def test_no_breakpoints(self):
    #
    #  Test - test the ability to run a kernel without setting any breakpoints
    #  TC-3
        self.client.execute_debuggee(
            hostprog_name='printf_tester',
            cl_name=self.CLNAME2)
        self.client.connect_to_server()
        self.client.start_session(0, 0, 0)

        self.client.debug_run_finish()

    @expectedFailureCDB
    def test_breakpoints_in_main_and_unreachable(self):
    #
    # Test a simple usage of breakpoints only set in main
    # TC-4,TC-5,TC-6
        self.client.execute_debuggee(
            hostprog_name='ndrange_inout',
            cl_name=self.CLNAME)
        self.client.connect_to_server()
        self.client.start_session(0, 0, 0)
        bps =[]
        for i in range(100):
            bps.append((self.CLNAME, self.MAIN_ROW + i))
        for i in range(100):
            self.assertEqual(self.client.debug_run(bps), bps[i])
        # done with tested break points, check var value
        bp = (self.CLNAME, self.RESULT_ROW)
        self.assertEqual(self.client.debug_run([bp]), bp)
        self.assertEqual(self.client.var_query_value('m'), '100')
        # now check for unreachable breakpoints
        bps2 = []
        for i in range(100):
            bps2.append((self.CLNAME, self.UNREACHED_ROW+i))
        self.client.debug_run_finish(bps2)

    @expectedFailureCDB
    def test_breakpoints_in_loop(self):
    #
    # Test a simple usage of breakpoints only set in a loop
    # TC-9
        self.client.execute_debuggee(
            hostprog_name='ndrange_inout',
            cl_name=self.CLNAME)
        self.client.connect_to_server()
        self.client.start_session(0, 0, 0)
        bps=[]
        bps.append((self.CLNAME, self.LOOP_ROW))
        bps.append((self.CLNAME, self.RESULT_ROW))
        for i in range(3):
            self.assertEqual(self.client.debug_run(bps), bps[0])
        bps.remove((self.CLNAME, self.LOOP_ROW))
        # done with tested break points, chech var value
        self.assertEqual(self.client.debug_run(bps), bps[0])
        self.assertEqual(self.client.var_query_value('m'), '100')
        self.client.debug_run_finish()

    @expectedFailureCDB
    def test_breakpoints_in_second_call_level(self):
    #
    # Test a simple usage of breakpoints only set in a second function call level
    # TC-8
        self.client.execute_debuggee(
            hostprog_name='ndrange_inout',
            cl_name=self.CLNAME)
        self.client.connect_to_server()
        self.client.start_session(0, 0, 0)
        bps = []
        for i in range(100):
            # set breakpoints at 100 unnested functions (functions name barn) one by one
            # first function is at UNNESTED_FIRST_FUNC_ROW and next function is UNNESTED_JUMP rows after that
            bps.append((self.CLNAME, self.UNNESTED_FIRST_FUNC_ROW + i*self.UNNESTED_JUMP))
        for i in range(100):
            self.assertEqual(self.client.debug_run(bps), bps[i])
        # done with tested break points, chech var value
        bp = (self.CLNAME, self.RESULT_ROW)
        self.assertEqual(self.client.debug_run([bp]), bp)
        self.assertEqual(self.client.var_query_value('m'), '100')
        self.client.debug_run_finish()

    @expectedFailureCDB
    def test_breakpoints_in_diffrent_call_level(self):
    #
    # Test a simple usage of breakpoints set in diffrent function call level
    # TC-7
        self.client.execute_debuggee(
            hostprog_name='ndrange_inout',
            cl_name=self.CLNAME)
        self.client.connect_to_server()
        self.client.start_session(0, 0, 0)
        bps = []
        for i in range(100):
		    # set breakpoints at 100 nested functions (functions name foon) one by one
            # first function is at NESTED_FIRST_FUNC_ROW and next function is NESTED_JUMP rows after that
            bps.append((self.CLNAME, self.NESTED_FIRST_FUNC_ROW - i*self.NESTED_JUMP))
        for i in range(100):
            self.assertEqual(self.client.debug_run(bps), bps[i])
        # done with tested break points, chech var value
        bp = (self.CLNAME, self.RESULT_ROW)
        self.assertEqual(self.client.debug_run([bp]), bp)
        self.assertEqual(self.client.var_query_value('m'), '100')
        self.client.debug_run_finish()
