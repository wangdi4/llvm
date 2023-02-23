from testlib.debuggertestcase import DebuggerTestCase


# Reproducer for OpenCL debugger crashes on struct types with self pointer
# fields circular references in struct type
class 100019603(DebuggerTestCase):
    def test_100019603(self):
        CLNAME = '100019603.cl'
        self.client.execute_debuggee(
            hostprog_name='ndrange_inout',
            cl_name=CLNAME)
        self.client.connect_to_server()
        self.client.start_session(0, 0, 0)
        # BP on fake assignment in a function
        bp = (CLNAME, 29)
        self.assertEqual(self.client.debug_run([bp]), bp)
        # done
        self.client.debug_run_finish()
