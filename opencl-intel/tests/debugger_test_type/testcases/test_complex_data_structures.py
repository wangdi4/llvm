from testlib.debuggertestcase import DebuggerTestCase


# Test a usage of query value for all variable types TC-33,34,35,36,37,38,39,40,4142,43
#
class TestComplexDataStructures(DebuggerTestCase):
    CLNAME = 'complex_data_stractures.cl'
    LAST_ROW = 36
    VARAIBLE_AMOUNT = 9
    def test_complex_data_structures(self):
    #
    # Test - tests values of diffrent data structures such as struct, pointers, multi array and vectors
    # TC-50
        self.client.execute_debuggee(
            hostprog_name='ndrange_inout',
            cl_name=self.CLNAME)
        self.client.connect_to_server()
        self.client.start_session(0, 0, 0)
        # check variables values after all assignment statements
        bp = (self.CLNAME, self.LAST_ROW)
        self.assertEqual(self.client.debug_run([bp]), bp)
        self.assertEqual(len(self.client.var_list()), self.VARAIBLE_AMOUNT)
        self.assertEqual(self.client.var_query_value('z'), '{3.0|-1.0}')
        self.assertEqual(self.client.var_query_value('pZ'), '39')
        self.assertEqual(self.client.var_query_value('ppZ'), '49')
        self.assertEqual(self.client.var_query_value('ulongArr'), '[0,1,2,3,4,5,6,7|8,9,10,11,12,13,14,15]')
        self.assertEqual(self.client.var_query_value('complexArr'), '[{0.0|0.0}|{1.0|-1.0}]')
        self.assertEqual(self.client.var_query_value('dim4Arr'),'[[[1|2|3|4][5|6|7|8][9|10|11|12]][[21|22|23|24][25|26|27|28][29|30|31|32]]]')
        self.client.debug_run_finish()
