from testlib.debuggertestcase import DebuggerTestCase


class StructKernelArg(DebuggerTestCase):
    def test_arg_values(self):
        CLNAME = 'struct_kernel_arg1.cl'
        self.client.execute_debuggee(
            hostprog_name='struct_kernel_arg',
            cl_name=CLNAME)
        self.client.connect_to_server()
        self.client.start_session(2, 0, 0)

        bp = (CLNAME, 40)
        self.assertEqual(self.client.debug_run([bp]), bp)

        # the default data size
        self.assertEqual(self.client.var_query_value('kf1'), '100.0')
        self.assertEqual(self.client.var_query_value('kf2'), '-9999.25')
        self.assertEqual(self.client.var_query_value('kposition'), '0.25,0.75,1.25,2.25')
        self.assertEqual(self.client.var_query_value('ksize'), '1024')

        self.assertEqual(self.client.var_query_value('sizeof_kernelarg'), '92')
        self.assertEqual(self.client.var_query_size('kcopy'), 92)

        self.assertEqual(
            float(self.client.var_query_value('ksum_rotation')),
            float(sum(range(16))))

        if self.use_gdb or self.use_cdb:
            # GDB's printing of user defined types in OCL is a little broken, but individual
            # members can be accessed OK.
            self.assertEqual(self.client.var_query_value('kcopy.f1'), '100.0')
            self.assertEqual(self.client.var_query_value('kcopy.f2'), '-9999.25')
            self.assertEqual(self.client.var_query_value('kcopy.position'), '0.25,0.75,1.25,2.25')
            self.assertEqual(self.client.var_query_value('kcopy.size'), '1024')
            self.assertEqual(self.client.var_query_value('kcopy.rotation'), '0.0,1.0,2.0,3.0,4.0,5.0,6.0,7.0,8.0,9.0,10.0,11.0,12.0,13.0,14.0,15.0')
        else:
            self.assertEqual(self.client.var_query_value('kcopy'),
                '{100.0|-9999.25|0.25,0.75,1.25,2.25|1024|' +
                '[0.0|1.0|2.0|3.0|4.0|5.0|6.0|7.0|8.0|9.0|10.0|11.0|12.0|13.0|14.0|15.0]}')

        self.client.debug_run_finish([bp])
