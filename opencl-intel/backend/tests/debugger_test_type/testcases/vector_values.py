from testlib.debuggertestcase import DebuggerTestCase


class VectorValues(DebuggerTestCase):
    def test_vector_values1(self):
        CLNAME = 'vector_values1.cl'
        self.client.execute_debuggee(
            hostprog_name='ndrange_inout',
            cl_name=CLNAME)
        self.client.connect_to_server()
        self.client.start_session(0, 0, 0)

        bp = (CLNAME, 45)
        self.assertEqual(self.client.debug_run([bp]), bp)

        self.assertEqual(self.client.var_query_value('cc2'), '97,98');

        self.assertEqual(self.client.var_query_value('ss4'), '-30000,32000,-99,444');
        self.assertEqual(self.client.var_query_value('uss3'), '52222,300,9');

        self.assertEqual(self.client.var_query_value('ii3'), '120,240,-20');
        self.assertEqual(self.client.var_query_value('uii8'), '1,2,3,4,5,6,7,8');

        self.assertEqual(self.client.var_query_value('ll2'), '-9000000000,100');
        self.assertEqual(self.client.var_query_value('ull2'), '345999999999,42');

        # values chosen to be exactly representable in floats, so no approximate
        # matching is required
        self.assertEqual(self.client.var_query_value('ff4'), '4.5,6.0,3.25,-7.75')
        if self.use_cdb:
            self.assertEqual(self.client.var_query_value('dd8'),
                '1.875,-9e+09.0,1.875,1.875,1.875,1.875,1.875,1.875')
        else:
            self.assertEqual(self.client.var_query_value('dd8'),
                '1.875,-9000000000.0,1.875,1.875,1.875,1.875,1.875,1.875')

        if self.use_gdb or self.use_cdb:
            # Check each value to avoid 'incomplete sequence' messages in gdb output
            self.assertEqual(self.client.var_query_value('ucc16[0]'), '240')
            for i in range(1, 14):
                self.assertEqual(self.client.var_query_value('ucc16[' + str(i) + ']'), '250')
            self.assertEqual(self.client.var_query_value('ucc16[15]'), '240')
            if self.use_gdb: # Multidimensional arrays are currently not supported by CDB backend
                self.assertEqual(self.client.var_query_value('iiarr'),
                    '{4,6},{-9,-20},{40,60},{234,-456}')
                self.assertEqual(self.client.var_query_value('marr'),
                    '{{1,2,3,4},{5,6,7,8},{9,10,11,12}},'+
                    '{{21,22,23,24},{25,26,27,28},{29,30,31,32}}')
        else:
            self.assertEqual(self.client.var_query_value('ucc16'),
                '240,250,250,250,250,250,250,250,250,250,250,250,250,250,250,240');
            self.assertEqual(self.client.var_query_value('iiarr'),
                '[4,6|-9,-20|40,60|234,-456]')
            self.assertEqual(self.client.var_query_value('marr'),
                '[[[1|2|3|4][5|6|7|8][9|10|11|12]]'+
                '[[21|22|23|24][25|26|27|28][29|30|31|32]]]')

        self.client.debug_run_finish([bp])
