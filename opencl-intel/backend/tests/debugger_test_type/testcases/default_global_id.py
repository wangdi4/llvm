from testlib.debuggertestcase import DebuggerTestCase, expectedFailureCDB

#
# TEST-CASE: DefaultGlobalId
#
# Tests the following issues:
#   1. Different work_items number with default global ID (TC-51)
#   2. Different number of dimensions which are used to specify the number of
#       work-items in work-group: minimum(TC-53) and maximum(TC-54)
#   3. Different offset used to calculate the global ID of a work item:
#       default(TC-55) and non-default(TC-56)
#   4. Minimum (TC-57) and maximum (TC-58) number of global work-items
#   5. Minimum (TC-60) and maximum (TC-61) size of work-group
#
class DefaultGlobalIdTest(DebuggerTestCase):

    #
    # TEST: DefaultGlobalId.test_1D_default_global_id
    #
    # Purpose
    # -------
    #
    # Test execution with the following parameters:
    #   1. Default global ID (0)
    #   2. Different number of work items (1, 16, 256, 4096)
    #   3. Minimum number of dimensions used to specify global work-items in
    #       work-group (1).
    #   4. global_work_offset is NULL - global ID start at offset 0.
    #   5. Minimum number of total number of global work-items (1)
    #   6. Minimum size of work-group
    def test_1D_default_global_id(self):
        import itertools
        CLNAME = 'check_ndrange_dimensions.cl'

        data_size_array = [1024]
        global_size_array = [1, 64, 4096]
        local_size_array = [1, 32, 1024]
#        global_size_array = [1]
#        local_size_array = [1]

        for (data_size,global_size_x,local_size_x) in (
            itertools.product(
                data_size_array,
                global_size_array,
                local_size_array)):
            if global_size_x >= local_size_x:
#               print(
#                   "global_size_array=",global_size_x,
#                   "local_size_array=",local_size_x)

                self.client.reset()
                self.client.execute_debuggee(
                    hostprog_name='ndrange_inout',
                    cl_name=CLNAME,
                    extra_args=[str(data_size),
                        '1',
                        str(global_size_x),
                        str(local_size_x)])
                self.client.connect_to_server()
                self.client.start_session(0, 0, 0)

                bp = (CLNAME, 29)
                self.assertEqual(self.client.debug_run([bp]), bp)


                self.assertEqual(self.client.var_query_value('workdim'), '1')

                self.assertEqual(
                    self.client.var_query_value('globalsize0'),
                    str(global_size_x))

                self.assertEqual(
                    self.client.var_query_value('localsize0'),
                    str(local_size_x))

                self.assertEqual(
                    self.client.var_query_value('numgroups0'),
                    str(global_size_x/local_size_x))

                self.assertEqual(self.client.var_query_value('gid0'), '0')

                self.assertEqual(self.client.var_query_value('lid0'), '0')

                self.assertEqual(self.client.var_query_value('groupid0'), '0')

                self.client.debug_run_finish()


    #
    # TEST: DefaultGlobalId.test_3D_default_global_id
    #
    # Purpose
    # -------
    #
    # Test execution with the following parameters:
    #   1. Default global ID (0,0,0)
    #   2. Different number of work items
    #   3. Maximum number of dimensions used to specify global work-items in
    #       work-group (3).
    #   4. global_work_offset is NULL - global ID start at offset (0,0,0).
    #   5. Minimum and maximum number of total number of global work-items
    #       (1 and 1024*1024*1024)
    #       Note: for now, the maximum number of total number of global
    #           work-items is (256*256*256) (CSSD100007249)
    #   6. Minimum and maximum size of work-group (1 and 1024)
    #
    def DISABLED_test_3D_default_global_id(self):
        import itertools
        CLNAME = 'check_ndrange_dimensions.cl'
        CL_DEVICE_MAX_WORK_GROUP_SIZE = 256
        CL_DEVICE_MAX_WORK_ITEM_SIZES = 16

        data_size_array = [64]
        global_size_array = [1, CL_DEVICE_MAX_WORK_ITEM_SIZES]
        local_size_array = [1, 8]
#        global_size_array = [1]
#        local_size_array = [1]

        for (data_size,
            global_size_x,
            global_size_y,
            global_size_z,
            local_size_x,
            local_size_y,
            local_size_z) in itertools.product(
                data_size_array,
                global_size_array,
                global_size_array,
                global_size_array,
                local_size_array,
                local_size_array,
                local_size_array):
            if ((global_size_x >= local_size_x) and
                (global_size_y >= local_size_y) and
                (global_size_z >= local_size_z)):
                work_group_size = (local_size_x * local_size_y * local_size_z)
                if work_group_size <= CL_DEVICE_MAX_WORK_GROUP_SIZE:
#                    print(
#                        "global_size_array=",
#                        global_size_x,global_size_y,global_size_z,
#                        "local_size_array=",
#                        local_size_x,local_size_y,local_size_z)

                    self.client.reset()
                    self.client.execute_debuggee(
                        hostprog_name='ndrange_inout',
                        cl_name=CLNAME,
                        extra_args=[str(data_size), '3',
                            str(global_size_x)+","+
                            str(global_size_y)+","+
                            str(global_size_z),
                            str(local_size_x)+","+
                            str(local_size_y)+","+
                            str(local_size_z)])
                    self.client.connect_to_server()
                    self.client.start_session(0, 0, 0)

                    bp = (CLNAME, 29)
                    self.assertEqual(self.client.debug_run([bp]), bp)

                    self.assertEqual(
                        self.client.var_query_value('workdim'), '3')

                    self.assertEqual(
                        self.client.var_query_value('globalsize0'),
                        str(global_size_x))
                    self.assertEqual(
                        self.client.var_query_value('globalsize1'),
                        str(global_size_y))
                    self.assertEqual(
                        self.client.var_query_value('globalsize2'),
                        str(global_size_z))

                    self.assertEqual(
                        self.client.var_query_value('localsize0'),
                        str(local_size_x))
                    self.assertEqual(
                        self.client.var_query_value('localsize1'),
                        str(local_size_y))
                    self.assertEqual(
                        self.client.var_query_value('localsize2'),
                        str(local_size_z))

                    self.assertEqual(
                        self.client.var_query_value('numgroups0'),
                        str(global_size_x/local_size_x))
                    self.assertEqual(
                        self.client.var_query_value('numgroups1'),
                        str(global_size_y/local_size_y))
                    self.assertEqual(
                        self.client.var_query_value('numgroups2'),
                        str(global_size_z/local_size_z))

                    self.assertEqual(
                        self.client.var_query_value('gid0'),
                        '0')
                    self.assertEqual(
                        self.client.var_query_value('gid1'),
                        '0')
                    self.assertEqual(
                        self.client.var_query_value('gid2'),
                        '0')

                    self.assertEqual(
                        self.client.var_query_value('lid0'),
                        '0')
                    self.assertEqual(
                        self.client.var_query_value('lid1'),
                        '0')
                    self.assertEqual(
                        self.client.var_query_value('lid2'),
                        '0')

                    self.assertEqual(
                        self.client.var_query_value('groupid0'),
                        '0')
                    self.assertEqual(
                        self.client.var_query_value('groupid1'),
                        '0')
                    self.assertEqual(
                        self.client.var_query_value('groupid2'),
                        '0')

                    self.client.debug_run_finish()
