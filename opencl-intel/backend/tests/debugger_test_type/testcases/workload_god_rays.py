from testlib.debuggertestcase import DebuggerTestCase


class WorkloadGodRays(DebuggerTestCase):
    CLNAME = 'intel_god_rays.cl'

    def test_simple_bp(self):
        # the god rays kernel is relatively large, so this is a sanity check
        # just to see it compiled at all
        self.client.execute_debuggee(
            hostprog_name='1d_float4_with_size',
            cl_name=self.CLNAME,
            extra_args=['128', '256', '32', '1'])
        self.client.connect_to_server()
        self.client.start_session(0, 0, 0)

        bps = [(self.CLNAME, 642), (self.CLNAME, 644)]
        self.assertEqual(self.client.debug_run(bps), bps[0])
        self.assertEqual(self.client.debug_run(bps), bps[1])

        # default size
        self.assertEqual(self.client.var_query_value('imgSize'), '128,256')
        # gid 0
        self.assertEqual(self.client.var_query_value('in_RayNum'), '0')

        self.client.debug_run_finish(bps)

    def test_evaluate_ray(self):
        # do some actual probing inside evaluateRay
        self.client.execute_debuggee(
            hostprog_name='1d_float4_with_size',
            cl_name=self.CLNAME,
            extra_args=['128', '256', '32', '1'])
        self.client.connect_to_server()
        self.client.start_session(0, 0, 0)

        # go to first iteration of the loop
        bp = (self.CLNAME, 37)
        self.assertEqual(self.client.debug_run([bp]), bp)

        self.assertEqual(self.client.var_query_value('imgSize'), '128,256')
        self.assertEqual(self.client.var_query_value('in_RayNum'), '0')
        self.assertEqual(self.client.var_query_value('blend'), '1')
        self.assertEqual(self.client.var_query_value('god_rays_bunch_size'), '15')
        self.assertEqual(self.client.var_query_value('x_last'), '127')
        self.assertEqual(self.client.var_query_value('y_last'), '255')

        # Look at the stack trace
        self.client.get_stack_trace()
        self.assertEqual(self.client.stack_query_func_name(0), 'evaluateRay')
        self.assertEqual(self.client.stack_query_call_location(0),
                (self.CLNAME, 644))
        self.assertEqual(self.client.stack_query_func_name(1), 'main_kernel')

        # go to after the call
        bp = (self.CLNAME, 645)
        self.assertEqual(self.client.debug_run([bp]), bp)
        self.client.get_stack_trace()
        self.assertEqual(self.client.stack_query_func_name(0), 'main_kernel')

        self.client.debug_run_finish([])
