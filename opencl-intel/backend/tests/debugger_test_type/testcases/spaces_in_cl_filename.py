from testlib.debuggertestcase import DebuggerTestCase


# Test running a CL file with spaces in its name
#
class SpacesInCLName(DebuggerTestCase):
    CLNAME = 'simple_buffer_copy spaces in name.cl'

    def test_spaces_in_name(self):
        self.client.execute_debuggee(
            hostprog_name='ndrange_inout',
            cl_name=self.CLNAME)
        self.client.connect_to_server()
        self.client.start_session(1, 0, 0)

        bp = (self.CLNAME, 4)
        self.assertEqual(self.client.debug_run([bp]), bp)

        # done
        self.client.debug_run_finish()
