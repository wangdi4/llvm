from testlib.debuggertestcase import DebuggerTestCase


class BuiltinsTester(DebuggerTestCase):
    def _vec2floats(self, vec_str):
        """ Given a string representing an OpenCL vector of floats, return
            a list of float objects.
        """
        return [float(f) for f in vec_str.split(',')]

    def test_use_builtins_math(self):
        CLNAME = 'builtins_math.cl'
        import math
        self.client.execute_debuggee(
            hostprog_name='ndrange_inout',
            cl_name=CLNAME)
        self.client.connect_to_server()
        self.client.start_session(0, 0, 0)

        bps = [(CLNAME, 15)]
        self.assertEqual(self.client.debug_run(bps), bps[0])

        fi = 8.32

        # check the scalar result of exp()
        self.assertAlmostEqual(
            float(self.client.var_query_value('fo')),
            math.exp(fi),
            2)

        # check the vectors
        floats_ff4 = self._vec2floats(self.client.var_query_value('ff4'))
        for i in range(4):
            self.assertAlmostEqual(floats_ff4[i], fi + i, 1)

        floats_expff4 = self._vec2floats(self.client.var_query_value('expff4'))
        for i in range(4):
            self.assertAlmostEqual(floats_expff4[i], math.exp(fi + i), 0)

        self.client.debug_run_finish(bps)

    def test_use_builtins_atomics(self):
        CLNAME = 'builtins_atomics.cl'
        self.client.execute_debuggee(
            hostprog_name='ndrange_inout',
            cl_name=CLNAME)
        self.client.connect_to_server()
        self.client.start_session(0, 0, 0)

        bps = [(CLNAME, 17)]
        self.assertEqual(self.client.debug_run(bps), bps[0])

        self.assertEqual(self.client.var_query_value('val1'), '517')
        self.assertEqual(self.client.var_query_value('val2'), '587')

        self.client.debug_run_finish(bps)
