# ===-- fpga_channels.py - a launcher for debugger test for FPGA channels -===//
#
# Copyright (C) 2017 Intel Corporation. All rights reserved.
#
# The information and source code contained herein is the exclusive property
# of Intel Corporation and may not be disclosed, examined or reproduced in
# whole or in part without explicit written authorization from the company.
#
# ===---------------------------------------------------------------------=== //
# The test check that
# * compilation of a kernel with channels and debug info is successful,
# * debugger stops on read_channel() / write_channel() calls

from testlib.debuggertestcase import DebuggerTestCase

class FPGAChannels(DebuggerTestCase):
    CLNAME = 'fpga_channels.cl'
    def test_breakpoints(self):
        self.client.execute_debuggee(
            hostprog_name='fpga_channels',
            cl_name=self.CLNAME,
            options={'build_opts':'-cl-std=CL2.0'})
        self.client.connect_to_server()
        self.client.start_session(0, 0, 0)

        # Set a breakpoint on write_channel_intel()
        bp = (self.CLNAME, 7)
        self.assertEqual(self.client.debug_run([bp]), bp)

        # Set a breakpoint on read_channel_intel()
        bp = (self.CLNAME, 12)
        self.assertEqual(self.client.debug_run([bp]), bp)

        self.client.debug_run_finish()
