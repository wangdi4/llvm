from testlib.debuggertestcase import DebuggerTestCase, expectedFailureCDB


# Test a usage of query value for all variable types (global,local,private,volatile,restricted,regular and pointers)
# in diffrent scopes (global, main, while, if, function)
#
class TestVariableTypes(DebuggerTestCase):
    CLNAME = 'diffrent_declaration_types.cl'
    CLNAME2 = 'all_types_declaration.cl'
    # these "defines" are for test_all_variable_types_are_available
    FUNCTION_ROW = 7
    IF_BLOCK_ROW = 26
    IF_BLOCK_ROW2 = 38
    WHILE_BLOCK_ROW_BEFORE_FUNCTION = 34
    WHILE_BLOCK_ROW_AFTER_FUNCTION = 35
    MAIN_ROW = 41
    MAIN_VARAIBLE_AMOUNT = 8
    BLOCK_VARAIBLE_AMOUNT = 6
    SECOND_BLOCK_VARAIBLE_AMOUNT = 6
    GLOBAL_VARAIBLE_AMOUNT = 1
    FUNCTION_VARAIBLE_AMOUNT = 7
    # these "defines" are for test_all_variable_types_are_available2
    LAST_ROW = 139

    def test_all_variable_types_are_available(self):
    #
    # Test - all variable types (global, local, arguments, volatile, restricted, private) in diffrent
    #        kind of blocks (in main, in an inner block in main, in a function)
    # TC-33, TC-34, TC-35, TC-40, TC-41, TC-42, TC-43
    #
    # Note: there are no image types here because they can not be local variables
        self.client.execute_debuggee(
            hostprog_name='ndrange_inout',
            cl_name=self.CLNAME)
        self.client.connect_to_server()
        self.client.start_session(0, 0, 0)

        # check variables values in if block before while block
        bp = (self.CLNAME, self.IF_BLOCK_ROW)
        self.assertEqual(self.client.debug_run([bp]), bp)
        # check if variables
        self.assertEqual(self.client.var_query_value('gb'), '1')
        self.assertEqual(self.client.var_query_value('pb'), '3')
        self.assertEqual(self.client.var_query_value('vb'), '4')
        self.assertEqual(self.client.var_query_value('ab'), '5')
        self.assertEqual(self.client.var_query_value('rb'), '678')
        if not self.use_cdb: # Takes 30 sec to evaluate with cdb
            self.assertEqual(self.client.var_query_value('globalInt'), '1')
        # check main and global variables
        self.assertEqual(self.client.var_query_value('g'), '1')
        self.assertEqual(self.client.var_query_value('p'), '3')
        self.assertEqual(self.client.var_query_value('v'), '4')
        self.assertEqual(self.client.var_query_value('a'), '5')
        self.assertEqual(self.client.var_query_value('r'), '6')
        if not self.use_cdb: # Takes 30 sec to evaluate with cdb
            self.assertEqual(self.client.var_query_value('globalInt'), '1')

        # check variables values in while block before function call
        bp = (self.CLNAME, self.WHILE_BLOCK_ROW_BEFORE_FUNCTION)
        self.assertEqual(self.client.debug_run([bp]), bp)
        # check while block variables
        self.assertEqual(self.client.var_query_value('gb2'), '1')
        self.assertEqual(self.client.var_query_value('pb2'), '3')
        self.assertEqual(self.client.var_query_value('vb2'), '4')
        self.assertEqual(self.client.var_query_value('ab2'), '5')
        self.assertEqual(self.client.var_query_value('rb2'), '6789')
        # check if block variables
        self.assertEqual(self.client.var_query_value('gb'), '1')
        self.assertEqual(self.client.var_query_value('pb'), '3')
        self.assertEqual(self.client.var_query_value('vb'), '4')
        self.assertEqual(self.client.var_query_value('ab'), '6')
        self.assertEqual(self.client.var_query_value('rb'), '678')
        # check main and global variables
        self.assertEqual(self.client.var_query_value('g'), '1')
        self.assertEqual(self.client.var_query_value('p'), '3')
        self.assertEqual(self.client.var_query_value('v'), '4')
        self.assertEqual(self.client.var_query_value('a'), '5')
        self.assertEqual(self.client.var_query_value('r'), '6')
        if not self.use_cdb: # Takes 30 sec to evaluate with cdb
            self.assertEqual(self.client.var_query_value('globalInt'), '1')

        # check variables values in function called from while block
        bp = (self.CLNAME, self.FUNCTION_ROW)
        self.assertEqual(self.client.debug_run([bp]), bp)
        # check function variables
        self.assertEqual(self.client.var_query_value('gf'), '1')
        self.assertEqual(self.client.var_query_value('pf'), '3')
        self.assertEqual(self.client.var_query_value('pff'), '3')
        self.assertEqual(self.client.var_query_value('vf'), '4')
        self.assertEqual(self.client.var_query_value('af'), '5')
        self.assertEqual(self.client.var_query_value('rf'), '67')
        if not self.use_cdb: # Takes 30 sec to evaluate with cdb
            self.assertEqual(self.client.var_query_value('globalInt'), '1')

        # check variables values in while block after function call
        bp = (self.CLNAME, self.WHILE_BLOCK_ROW_AFTER_FUNCTION)
        self.assertEqual(self.client.debug_run([bp]), bp)
        # check while block variables
        self.assertEqual(self.client.var_query_value('gb2'), '1')
        self.assertEqual(self.client.var_query_value('pb2'), '3')
        self.assertEqual(self.client.var_query_value('vb2'), '4')
        self.assertEqual(self.client.var_query_value('ab2'), '5')
        self.assertEqual(self.client.var_query_value('rb2'), '6789')
        # check if block variables
        self.assertEqual(self.client.var_query_value('gb'), '1')
        self.assertEqual(self.client.var_query_value('pb'), '3')
        self.assertEqual(self.client.var_query_value('vb'), '4')
        self.assertEqual(self.client.var_query_value('ab'), '6')
        self.assertEqual(self.client.var_query_value('rb'), '678')
        # check main and global variables
        self.assertEqual(self.client.var_query_value('g'), '1')
        self.assertEqual(self.client.var_query_value('p'), '3')
        self.assertEqual(self.client.var_query_value('v'), '4')
        self.assertEqual(self.client.var_query_value('a'), '5')
        self.assertEqual(self.client.var_query_value('r'), '6')
        if not self.use_cdb: # Takes 30 sec to evaluate with cdb
            self.assertEqual(self.client.var_query_value('globalInt'), '1')

        # check variables values in if block after while block
        bp = (self.CLNAME, self.IF_BLOCK_ROW2)
        self.assertEqual(self.client.debug_run([bp]), bp)
        # check if variables
        self.assertEqual(self.client.var_query_value('gb'), '1')
        self.assertEqual(self.client.var_query_value('pb'), '3')
        self.assertEqual(self.client.var_query_value('vb'), '4')
        self.assertEqual(self.client.var_query_value('ab'), '6')
        self.assertEqual(self.client.var_query_value('rb'), '678')
        if not self.use_cdb: # Takes 30 sec to evaluate with cdb
            self.assertEqual(self.client.var_query_value('globalInt'), '1')
        # check main and global variables
        self.assertEqual(self.client.var_query_value('g'), '1')
        self.assertEqual(self.client.var_query_value('p'), '3')
        self.assertEqual(self.client.var_query_value('v'), '4')
        self.assertEqual(self.client.var_query_value('a'), '5')
        self.assertEqual(self.client.var_query_value('r'), '6')
        if not self.use_cdb: # Takes 30 sec to evaluate with cdb
            self.assertEqual(self.client.var_query_value('globalInt'), '1')

        # check variables values in function
        bp = (self.CLNAME, self.FUNCTION_ROW)
        self.assertEqual(self.client.debug_run([bp]), bp)
        # check function variables
        self.assertEqual(self.client.var_query_value('gf'), '1')
        self.assertEqual(self.client.var_query_value('pf'), '3')
        self.assertEqual(self.client.var_query_value('pff'), '3')
        self.assertEqual(self.client.var_query_value('vf'), '4')
        self.assertEqual(self.client.var_query_value('af'), '5')
        self.assertEqual(self.client.var_query_value('rf'), '67')
        if not self.use_cdb: # Takes 30 sec to evaluate with cdb
            self.assertEqual(self.client.var_query_value('globalInt'), '1')

        # check variables values in main
        bp = (self.CLNAME, self.MAIN_ROW)
        self.assertEqual(self.client.debug_run([bp]), bp)
        # check main and global variables
        self.assertEqual(self.client.var_query_value('g'), '1')
        self.assertEqual(self.client.var_query_value('p'), '3')
        self.assertEqual(self.client.var_query_value('v'), '4')
        self.assertEqual(self.client.var_query_value('a'), '5')
        self.assertEqual(self.client.var_query_value('r'), '6')
        if not self.use_cdb: # Takes 30 sec to evaluate with cdb
            self.assertEqual(self.client.var_query_value('globalInt'), '1')
        self.client.debug_run_finish()

    @expectedFailureCDB
    def test_all_variable_types_are_available2(self):
    #
    #  Test - test all variable types (int, char, bool, etc..) are available, by checking variables values after an assignment
    #  TC-36, TC-37
        self.client.execute_debuggee(
            hostprog_name='ndrange_inout',
            cl_name=self.CLNAME2)
        self.client.connect_to_server()
        self.client.start_session(0, 0, 0)
        bp = (self.CLNAME2, self.LAST_ROW)
        self.assertEqual(self.client.debug_run([bp]), bp)
        self.assertEqual(self.client.var_query_value('var_bool'), 'true')
        self.assertEqual(self.client.var_query_value('p_bool'), '34')
        self.assertEqual(self.client.var_query_value('var_char'), '2')
        self.assertEqual(self.client.var_query_value('p_char'), '34')
        self.assertEqual(self.client.var_query_value('var_uchar'), '2')
        self.assertEqual(self.client.var_query_value('p_uchar'), '34')
        self.assertEqual(self.client.var_query_value('var_short'), '28')
        self.assertEqual(self.client.var_query_value('p_short'), '34')
        self.assertEqual(self.client.var_query_value('var_ushort'), '28')
        self.assertEqual(self.client.var_query_value('p_ushort'), '34')
        self.assertEqual(self.client.var_query_value('var_int'), '28')
        self.assertEqual(self.client.var_query_value('p_int'), '34')
        self.assertEqual(self.client.var_query_value('var_uint'), '28')
        self.assertEqual(self.client.var_query_value('p_uint'), '34')
        self.assertEqual(self.client.var_query_value('var_long'), '28')
        self.assertEqual(self.client.var_query_value('p_long'), '34')
        self.assertEqual(self.client.var_query_value('var_ulong'), '28')
        self.assertEqual(self.client.var_query_value('p_ulong'), '34')
        self.assertEqual(self.client.var_query_value('var_float'), '28.0')
        self.assertEqual(self.client.var_query_value('p_float'), '34')
        self.assertEqual(self.client.var_query_value('var_double'), '28.0')
        self.assertEqual(self.client.var_query_value('p_double'), '34')
        self.assertEqual(self.client.var_query_value('var_size_t'), '28')
        self.assertEqual(self.client.var_query_value('p_size_t'), '34')
        self.assertEqual(self.client.var_query_value('var_ptrdiff_t'), '28')
        self.assertEqual(self.client.var_query_value('p_ptrdiff_t'), '34')
        self.assertEqual(self.client.var_query_value('var_intptr_t'), '28')
        self.assertEqual(self.client.var_query_value('p_intptr_t'), '34')
        self.assertEqual(self.client.var_query_value('var_uintptr_t'), '28')
        self.assertEqual(self.client.var_query_value('p_uintptr_t'), '34')
        self.assertEqual(self.client.var_query_value('var_char2'), '2,3')
        self.assertEqual(self.client.var_query_value('p_char2'), '34')
        self.assertEqual(self.client.var_query_value('var_char3'), '2,3,4')
        self.assertEqual(self.client.var_query_value('p_char3'), '34')
        self.assertEqual(self.client.var_query_value('var_char4'), '2,3,4,5')
        self.assertEqual(self.client.var_query_value('p_char4'), '34')
        self.assertEqual(self.client.var_query_value('var_char8'), '2,3,4,5,6,7,8,9')
        self.assertEqual(self.client.var_query_value('p_char8'), '34')
        self.assertEqual(self.client.var_query_value('var_char16'), '2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17')
        self.assertEqual(self.client.var_query_value('p_char16'), '34')
        # GDB prints uchar variables in different ways depending on the value.
        # As such, we handle the assertions below on a per-client basis.
        if self.use_gdb:
            self.assertEqual(self.client.var_query_value('var_uchar2'), '"\\002\\003"')
        else:
            self.assertEqual(self.client.var_query_value('var_uchar2'), '2,3')
        self.assertEqual(self.client.var_query_value('p_uchar2'), '34')
        if self.use_gdb:
            self.assertEqual(self.client.var_query_value('var_uchar3'), '"\\002\\003\\004"')
        else:
            self.assertEqual(self.client.var_query_value('var_uchar3'), '2,3,4')
        self.assertEqual(self.client.var_query_value('p_uchar3'), '34')
        if self.use_gdb:
            self.assertEqual(self.client.var_query_value('var_uchar4'), '"\\002\\003\\004\\005"')
        else:
            self.assertEqual(self.client.var_query_value('var_uchar4'), '2,3,4,5')
        self.assertEqual(self.client.var_query_value('p_uchar4'), '34')
        if self.use_gdb:
            expected_value = '"\\002\\003\\004\\005\\006\\a\\b\\t"'
            self.assertEqual(self.client.var_query_value('var_uchar8'), expected_value)
        else:
            self.assertEqual(self.client.var_query_value('var_uchar8'), '2,3,4,5,6,7,8,9')
        self.assertEqual(self.client.var_query_value('p_uchar8'), '34')
        if self.use_gdb:
            expected_value = '"\\002\\003\\004\\005\\006\\a\\b\\t\\n\\v\\f\\r\\016\\017\\020\\021"'
            self.assertEqual(self.client.var_query_value('var_uchar16'), expected_value)
        else:
            self.assertEqual(self.client.var_query_value('var_uchar16'), '2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17')
        self.assertEqual(self.client.var_query_value('p_uchar16'), '34')
        self.assertEqual(self.client.var_query_value('var_short2'), '2,3')
        self.assertEqual(self.client.var_query_value('p_short2'), '34')
        self.assertEqual(self.client.var_query_value('var_short3'), '2,3,4')
        self.assertEqual(self.client.var_query_value('p_short3'), '34')
        self.assertEqual(self.client.var_query_value('var_short4'), '2,3,4,5')
        self.assertEqual(self.client.var_query_value('p_short4'), '34')
        self.assertEqual(self.client.var_query_value('var_short8'), '2,3,4,5,6,7,8,9')
        self.assertEqual(self.client.var_query_value('p_short8'), '34')
        self.assertEqual(self.client.var_query_value('var_short16'), '2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17')
        self.assertEqual(self.client.var_query_value('p_short16'), '34')
        self.assertEqual(self.client.var_query_value('var_ushort2'), '2,3')
        self.assertEqual(self.client.var_query_value('p_ushort2'), '34')
        self.assertEqual(self.client.var_query_value('var_ushort3'), '2,3,4')
        self.assertEqual(self.client.var_query_value('p_ushort3'), '34')
        self.assertEqual(self.client.var_query_value('var_ushort4'), '2,3,4,5')
        self.assertEqual(self.client.var_query_value('p_ushort4'), '34')
        self.assertEqual(self.client.var_query_value('var_ushort8'), '2,3,4,5,6,7,8,9')
        self.assertEqual(self.client.var_query_value('p_ushort8'), '34')
        self.assertEqual(self.client.var_query_value('var_ushort16'), '2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17')
        self.assertEqual(self.client.var_query_value('p_ushort16'), '34')
        self.assertEqual(self.client.var_query_value('var_int2'), '2,3')
        self.assertEqual(self.client.var_query_value('p_int2'), '34')
        self.assertEqual(self.client.var_query_value('var_int3'), '2,3,4')
        self.assertEqual(self.client.var_query_value('p_int3'), '34')
        self.assertEqual(self.client.var_query_value('var_int4'), '2,3,4,5')
        self.assertEqual(self.client.var_query_value('p_int4'), '34')
        self.assertEqual(self.client.var_query_value('var_int8'), '2,3,4,5,6,7,8,9')
        self.assertEqual(self.client.var_query_value('p_int8'), '34')
        self.assertEqual(self.client.var_query_value('var_int16'), '2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17')
        self.assertEqual(self.client.var_query_value('p_int16'), '34')
        self.assertEqual(self.client.var_query_value('var_uint2'), '2,3')
        self.assertEqual(self.client.var_query_value('p_uint2'), '34')
        self.assertEqual(self.client.var_query_value('var_uint3'), '2,3,4')
        self.assertEqual(self.client.var_query_value('p_uint3'), '34')
        self.assertEqual(self.client.var_query_value('var_uint4'), '2,3,4,5')
        self.assertEqual(self.client.var_query_value('p_uint4'), '34')
        self.assertEqual(self.client.var_query_value('var_uint8'), '2,3,4,5,6,7,8,9')
        self.assertEqual(self.client.var_query_value('p_uint8'), '34')
        self.assertEqual(self.client.var_query_value('var_uint16'), '2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17')
        self.assertEqual(self.client.var_query_value('p_uint16'), '34')
        self.assertEqual(self.client.var_query_value('var_long2'), '2,3')
        self.assertEqual(self.client.var_query_value('p_long2'), '34')
        self.assertEqual(self.client.var_query_value('var_long3'), '2,3,4')
        self.assertEqual(self.client.var_query_value('p_long3'), '34')
        self.assertEqual(self.client.var_query_value('var_long4'), '2,3,4,5')
        self.assertEqual(self.client.var_query_value('p_long4'), '34')
        self.assertEqual(self.client.var_query_value('var_long8'), '2,3,4,5,6,7,8,9')
        self.assertEqual(self.client.var_query_value('p_long8'), '34')
        self.assertEqual(self.client.var_query_value('var_long16'), '2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17')
        self.assertEqual(self.client.var_query_value('p_long16'), '34')
        self.assertEqual(self.client.var_query_value('var_ulong2'), '2,3')
        self.assertEqual(self.client.var_query_value('p_ulong2'), '34')
        self.assertEqual(self.client.var_query_value('var_ulong3'), '2,3,4')
        self.assertEqual(self.client.var_query_value('p_ulong3'), '34')
        self.assertEqual(self.client.var_query_value('var_ulong4'), '2,3,4,5')
        self.assertEqual(self.client.var_query_value('p_ulong4'), '34')
        self.assertEqual(self.client.var_query_value('var_ulong8'), '2,3,4,5,6,7,8,9')
        self.assertEqual(self.client.var_query_value('p_ulong8'), '34')
        self.assertEqual(self.client.var_query_value('var_ulong16'), '2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17')
        self.assertEqual(self.client.var_query_value('p_ulong16'), '34')
        self.assertEqual(self.client.var_query_value('var_float2'), '2.0,3.0')
        self.assertEqual(self.client.var_query_value('p_float2'), '34')
        self.assertEqual(self.client.var_query_value('var_float3'), '2.0,3.0,4.0')
        self.assertEqual(self.client.var_query_value('p_float3'), '34')
        self.assertEqual(self.client.var_query_value('var_float4'), '2.0,3.0,4.0,5.0')
        self.assertEqual(self.client.var_query_value('p_float4'), '34')
        self.assertEqual(self.client.var_query_value('var_float8'), '2.0,3.0,4.0,5.0,6.0,7.0,8.0,9.0')
        self.assertEqual(self.client.var_query_value('p_float8'), '34')
        self.assertEqual(self.client.var_query_value('var_float16'), '2.0,3.0,4.0,5.0,6.0,7.0,8.0,9.0,2.0,3.0,4.0,5.0,6.0,7.0,8.0,9.0')
        self.assertEqual(self.client.var_query_value('p_float16'), '34')
        self.assertEqual(self.client.var_query_value('var_double2'), '2.0,3.0')
        self.assertEqual(self.client.var_query_value('p_double2'), '34')
        self.assertEqual(self.client.var_query_value('var_double3'), '2.0,3.0,4.0')
        self.assertEqual(self.client.var_query_value('p_double3'), '34')
        self.assertEqual(self.client.var_query_value('var_double4'), '2.0,3.0,4.0,5.0')
        self.assertEqual(self.client.var_query_value('p_double4'), '34')
        self.assertEqual(self.client.var_query_value('var_double8'), '2.0,3.0,4.0,5.0,6.0,7.0,8.0,9.0')
        self.assertEqual(self.client.var_query_value('p_double8'), '34')
        self.assertEqual(self.client.var_query_value('var_double16'), '2.0,3.0,4.0,5.0,6.0,7.0,8.0,9.0,2.0,3.0,4.0,5.0,6.0,7.0,8.0,9.0')
        self.assertEqual(self.client.var_query_value('p_double16'), '34')
        self.assertEqual(self.client.var_query_value('var_sampler_t'), '1')
        self.assertEqual(self.client.var_query_value('p_event_t'), '34')
        self.assertEqual(self.client.var_query_value('p_half'), '34')
        self.client.debug_run_finish()
