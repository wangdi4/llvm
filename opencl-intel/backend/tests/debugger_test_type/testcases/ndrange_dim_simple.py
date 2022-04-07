from testlib.debuggertestcase import DebuggerTestCase, expectedFailureGDB, expectedFailureCDB


class NDRangeDimSimple(DebuggerTestCase):
    @expectedFailureCDB
    def test_ndrange_1d_dimensions(self):
        # 16 work items, 8 per group ==> 2 groups
        # WI 13 is then in WG 2, and its local ID there is 5
        CLNAME = 'check_ndrange_dimensions.cl'
        self.client.execute_debuggee(
            hostprog_name='ndrange_inout',
            cl_name=CLNAME,
            extra_args=['128', '1', '16', '8'])
        self.client.connect_to_server()
        self.client.start_session(13, 0, 0)

        bp = (CLNAME, 29)
        self.assertEqual(self.client.debug_run([bp]), bp)

        self.assertEqual(self.client.var_query_value('workdim'), '1')

        self.assertEqual(self.client.var_query_value('globalsize0'), '16')

        self.assertEqual(self.client.var_query_value('localsize0'), '8')

        self.assertEqual(self.client.var_query_value('numgroups0'), '2')

        self.assertEqual(self.client.var_query_value('gid0'), '13')

        self.assertEqual(self.client.var_query_value('lid0'), '5')

        self.assertEqual(self.client.var_query_value('groupid0'), '1')

        self.client.debug_run_finish()

    @expectedFailureCDB
    def test_ndrange_2d_dimensions(self):
        # x: 16 work items, 2 per group ==> 8 groups
        #    WI 13 is then in WG 6, and its local ID there is 1
        #
        # y: 64 work items, 8 per group ==> 8 groups
        #    WI 50 is in WG 6, and its local ID there is 2
        #
        CLNAME = 'check_ndrange_dimensions.cl'
        self.client.execute_debuggee(
            hostprog_name='ndrange_inout',
            cl_name=CLNAME,
            extra_args=['4096', '2', '16,64', '2,8'])
        self.client.connect_to_server()
        self.client.start_session(13, 50, 0)

        bp = (CLNAME, 29)
        self.assertEqual(self.client.debug_run([bp]), bp)

        self.assertEqual(self.client.var_query_value('workdim'), '2')

        self.assertEqual(self.client.var_query_value('globalsize0'), '16')
        self.assertEqual(self.client.var_query_value('globalsize1'), '64')

        self.assertEqual(self.client.var_query_value('localsize0'), '2')
        self.assertEqual(self.client.var_query_value('localsize1'), '8')

        self.assertEqual(self.client.var_query_value('numgroups0'), '8')
        self.assertEqual(self.client.var_query_value('numgroups1'), '8')

        self.assertEqual(self.client.var_query_value('gid0'), '13')
        self.assertEqual(self.client.var_query_value('gid1'), '50')

        self.assertEqual(self.client.var_query_value('lid0'), '1')
        self.assertEqual(self.client.var_query_value('lid1'), '2')

        self.assertEqual(self.client.var_query_value('groupid0'), '6')
        self.assertEqual(self.client.var_query_value('groupid1'), '6')

        self.client.debug_run_finish()

    # Expected to fail due to CSSD100014059
    # In larger kernels like this one, the overhead to evaluate a conditional
    # breakpoint causes a timeout.
    @expectedFailureCDB
    @expectedFailureGDB
    def test_ndrange_3d_dimensions(self):
        # x: 16 work items, 2 per group ==> 8 groups
        #    WI 13 is then in WG 6, and its local ID there is 1
        #
        # y: 256 work items, 8 per group ==> 32 groups
        #    WI 50 is in WG 6, and its local ID there is 2
        #
        # z: 1024 work items, 64 per group ==> 16 groups
        #    WI 1023 is in WG 15, and its local ID there is 63
        #
        CLNAME = 'check_ndrange_dimensions.cl'
        self.client.execute_debuggee(
            hostprog_name='ndrange_inout',
            cl_name=CLNAME,
            extra_args=['4096', '3', '16,256,1024', '2,8,64'])
        self.client.connect_to_server()
        self.client.start_session(13, 50, 1023)

        bp = (CLNAME, 29)
        self.assertEqual(self.client.debug_run([bp]), bp)

        self.assertEqual(self.client.var_query_value('workdim'), '3')

        self.assertEqual(self.client.var_query_value('globalsize0'), '16')
        self.assertEqual(self.client.var_query_value('globalsize1'), '256')
        self.assertEqual(self.client.var_query_value('globalsize2'), '1024')

        self.assertEqual(self.client.var_query_value('localsize0'), '2')
        self.assertEqual(self.client.var_query_value('localsize1'), '8')
        self.assertEqual(self.client.var_query_value('localsize2'), '64')

        self.assertEqual(self.client.var_query_value('numgroups0'), '8')
        self.assertEqual(self.client.var_query_value('numgroups1'), '32')
        self.assertEqual(self.client.var_query_value('numgroups2'), '16')

        self.assertEqual(self.client.var_query_value('gid0'), '13')
        self.assertEqual(self.client.var_query_value('gid1'), '50')
        self.assertEqual(self.client.var_query_value('gid2'), '1023')

        self.assertEqual(self.client.var_query_value('lid0'), '1')
        self.assertEqual(self.client.var_query_value('lid1'), '2')
        self.assertEqual(self.client.var_query_value('lid2'), '63')

        self.assertEqual(self.client.var_query_value('groupid0'), '6')
        self.assertEqual(self.client.var_query_value('groupid1'), '6')
        self.assertEqual(self.client.var_query_value('groupid2'), '15')

        self.client.debug_run_finish()
