from testlib.debuggertestcase import DebuggerTestCase, expectedFailureCDB

#
# TEST-CASE: GlobalId
#
# Tests the following issues:
#   1. Different work_items number with non-default global ID (TC-52)
#   2. Different number of dimensions which are used to specify the number of
#       work-items in work-group: minimum(TC-53) and maximum(TC-54)
#   3. Different offset used to calculate the global ID of a work item:
#       default(TC-55) and non-default(TC-56)
#   4. Minimum (TC-57) and maximum (TC-58) number of global work-items
#   5. Minimum (TC-60) and maximum (TC-61) size of work-group
#
class GlobalIdTest(DebuggerTestCase):

    #
    # TEST: GlobalId.test_1D_non_default_global_id
    #
    # Purpose
    # -------
    #
    # Test execution with the following parameters:
    #   1. Non-default global ID
    #   2. Different number of work items (1, 16, 256, 4096)
    #   3. Minimum number of dimensions used to specify global work-items in
    #       work-group (1)
    #   4. Description of the offset used to calculate the global ID of a
    #       work-item - global_work_offset is not NULL
    #   5. Minimum number of total number of global work-items (1)
    #   6. Minimum size of work-group (1)
    #
    # For each test we generate a random global_work_offset (global_offeset_x)
    #   and a global_id (global_id_x).
    # Based on global_id_x, we calculate the group_id (group_id_x) and the
    #   local_id (local_id_x)
    #
    @expectedFailureCDB
    def test_1D_non_default_global_id(self):
        import random
        import itertools
        CLNAME = 'check_ndrange_dimensions.cl'

        data_size_array = [1024]
        global_size_array = [1, 64, 4096]
        local_size_array = [1, 16, 256]
#        global_size_array = [1]
#        local_size_array = [1]

        for (data_size,global_size_x,local_size_x) in (
            itertools.product(data_size_array,global_size_array,local_size_array)):
            if global_size_x >= local_size_x:
                global_offset_x = random.randint(0,global_size_x-1)
                global_id_x = random.randint(0,global_size_x-global_offset_x-1)
                numgroups0 = global_size_x/local_size_x
                group_id_x = global_id_x/local_size_x
                local_id_x = global_id_x - (group_id_x * local_size_x)

#                print(
#                    "global_size_array=",global_size_x,
#                    "local_size_array=",local_size_x,
#                    "global_offset=",global_offset_x,
#                    "global_id=",global_id_x,
#                    "group_id=",group_id_x,
#                    "local_id=",local_id_x)

                self.client.reset()
                self.client.execute_debuggee(
                    hostprog_name='ndrange_inout',
                    cl_name=CLNAME,
                    extra_args=[
                        str(data_size),
                        '1',
                        str(global_size_x),
                        str(local_size_x)])
                self.client.connect_to_server()
                self.client.start_session(global_id_x, 0, 0)

                asert_string = (
                    "global_offset=" + str(global_offset_x) +
                    " global_id=" + str(global_id_x))

                bp = (CLNAME, 29)
                self.assertEqual(self.client.debug_run([bp]), bp, asert_string)

                self.assertEqual(self.client.var_query_value('workdim'),
                                 '1',
                                 asert_string)

                self.assertEqual(
                    self.client.var_query_value('globalsize0'),
                    str(global_size_x),
                    asert_string)

                self.assertEqual(
                    self.client.var_query_value('localsize0'),
                    str(local_size_x),
                    asert_string)

                self.assertEqual(
                    self.client.var_query_value('numgroups0'),
                    str(numgroups0),
                    asert_string)

                self.assertEqual(
                    self.client.var_query_value('gid0'),
                    str(global_id_x),
                    asert_string)

                self.assertEqual(
                    self.client.var_query_value('lid0'),
                    str(local_id_x),
                    asert_string)

                self.assertEqual(
                    self.client.var_query_value('groupid0'),
                    str(group_id_x),
                    asert_string)

                self.client.debug_run_finish()


    #
    # TEST: GlobalId.test_3D_non_default_global_id
    #
    # Purpose
    # -------
    #
    # Test execution with the following parameters:
    #   1. Non-default global ID
    #   2. Different number of work items
    #   3. Maximum number of dimensions used to specify global work-items in
    #       work-group (3).
    #   4. Description of the offset used to calculate the global ID of a
    #       work-item - global_work_offset is not NULL
    #   5. Minimum and maximum number of total number of global work-items (1 and 1024*1024*1024)
    #   6. Minimum and maximum size of work-group (1 and 1024)
    #       Note: for now, the maximum number of total number of global
    #           work-items is (256*256*256) (CSSD100007249)
#
    # For each test we generate a random global_work_offset (global_ffset_x,
    #   global_offset_y, global_offset_z) and a global_id (global_id_x, global_id_y, global_id_z).
    # Based on global_id, we calculate the group_id (group_id_x, group_id_y,
    #   group_id_z) and the local_id (local_id_x, local_id_y, local_id_z)
    #
    def DISABLED_test_3D_non_default_global_id(self):
        import random
        import itertools
        CLNAME = 'check_ndrange_dimensions.cl'
        CL_DEVICE_MAX_WORK_GROUP_SIZE = 256
        CL_DEVICE_MAX_WORK_ITEM_SIZES = 16

        data_size_array = [64]
        global_size_array = [1, CL_DEVICE_MAX_WORK_ITEM_SIZES]
        local_size_array = [1, 8]
