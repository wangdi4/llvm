from testlib.debuggertestcase import DebuggerTestCase, expectedFailureCDB

class TestStepsAndVariableValues(DebuggerTestCase):
    CLNAME = 'nested_calls4.cl'

    # GDB cannot stop on line 57 because it is an end-brace; use 58 instead
    # to represent the same "stop after the if statement" logic
    INNER1_FUNCTION_ROW = 58
    INNER1_FUNCTION_BLOCK_ROW = 55
    INNER2_FUNCTION_ROW = 32
    INNER2_FUNCTION_BLOCK_ROW = 37
    INNER3_FUNCTION_ROW = 22
    INNER3_FUNCTION_BLOCK_ROW = 19
    FOO_FUNCTION_ROW = 6

    def test_in_nested_function_calls(self):
    #
    #  Test - test the variable values query after making diffrent type of step (in, over and out)
    #  TC-47, TC-48, TC-49
        gdb_offset = 0
        if self.use_gdb or self.use_cdb:
          gdb_offset = 1

        self.client.execute_debuggee(
            hostprog_name='ndrange_inout',
            cl_name=self.CLNAME)
        self.client.connect_to_server()
        self.client.start_session(0, 0, 0)

        # check variables values in block and after block in function test3, after using step in
        bp = (self.CLNAME, self.INNER3_FUNCTION_BLOCK_ROW)
        self.assertEqual(self.client.debug_run([bp]), bp)
        bp = (self.CLNAME, self.INNER3_FUNCTION_BLOCK_ROW+1)
        self.assertEqual(self.client.debug_step_in(), bp)
        # in block
        self.assertEqual(self.client.var_query_value('i3'), '4')
        self.assertEqual(self.client.var_query_value('a'), '3')
        self.assertEqual(self.client.var_query_value('c3'), '2')
        self.assertEqual(self.client.var_query_value('r3'), '67')
        self.assertEqual(self.client.var_query_value('ai3'), '4')
        self.assertEqual(self.client.var_query_value('ac3'), '2')
        self.assertEqual(self.client.var_query_value('globalLong'), '1,22,333,4444')
        # GDB does not step into closing-brace instructions so apply offset
        # as GDB stops on the next line after the brace.
        bp = (self.CLNAME, self.INNER3_FUNCTION_BLOCK_ROW + 2 + gdb_offset)
        self.assertEqual(self.client.debug_step_in(), bp)
        if not self.use_gdb and not self.use_cdb:
          # Simulator debugger considers the closing-brace a steppable
          # instruction, whereas GDB does not. Advance simulator to next line.
          bp = (self.CLNAME, self.INNER3_FUNCTION_ROW)
          self.assertEqual(self.client.debug_step_in(), bp)
        # after block
        self.assertEqual(self.client.var_query_value('i3'), '4')
        self.assertEqual(self.client.var_query_value('a'), '3')
        self.assertEqual(self.client.var_query_value('c3'), '2')
        self.assertEqual(self.client.var_query_value('r3'), '67')
        self.assertEqual(self.client.var_query_value('globalLong'), '1,22,333,4444')

        # check variables values in block and before block in function test2, after using step out
        # The source location of the instruction after step out could be either the call line
        # or the next one
        bp = (self.CLNAME, self.INNER2_FUNCTION_ROW)
        result = self.client.debug_step_out()
        assertion = result[0] == bp[0] and (result[1] == bp[1] or result[1] == bp[1] + 1)
        self.assertTrue(assertion)
        # before block
        self.assertEqual(self.client.var_query_value('i2'), '3')
        self.assertEqual(self.client.var_query_value('a'), '2')
        self.assertEqual(self.client.var_query_value('c2'), '2')
        self.assertEqual(self.client.var_query_value('r2'), '67')
        self.assertEqual(self.client.var_query_value('globalLong'), '1,22,333,4444')
        bp = (self.CLNAME, self.FOO_FUNCTION_ROW)
        self.assertEqual(self.client.debug_run([bp]), bp)
        # The source location of the instruction after step out could be either the call line
        # or the next one
        bp = (self.CLNAME, self.INNER2_FUNCTION_BLOCK_ROW)
        result = self.client.debug_step_out()
        assertion = result[0] == bp[0] and (result[1] == bp[1] or result[1] == bp[1] + 1)
        self.assertTrue(assertion)
        # in block
        self.assertEqual(self.client.var_query_value('i2'), '3')
        self.assertEqual(self.client.var_query_value('a'), '2')
        self.assertEqual(self.client.var_query_value('c2'), '2')
        self.assertEqual(self.client.var_query_value('r2'), '67')
        self.assertEqual(self.client.var_query_value('ai2'), '3')
        self.assertEqual(self.client.var_query_value('ac2'), '2')
        self.assertEqual(self.client.var_query_value('globalLong'), '1,22,333,4444')

        # check variables values in block and after block in function test1, after using step over
        bp = (self.CLNAME, self.INNER1_FUNCTION_BLOCK_ROW)
        # See Defect 5559
        # if self.use_gdb:
        #  self.assertEqual(self.client.debug_run([bp]), (self.CLNAME, self.FOO_FUNCTION_ROW))
        self.assertEqual(self.client.debug_run([bp]), bp)
        bp = (self.CLNAME, self.INNER1_FUNCTION_BLOCK_ROW + 1)
        self.assertEqual(self.client.debug_step_over(), bp)
        # in block
        self.assertEqual(self.client.var_query_value('i1'), '2')
        self.assertEqual(self.client.var_query_value('a'), '1')
        self.assertEqual(self.client.var_query_value('c1'), '2')
        self.assertEqual(self.client.var_query_value('r1'), '67')
        self.assertEqual(self.client.var_query_value('ac1'), '2')
        self.assertEqual(self.client.var_query_value('ai1'), '2')
        self.assertEqual(self.client.var_query_value('globalLong'), '1,22,333,4444')
        bp = (self.CLNAME, self.INNER1_FUNCTION_ROW)
        self.assertEqual(self.client.debug_run([bp]), bp)
        bp = (self.CLNAME, self.INNER1_FUNCTION_ROW + 1)
        self.assertEqual(self.client.var_query_value('a'), '1')
        self.assertEqual(self.client.debug_step_over(), bp)
        self.assertEqual(self.client.var_query_value('a'), '2')
        # after block
        self.assertEqual(self.client.var_query_value('i1'), '2')
        self.assertEqual(self.client.var_query_value('c1'), '2')
        self.assertEqual(self.client.var_query_value('r1'), '67')
        self.assertEqual(self.client.var_query_value('globalLong'), '1,22,333,4444')
        self.client.debug_run_finish()
