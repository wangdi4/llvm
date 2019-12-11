from testlib.debuggertestcase import DebuggerTestCase, expectedFailureGDB, expectedFailureCDB, skipNotGDBorCDB

# Test usage of setting value for some variable types
# (global, local, private, volatile, regular and pointers) in gdb
#
class TestVariableSetting(DebuggerTestCase):
    CLNAME = 'diffrent_declaration_types2.cl'

    @skipNotGDBorCDB
    def test_variables_setting(self):
    #
    # Test - some variable types (global, arguments, volatile, private)
        self.client.execute_debuggee(
            hostprog_name='ndrange_inout',
            cl_name=self.CLNAME)
        self.client.connect_to_server()
        self.client.start_session(0, 0, 0)

        # check variables values before setting them
        bp = (self.CLNAME, 16)
        self.assertEqual(self.client.debug_run([bp]), bp)

        # check the variables
        self.assertEqual(self.client.var_query_value('bb'), 'false')
        self.assertEqual(self.client.var_query_value('ii'), '1')
        self.assertEqual(self.client.var_query_value('pp'), '3.0')
        self.assertEqual(self.client.var_query_value('vv'), '4')
        self.assertEqual(self.client.var_query_value('globalInt'), '1')

        # set the variables
        self.client.var_set_value('bb', '"true"')
        self.client.var_set_value('ii', '2')
        self.client.var_set_value('pp', '4.0')
        self.client.var_set_value('vv', '5')
        self.client.var_set_value('globalInt', '2')

        # check that setting variables has taken effect
        self.assertEqual(self.client.var_query_value('bb'), 'true')
        self.assertEqual(self.client.var_query_value('ii'), '2')
        self.assertEqual(self.client.var_query_value('pp'), '4.0')
        self.assertEqual(self.client.var_query_value('vv'), '5')
        self.assertEqual(self.client.var_query_value('globalInt'), '2')

        # check that the set variables have taken effect and output of a simple increment is correct
        bp = (self.CLNAME, 22)
        self.assertEqual(self.client.debug_run([bp]), bp)

        # check the variables
        self.assertEqual(self.client.var_query_value('bb'), 'true')
        self.assertEqual(self.client.var_query_value('ii'), '3')
        self.assertEqual(self.client.var_query_value('pp'), '5.0')
        self.assertEqual(self.client.var_query_value('vv'), '6')
        self.assertEqual(self.client.var_query_value('globalInt'), '2')
        self.client.debug_run_finish()

    @skipNotGDBorCDB
    @expectedFailureGDB
    @expectedFailureCDB
    # Expected to fail due to ClearQuest defect CSSD100014001
    # (setting __local variables has no effect on program execution)
    def test_local_variable_setting(self):
    #
    # Test that setting variable in local AS has taken effect
        self.client.execute_debuggee(
            hostprog_name='ndrange_inout',
            cl_name=self.CLNAME)
        self.client.connect_to_server()
        self.client.start_session(0, 0, 0)

        # check the variable value before setting it
        bp = (self.CLNAME, 16)
        self.assertEqual(self.client.debug_run([bp]), bp)
        self.assertEqual(self.client.var_query_value('ll'), '2.0')

        # set the variable
        self.client.var_set_value('ll', '3.0')

        # check that setting variable has taken effect
        self.assertEqual(self.client.var_query_value('ll'), '3.0')

        # and output of a simple increment is correct
        bp = (self.CLNAME, 22)
        self.assertEqual(self.client.debug_run([bp]), bp)
        self.assertEqual(self.client.var_query_value('ll'), '4.0')
        self.client.debug_run_finish()
