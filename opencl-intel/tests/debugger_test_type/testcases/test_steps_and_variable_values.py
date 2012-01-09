from testlib.debuggertestcase import DebuggerTestCase

class TestStepsAndVariableValues(DebuggerTestCase):
    CLNAME = 'nested_calls4.cl'
    INNER1_FUNCTION_ROW = 57
    INNER1_FUNCTION_BLOCK_ROW = 55
    INNER2_FUNCTION_ROW = 33
    INNER2_FUNCTION_BLOCK_ROW = 38
    INNER3_FUNCTION_ROW = 22
    INNER3_FUNCTION_BLOCK_ROW = 19
    FOO_FUNCTION_ROW = 6
    def test_in_nested_function_calls(self):
    #
    #  Test - test the variable values query after making diffrent type of step (in, over and out)
    #  TC-47, TC-48, TC-49
        self.client.execute_debuggee(
            hostprog_name='ndrange_inout',
            cl_name=self.CLNAME)
        self.client.connect_to_server()
        self.client.start_session(0, 0, 0)
        
        # check variables values in block and after block in function test3, after using step in
        bp = (self.CLNAME, self.INNER3_FUNCTION_BLOCK_ROW)
        self.assertEqual(self.client.debug_run([bp]), bp)
        bp = (self.CLNAME, self.INNER3_FUNCTION_BLOCK_ROW+1)
        self.assertEqual(self.client.debug_step_in(), bp)
        # in block
        self.assertEqual(self.client.var_query_value('i3'), '4')
        self.assertEqual(self.client.var_query_value('a'), '3')
        self.assertEqual(self.client.var_query_value('c3'), '2')
        self.assertEqual(self.client.var_query_value('r3'), '67')
        self.assertEqual(self.client.var_query_value('ai3'), '4')
        self.assertEqual(self.client.var_query_value('ac3'), '2')
        self.assertEqual(self.client.var_query_value('globalLong'), '1,22,333,4444')
        bp = (self.CLNAME, self.INNER3_FUNCTION_BLOCK_ROW+2)
        self.assertEqual(self.client.debug_step_in(), bp)        
        bp = (self.CLNAME, self.INNER3_FUNCTION_ROW)
        self.assertEqual(self.client.debug_step_in(), bp)
        # after block
        self.assertEqual(self.client.var_query_value('i3'), '4')
        self.assertEqual(self.client.var_query_value('a'), '3')
        self.assertEqual(self.client.var_query_value('c3'), '2')
        self.assertEqual(self.client.var_query_value('r3'), '67')
        self.assertEqual(self.client.var_query_value('globalLong'), '1,22,333,4444')
        
        # check variables values in block and before block in function test2, after using step out
        bp = (self.CLNAME, self.INNER2_FUNCTION_ROW)
        self.assertEqual(self.client.debug_step_out(), bp)
        # before block
        self.assertEqual(self.client.var_query_value('i2'), '3')
        self.assertEqual(self.client.var_query_value('a'), '2')
        self.assertEqual(self.client.var_query_value('c2'), '2')
        self.assertEqual(self.client.var_query_value('r2'), '67')
        self.assertEqual(self.client.var_query_value('globalLong'), '1,22,333,4444')
        bp = (self.CLNAME, self.FOO_FUNCTION_ROW)
        self.assertEqual(self.client.debug_run([bp]), bp)
        bp = (self.CLNAME, self.INNER2_FUNCTION_BLOCK_ROW)
        self.assertEqual(self.client.debug_step_out(), bp)
        # in block
        self.assertEqual(self.client.var_query_value('i2'), '3')
        self.assertEqual(self.client.var_query_value('a'), '2')
        self.assertEqual(self.client.var_query_value('c2'), '2')
        self.assertEqual(self.client.var_query_value('r2'), '67')
        self.assertEqual(self.client.var_query_value('ai2'), '3')
        self.assertEqual(self.client.var_query_value('ac2'), '2')
        self.assertEqual(self.client.var_query_value('globalLong'), '1,22,333,4444')

        # check variables values in block and after block in function test1, after using step over        
        bp = (self.CLNAME, self.INNER1_FUNCTION_BLOCK_ROW )
        self.assertEqual(self.client.debug_run([bp]), bp)
        bp = (self.CLNAME, self.INNER1_FUNCTION_BLOCK_ROW + 1)
        self.assertEqual(self.client.debug_step_over(), bp)
        # in block
        self.assertEqual(self.client.var_query_value('i1'), '2')
        self.assertEqual(self.client.var_query_value('a'), '1')
        self.assertEqual(self.client.var_query_value('c1'), '2')
        self.assertEqual(self.client.var_query_value('r1'), '67')
        self.assertEqual(self.client.var_query_value('ac1'), '2')
        self.assertEqual(self.client.var_query_value('ai1'), '2')
        self.assertEqual(self.client.var_query_value('globalLong'), '1,22,333,4444')
        bp = (self.CLNAME, self.INNER1_FUNCTION_ROW )
        self.assertEqual(self.client.debug_run([bp]), bp)
        bp = (self.CLNAME, self.INNER1_FUNCTION_ROW + 1)
        self.assertEqual(self.client.debug_step_over(), bp)
        # after block
        self.assertEqual(self.client.var_query_value('i1'), '2')
        self.assertEqual(self.client.var_query_value('a'), '1')
        self.assertEqual(self.client.var_query_value('c1'), '2')
        self.assertEqual(self.client.var_query_value('r1'), '67')
        self.assertEqual(self.client.var_query_value('globalLong'), '1,22,333,4444')
        self.client.debug_run_finish()