#        global_size_array = [1]
#        local_size_array = [1]

        for (
            data_size,
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
            if (
                (global_size_x >= local_size_x) and
                (global_size_y >= local_size_y) and
                (global_size_z >= local_size_z)):
                work_group_size = (local_size_x * local_size_y * local_size_z)
                if work_group_size <= CL_DEVICE_MAX_WORK_GROUP_SIZE:
                    global_offset_x = random.randint(0,global_size_x-1)
                    global_offset_y = random.randint(0,global_size_y-1)
                    global_offset_z = random.randint(0,global_size_z-1)
                    global_id_x = random.randint(
                        0,global_size_x-global_offset_x-1)
                    global_id_y = random.randint(
                        0,global_size_y-global_offset_y-1)
                    global_id_z = random.randint(
                        0,global_size_z-global_offset_z-1)
                    numgroups0 = global_size_x/local_size_x
                    numgroups1 = global_size_y/local_size_y
                    numgroups2 = global_size_z/local_size_z
                    group_id_x = global_id_x/local_size_x;
                    group_id_y = global_id_y/local_size_y;
                    group_id_z = global_id_z/local_size_z;
                    local_id_x = global_id_x - (group_id_x * local_size_x)
                    local_id_y = global_id_y - (group_id_y * local_size_y)
                    local_id_z = global_id_z - (group_id_z * local_size_z)

#                    print(
#                        "global_size_array=",
#                        global_size_x,global_size_y,global_size_z,
#                        "local_size_array=",
#                        local_size_x,local_size_y,local_size_z,
#                        "global_work_offset=",
#                        global_offset_x,global_offset_y,global_offset_z,
#                        "global_id=",
#                        global_id_x,global_id_y,global_id_z,
#                        "group_id=",
#                        group_id_x,group_id_y,group_id_z,
#                        "local_id=",
#                        local_id_x,local_id_y,local_id_z)

                    self.client.reset()
                    self.client.execute_debuggee(
                        hostprog_name='ndrange_inout',
                        cl_name=CLNAME,
                        extra_args=[
                            str(data_size), '3',
                            str(global_size_x)+","+
                            str(global_size_y)+","+
                            str(global_size_z),
                            str(local_size_x)+","+
                            str(local_size_y)+","+
                            str(local_size_z)])
                    self.client.connect_to_server()
                    self.client.start_session(
                        global_id_x,
                        global_id_y,
                        global_id_z)

                    asert_string = (
                        "global_offset=" +
                        str(global_offset_x) + "," +
                        str(global_offset_y) + "," +
                        str(global_offset_z) + "," +
                        " global_id=" +
                        str(global_id_x) + "," +
                        str(global_id_y) + "," +
                        str(global_id_z))

                    bp = (CLNAME, 29)
                    self.assertEqual(
                        self.client.debug_run([bp],60),
                        bp,
                        asert_string)

                    self.assertEqual(
                        self.client.var_query_value('workdim'),
                        '3',
                        asert_string)

                    self.assertEqual(
                        self.client.var_query_value('globalsize0'),
                        str(global_size_x),
                        asert_string)

                    self.assertEqual(
                        self.client.var_query_value('globalsize1'),
                        str(global_size_y),
                        asert_string)

                    self.assertEqual(
                        self.client.var_query_value('globalsize2'),
                        str(global_size_z),
                        asert_string)

                    self.assertEqual(
                        self.client.var_query_value('localsize0'),
                        str(local_size_x),
                        asert_string)

                    self.assertEqual(
                        self.client.var_query_value('localsize1'),
                        str(local_size_y),
                        asert_string)

                    self.assertEqual(
                        self.client.var_query_value('localsize2'),
                        str(local_size_z),
                        asert_string)

                    self.assertEqual(
                        self.client.var_query_value('numgroups0'),
                        str(numgroups0),
                        asert_string)

                    self.assertEqual(
                        self.client.var_query_value('numgroups1'),
                        str(numgroups1),
                        asert_string)

                    self.assertEqual(
                        self.client.var_query_value('numgroups2'),
                        str(numgroups2),
                        asert_string)

                    self.assertEqual(
                        self.client.var_query_value('gid0'),
                        str(global_id_x),
                        asert_string)

                    self.assertEqual(
                        self.client.var_query_value('gid1'),
                        str(global_id_y),
                        asert_string)

                    self.assertEqual(
                        self.client.var_query_value('gid2'),
                        str(global_id_z),
                        asert_string)

                    self.assertEqual(
                        self.client.var_query_value('lid0'),
                        str(local_id_x),
                        asert_string)

                    self.assertEqual(
                        self.client.var_query_value('lid1'),
                        str(local_id_y),
                        asert_string)

                    self.assertEqual(
                        self.client.var_query_value('lid2'),
                        str(local_id_z),
                        asert_string)

                    self.assertEqual(
                        self.client.var_query_value('groupid0'),
                        str(group_id_x),
                        asert_string)

                    self.assertEqual(
                        self.client.var_query_value('groupid1'),
                        str(group_id_y),
                        asert_string)

                    self.assertEqual(
                        self.client.var_query_value('groupid2'),
                        str(group_id_z),
                        asert_string)

                    self.client.debug_run_finish()
