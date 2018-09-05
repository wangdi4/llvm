# == fpga_host_side_pipes.py - a launcher for debugger host-side pipes test ==//
#
# Copyright (C) 2017 Intel Corporation. All rights reserved.
#
# The information and source code contained herein is the exclusive property
# of Intel Corporation and may not be disclosed, examined or reproduced in
# whole or in part without explicit written authorization from the company.
#
# ==-----------------------------------------------------------------------== //
# The test checks that
# * debugger stops on read_pipe() call
# * debugger can print data which has been read from a pipe
# Note: the test isn't supported by Windows debugger simulator

from testlib.debuggertestcase import DebuggerTestCase, skipNotGDB

class FPGAHostSidePipes(DebuggerTestCase):
    CLNAME = 'fpga_host_side_pipes.cl'

    @skipNotGDB
    def test_breakpoints(self):
        self.client.execute_debuggee(
            hostprog_name='fpga_host_side_pipes',
            cl_name=self.CLNAME)
        self.client.connect_to_server()
        self.client.start_session(0, 0, 0)

        # Set a breakpoint on 'for' #6  and 'read_pipe' #9
        bps = [(self.CLNAME, 6), (self.CLNAME, 9)]
        self.assertEqual(self.client.debug_run(bps), bps[0])

        # Stop first time on read_pipe and check a value of 'val'
        self.assertEqual(self.client.debug_continue(), bps[1])
        self.assertEqual(self.client.var_query_value('val'), '0')

        # Stop second time on read_pipe and check a value of 'val'
        self.assertEqual(self.client.debug_continue(), bps[1])
        self.assertEqual(self.client.var_query_value('val'), '1')

        self.client.debug_run_finish()
