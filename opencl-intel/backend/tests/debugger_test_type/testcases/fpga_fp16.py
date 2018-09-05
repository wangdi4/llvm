# ===-- fpga_fp16.py - a launcher for debugger test for FP16              -===//
#
# Copyright (C) 2018 Intel Corporation. All rights reserved.
#
# The information and source code contained herein is the exclusive property
# of Intel Corporation and may not be disclosed, examined or reproduced in
# whole or in part without explicit written authorization from the company.
#
# ===---------------------------------------------------------------------=== //

# Note: the test isn't supported by Windows debugger simulator

from testlib.debuggertestcase import DebuggerTestCase, skipNotGDB

class FP16(DebuggerTestCase):
    CLNAME = 'fpga_fp16.cl'

    @skipNotGDB
    # Test that fp16 variables can be printed and set from GDB.
    def test_fp16(self):
        self.client.execute_debuggee(
            hostprog_name='fpga_fp16',
            cl_name=self.CLNAME)

        self.client.connect_to_server()
        self.client.start_session(0, 0, 0)

        bps = [(self.CLNAME, 6)]
        self.assertEqual(self.client.debug_run(bps), bps[0])

        self.assertEqual(
            self.client.var_query_value('half_value'), '0.333251953')

        self.client.var_set_value('*output', '0.5')

        self.client.debug_run_finish()
