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
# * stop breakpoints, step and continue in autorun kernel
# Note: the test isn't supported by Windows debugger simulator

from testlib.debuggertestcase import DebuggerTestCase, skipNotGDB

class FPGAAutorun(DebuggerTestCase):
    CLNAME = 'fpga_autorun.cl'

    @skipNotGDB
    def test_breakpoints(self):
        self.client.execute_debuggee(
            hostprog_name='fpga_autorun',
            cl_name=self.CLNAME,
            options={'build_opts':'"-DN=5"'})
        self.client.connect_to_server()
        self.client.start_session(0, 0, 0)

        # Set a breakpoint in an autorun kernel
        bp = (self.CLNAME, 8)
        self.assertEqual(self.client.debug_run([bp]), bp)

        # Set a breakpoint and test 'next' and 'continue'
        bps = [(self.CLNAME, 16), (self.CLNAME, 17)]

        # Prevent switching threads
        self.client._command('set scheduler-locking step')

        # Go through all 5 replicas of autorun kernel 'chained_plus()'
        # stop at the 16th line and test 'next'
        self.assertEqual(self.client.debug_run([bps[0]]), bps[0])
        self.assertEqual(self.client.debug_step_over(), bps[1])

        self.assertEqual(self.client.debug_continue(), bps[0])
        self.assertEqual(self.client.debug_step_over(), bps[1])

        self.assertEqual(self.client.debug_continue(), bps[0])
        self.assertEqual(self.client.debug_step_over(), bps[1])

        self.assertEqual(self.client.debug_continue(), bps[0])
        self.assertEqual(self.client.debug_step_over(), bps[1])

        self.assertEqual(self.client.debug_continue(), bps[0])
        self.assertEqual(self.client.debug_step_over(), bps[1])

        self.client.debug_run_finish()
