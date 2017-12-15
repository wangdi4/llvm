# ===-- fpga_autorun.py - a launcher for debugger test for FPGA autoruns --===//
#
# Copyright (C) 2017 Intel Corporation. All rights reserved.
#
# The information and source code contained herein is the exclusive property
# of Intel Corporation and may not be disclosed, examined or reproduced in
# whole or in part without explicit written authorization from the company.
#
# ===---------------------------------------------------------------------=== //
# The test check that
# * compiltion of a programm with autorun kernels and debug info is successful,

from testlib.debuggertestcase import DebuggerTestCase

class FPGAAutorun(DebuggerTestCase):
    CLNAME = 'fpga_autorun.cl'
    def test_breakpoints(self):
        self.client.execute_debuggee(
            hostprog_name='fpga_autorun',
            cl_name=self.CLNAME,
            options={'build_opts':'"-cl-std=CL2.0 -DN=5"'})
        self.client.connect_to_server()
        self.client.start_session(0, 0, 0)

        # TODO: write a meaningful scenario for GDB
        # Compilation of 'fpga_autorun.cl' with debug info fails due to a
        # known issue in PipeSupportPass. Need to complete this test by
        # meaningful debugging scenario when the bug will be fixed.

        # This test checks only that compilation with debug info works fine.

        # Set a dummy breakpoint
        bp = (self.CLNAME, 7)
        self.assertEqual(self.client.debug_run([bp]), bp)

        self.client.debug_run_finish()
