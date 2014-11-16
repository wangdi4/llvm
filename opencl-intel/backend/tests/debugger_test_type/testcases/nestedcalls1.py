from testlib.debuggertestcase import DebuggerTestCase


# Test a simple usage of call stack querying in a function call
#
class NestedCalls1(DebuggerTestCase):
    CLNAME = 'nested_calls1.cl'

    def test_nested_calls1(self):
        self.client.execute_debuggee(
            hostprog_name='ndrange_inout',
            cl_name=self.CLNAME)
        self.client.connect_to_server()
        self.client.start_session(0, 0, 0)

        # get to the third innermost call, inside test3
        bp = (self.CLNAME, 16)
        self.assertEqual(self.client.debug_run([bp]), bp)

        # check the value of the i3 var
        self.assertEqual(self.client.var_query_value('i3'), '3')

        # look at the stack trace, check values of vars in upper frames
        self.client.get_stack_trace()
        self.assertEqual(self.client.stack_query_func_name(0), 'test3')
        self.assertEqual(self.client.stack_query_func_name(1), 'test2')
        self.assertEqual(self.client.stack_query_func_name(2), 'test1')
        self.assertEqual(self.client.stack_query_func_name(3), 'main_kernel')

        self.assertEqual(self.client.var_query_value('i2', 1), '2')
        self.assertEqual(self.client.var_query_value('i1', 2), '1')

        # done
        self.client.debug_run_finish()
