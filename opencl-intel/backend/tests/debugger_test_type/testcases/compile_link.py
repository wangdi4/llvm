# ===-- compile_link.py - a launcher for debugger test                   --===//
#
# Copyright (C) 2019 Intel Corporation. All rights reserved.
#
# The information and source code contained herein is the exclusive property
# of Intel Corporation and may not be disclosed, examined or reproduced in
# whole or in part without explicit written authorization from the company.
#
# ===---------------------------------------------------------------------=== //
# The test check that
# * if a program is built using compile + link flow, full debug information is
#   available

from testlib.debuggertestcase import DebuggerTestCase, skipNotGDB

class CompileLink(DebuggerTestCase):
    CLNAME = 'compile_link.cl'

    @skipNotGDB
    def test_breakpoints(self):
        self.client.execute_debuggee(
            hostprog_name='host_compile_link',
            cl_name=self.CLNAME)
        self.client.connect_to_server()
        self.client.start_session(0, 0, 0)


        bp = (self.CLNAME, 1)
        self.client.debug_run([bp])

        # Check that we have "special" variables which are only available if
        # "-g" flag was handled properly
        self.assertEqual(
            self.client.var_query_value('__ocl_dbg_gid0'), '0')

        self.client.debug_run_finish()
