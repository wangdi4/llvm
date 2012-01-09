from testlib.debuggertestcase import DebuggerTestCase


# Test providing a custom port (not the default) to the server
#
class Customport(DebuggerTestCase):
    CLNAME = 'simple_func_calls.cl'

    def test_custom_port(self):
        self.client.set_server_port(44992)
        self.client.execute_debuggee(
            hostprog_name='ndrange_inout',
            cl_name=self.CLNAME)
        self.client.connect_to_server()
        self.client.start_session(1, 0, 0)

        # get to the third call to foo()
        bp = (self.CLNAME, 16)
        self.assertEqual(self.client.debug_run([bp]), bp)
        
        # done
        self.client.debug_run_finish()

