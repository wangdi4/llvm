# ===-- fpga_channels_by_value.py - a launcher for debugger test for FPGA channels -===//
#
# Copyright (C) 2018 Intel Corporation. All rights reserved.
#
# The information and source code contained herein is the exclusive property
# of Intel Corporation and may not be disclosed, examined or reproduced in
# whole or in part without explicit written authorization from the company.
#
# ===---------------------------------------------------------------------=== //
# The test check that
# * compilation of a kernel with channels and debug info is successful,
# * debugger stops inside read_channel_func, write_channel_func
# Note: the test isn't supported by Windows debugger simulator

from testlib.debuggertestcase import DebuggerTestCase, skipNotGDB

class FPGAChannels(DebuggerTestCase):
    CLNAME = 'fpga_channels_by_value.cl'

    @skipNotGDB
    def test_breakpoints(self):
        self.client.execute_debuggee(
            hostprog_name='fpga_channels',
            cl_name=self.CLNAME)
        self.client.connect_to_server()
        self.client.start_session(0, 0, 0)

        # set a breakpoint on channel_writer
        bp = (self.CLNAME, 17)
        self.assertEqual(self.client.debug_run([bp]), bp)

        # step into channel_writer_func
        self.assertEqual(self.client.debug_step_in(), (self.CLNAME, 7))

        # set a breakpoint on channel_reader
        bp = (self.CLNAME, 22)
        self.assertEqual(self.client.debug_run([bp]), bp)

        # step into channel_reader_func
        self.assertEqual(self.client.debug_step_in(), (self.CLNAME, 12))

        self.client.debug_run_finish()

