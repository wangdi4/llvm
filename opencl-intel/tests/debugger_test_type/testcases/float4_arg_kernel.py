from testlib.debuggertestcase import DebuggerTestCase


class Float4ArgKernel(DebuggerTestCase):
    def test_values(self):
        CLNAME = '1d_float4_to_float4.cl'
        self.client.execute_debuggee(
            hostprog_name='1d_float4_with_size',
            cl_name=CLNAME,
            extra_args=['256', '2048', '64', '16'])
        self.client.connect_to_server()
        self.client.start_session(30, 0, 0)
        
        bp = (CLNAME, 19)
        # longer timeout needed here because the loop runs 256*2048 times.
        self.assertEqual(self.client.debug_run([bp], timeout=20), bp)
        
        self.assertEqual(self.client.var_query_value('imgSize'), '256,2048')
        self.assertEqual(self.client.var_query_value('fl8'), '8.0,8.0,8.0,8.0')
        self.assertEqual(self.client.var_query_value('fl9'), '9.0,9.0,9.0,9.0')
        self.assertEqual(int(self.client.var_query_value('i')), 256*2048)
        
        self.client.debug_run_finish([bp])


