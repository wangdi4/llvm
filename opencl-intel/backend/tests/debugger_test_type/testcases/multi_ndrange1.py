from testlib.debuggertestcase import DebuggerTestCase, expectedFailureCDB

# Test debugging into more than one NDRange in a single session
#
class MultiNDRange1(DebuggerTestCase):
    CLNAME1 = 'nested_calls1.cl'
    CLNAME2 = 'simple_buffer_copy.cl'

    @expectedFailureCDB
    def test_multi_ndrange1(self):
        # Well, this is a hack: CLNAME1 goes into the cl_name argument, which
        # the client converts automatically to absolute, but CLNAME2 has to
        # go through extra_args and thus the absolute path conversion is done
        # explicitly
        #
        self.client.execute_debuggee(
            hostprog_name='multi_ndrange',
            cl_name=self.CLNAME1,
            extra_args=[self.client.cl_abs_filename(self.CLNAME2)])
        self.client.connect_to_server()
        self.client.start_session(1, 0, 0)

        # CLNAME1 runs first, then runs CLNAME2
        #
        bps = [(self.CLNAME1, 22), (self.CLNAME2, 4)]

        # First we stop in CLNAME1
        self.assertEqual(self.client.debug_run(bps), bps[0])

        # do some stepping and querying to make sure we got to the right place
        self.client.debug_step_in()
        self.client.get_stack_trace()
        self.assertEqual(self.client.stack_query_func_name(0), 'test1')
        self.assertEqual(self.client.stack_query_func_name(1), 'main_kernel')

        # Then we stop in CLNAME2
        self.assertEqual(self.client.debug_run(bps), bps[1])
        self.assertEqual(self.client.var_query_value('gid'), '1');

        # done
        self.client.debug_run_finish()
