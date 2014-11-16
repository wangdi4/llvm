from testlib.debuggertestcase import DebuggerTestCase


# Test a simple usage of call stack querying in a function call
#
class Callstack1(DebuggerTestCase):
    def test_simple_callstack(self):
        CLNAME = 'simple_func_calls.cl'
        self.client.execute_debuggee(
            hostprog_name='ndrange_inout',
            cl_name=CLNAME)
        self.client.connect_to_server()
        self.client.start_session(0, 0, 0)

        # get to the third call to foo()
        bp = (CLNAME, 16)
        self.assertEqual(self.client.debug_run([bp]), bp)

        # get into foo(), to its last line
        bp = (CLNAME, 6)
        self.assertEqual(self.client.debug_run([bp]), bp)

        # Check the values of variables in foo
        self.assertEqual(self.client.var_query_value('c'), '12')
        self.assertEqual(self.client.var_query_value('d'), '15')

        # Look at the stack trace
        self.client.get_stack_trace()
        self.assertEqual(self.client.stack_query_func_name(0), 'foo')
        self.assertEqual(self.client.stack_query_call_location(0),
                (CLNAME, 16))
        self.assertEqual(self.client.stack_query_func_name(1), 'main_kernel')

        # Now check the values of variables one stack frame up - in main_kernel
        self.assertEqual(self.client.var_query_value('p', 1), '12')

        # done
        self.client.debug_run_finish()

    def test_deep_nesting(self):
        CLNAME = 'deep_call_nesting.cl'
        self.client.execute_debuggee(
            hostprog_name='ndrange_inout',
            cl_name=CLNAME)
        self.client.connect_to_server()
        self.client.start_session(0, 0, 0)

        # put a breakpoint inside the leaf function
        bp = (CLNAME, 5)
        self.assertEqual(self.client.debug_run([bp]), bp)

        self.client.get_stack_trace()
        for i in range(0, 8):   # stack frames 0 .. 7
            self.assertEqual(
                self.client.stack_query_func_name(i),
                'foo' + str(9 - i))
            self.assertEqual(
                self.client.stack_query_call_location(i),
                (CLNAME, 11 + i * 7))
        self.assertEqual(self.client.stack_query_func_name(8), 'foo1')
        self.assertEqual(self.client.stack_query_call_location(8), (CLNAME, 68))

        # done
        self.client.debug_run_finish()
