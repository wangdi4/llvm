from testlib.debuggertestcase import DebuggerTestCase, expectedFailureCDB


# Test a usage of query value for all variable types TC-33,34,35,36,37,38,39,40,4142,43
#
class TestComplexDataStructures(DebuggerTestCase):
    CLNAME = 'complex_data_stractures.cl'
    LAST_ROW = 36
    VARAIBLE_AMOUNT = 9

    @expectedFailureCDB
    def test_complex_data_structures(self):
    #
    # Test - tests values of different data structures such as struct, pointers, multi array and vectors
    # TC-50
        self.client.execute_debuggee(
            hostprog_name='ndrange_inout',
            cl_name=self.CLNAME)
        self.client.connect_to_server()
        self.client.start_session(0, 0, 0)

        # break after all assignment statements
        bp = (self.CLNAME, self.LAST_ROW)
        self.assertEqual(self.client.debug_run([bp]), bp)

        # check the number of local variables
        if self.use_gdb or self.use_cdb:
            # In native (gdb) debugging, 3 extra implicit locals exist for the GID
            self.assertEqual(len(self.client.var_list()), self.VARAIBLE_AMOUNT + 3)
        else:
            self.assertEqual(len(self.client.var_list()), self.VARAIBLE_AMOUNT)

        # check variables values
        self.assertEqual(self.client.var_query_value('pZ'), '39')
        self.assertEqual(self.client.var_query_value('ppZ'), '49')

        # check struct values including nested structs. GDB and simulator clients
        # will print arrays and vectors and user-defined types slightly differently.
        if self.use_gdb:
            self.assertEqual(self.client.var_query_value('ulongArr'), '{0,1,2,3,4,5,6,7},{8,9,10,11,12,13,14,15}')
            self.assertEqual(self.client.var_query_value('complexArr'), '{r=0,i=0},{r=1,i=-1}')
            self.assertEqual(self.client.var_query_value('dim4Arr'),'{{1,2,3,4},{5,6,7,8},{9,10,11,12}},{{21,22,23,24},{25,26,27,28},{29,30,31,32}}')
        elif self.use_cdb: # Multidimensional arrays are currently not supported by the cdb backend
            self.assertEqual(self.client.var_query_type('ulongArr'), 'unsigned __int64 [2][8]')
            self.assertEqual(self.client.var_query_type('complexArr'), 'Complex [2]')
            self.assertEqual(self.client.var_query_type('dim4Arr'), 'int [2][3][4]')
        else:
            self.assertEqual(self.client.var_query_value('ulongArr'), '[0,1,2,3,4,5,6,7|8,9,10,11,12,13,14,15]')
            self.assertEqual(self.client.var_query_value('complexArr'), '[{0.0|0.0}|{1.0|-1.0}]')
            self.assertEqual(self.client.var_query_value('dim4Arr'),'[[[1|2|3|4][5|6|7|8][9|10|11|12]][[21|22|23|24][25|26|27|28][29|30|31|32]]]')

        self.client.debug_run_finish()
