from testlib.debuggertestcase import DebuggerTestCase, expectedFailureCDB


# Test a usage of query value for all variable types TC-
#
class TestVariableTypesInSeveralKernels(DebuggerTestCase):
    CLNAME = 'several_kernels_only_global_variables.cl'
    CLNAME2 = 'several_kernels_no_global_variables.cl'
    CLNAME3 = 'several_kernels_only_argument_variables.cl'
    # these "defines" are for test_global_variable_several_kernels
    INNER_KERNEL_ROW = 6
    # these "defines" are for test_no_global_variable_several_kernels
    INNER_KERNEL_ROW2 = 6
    # these "defines" are for test_only_arguments_variables_several_kernels
    INNER_KERNEL_ROW3 = 5

    @expectedFailureCDB
    def test_global_variables_several_kernels(self):
    #
    # Test usage of global varables by testing it values in several of nested kernels
    # TC-44
        self.client.execute_debuggee(
            hostprog_name='ndrange_inout',
            cl_name=self.CLNAME)
        self.client.connect_to_server()
        self.client.start_session(0, 0, 0)
        # check variables values in if block
        bp = (self.CLNAME, self.INNER_KERNEL_ROW)
        self.assertEqual(self.client.debug_run([bp]), bp)
        self.assertEqual(self.client.var_query_value('c'), '6')
        self.assertEqual(self.client.var_query_value('b'), '5')
        self.client.get_stack_trace()
        self.assertEqual(self.client.stack_query_func_name(0), 'fourth_kernel')
        self.assertEqual(self.client.stack_query_func_name(1), 'third_kernel')
        self.assertEqual(self.client.stack_query_func_name(2), 'second_kernel')
        self.assertEqual(self.client.stack_query_func_name(3), 'main_kernel')
        self.client.debug_run_finish()

    @expectedFailureCDB
    def test_no_global_variables_several_kernels(self):
    #
    # Test usage of non-global varables by testing it values in several of nested kernels
    # TC-45
        self.client.execute_debuggee(
            hostprog_name='ndrange_inout',
            cl_name=self.CLNAME2)
        self.client.connect_to_server()
        self.client.start_session(0, 0, 0)
        # check variables values in if block
        bp = (self.CLNAME2, self.INNER_KERNEL_ROW2)
        self.assertEqual(self.client.debug_run([bp]), bp)
        self.assertEqual(self.client.var_query_value('c'), '8')
        self.assertEqual(self.client.var_query_value('i1'), '4')
        self.assertEqual(self.client.var_query_value('i2'), '2')
        self.client.get_stack_trace()
        self.assertEqual(self.client.stack_query_func_name(0), 'fourth_kernel')
        self.assertEqual(self.client.stack_query_func_name(1), 'third_kernel')
        self.assertEqual(self.client.stack_query_func_name(2), 'second_kernel')
        self.assertEqual(self.client.stack_query_func_name(3), 'main_kernel')
        self.client.debug_run_finish()

    @expectedFailureCDB
    def test_only_arguments_variables_several_kernels(self):
    #
    # Test usage of function arguments by testing it values in several of nested kernels
    # TC-46
        self.client.execute_debuggee(
            hostprog_name='ndrange_inout',
            cl_name=self.CLNAME3)
        self.client.connect_to_server()
        self.client.start_session(0, 0, 0)
        # check variables values in if block
        bp = (self.CLNAME3, self.INNER_KERNEL_ROW3)
        self.assertEqual(self.client.debug_run([bp]), bp)
        self.assertEqual(self.client.var_query_value('a'), '4')
        self.assertEqual(self.client.var_query_value('b'), '5')
        self.client.get_stack_trace()
        self.assertEqual(self.client.stack_query_func_name(0), 'fourth_kernel')
        self.assertEqual(self.client.stack_query_func_name(1), 'third_kernel')
        self.assertEqual(self.client.stack_query_func_name(2), 'second_kernel')
        self.assertEqual(self.client.stack_query_func_name(3), 'main_kernel')
        self.client.debug_run_finish()
