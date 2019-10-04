from testlib.debuggertestcase import DebuggerTestCase

class TestBarriers(DebuggerTestCase):
    CLNAME = 'barriers_and_mem_sync.cl'
    FIRST_BARRIER_ROW = 20
    FENCE_BREAKPOINT = 22
    SECOND_BARRIER_ROW = 39
    FIRST_COPY_ROW = 48
    SECOND_COPY_ROW = 39

    def test_barrier_and_mem_fence(self):
        # test use of barrier and calls some mem fence commands
        # Set both global and local work sizes to 32.
        # 32 work items in a single work-group
        # TC-74, TC-75, TC-76, TC-77, TC-78, TC-79, TC-80, TC-81

        self.client.execute_debuggee(
            hostprog_name='ndrange_inout',
            cl_name=self.CLNAME,
            extra_args=['1024', '1', '32', '32'])
        self.client.connect_to_server()
        self.client.start_session(0, 0, 0)
        # pass first barrier
        bp = (self.CLNAME, self.FIRST_BARRIER_ROW)
        self.assertEqual(self.client.debug_run([bp]), bp)
        # initiallized to 1024 and incremented 32 times by each WI
        self.assertEqual(self.client.var_query_value('result'), '1056')
        # go before 1 mem fence command
        bp = (self.CLNAME, self.FENCE_BREAKPOINT)
        self.assertEqual(self.client.debug_run([bp]), bp)
        # go before 2 mem fence command
        bp = (self.CLNAME, self.FENCE_BREAKPOINT + 1)
        self.assertEqual(self.client.debug_run([bp]), bp)
        # go before 3 mem fence command
        bp = (self.CLNAME, self.FENCE_BREAKPOINT + 2)
        self.assertEqual(self.client.debug_run([bp]), bp)
        # go before 4 mem fence command
        bp = (self.CLNAME, self.FENCE_BREAKPOINT + 3)
        self.assertEqual(self.client.debug_run([bp]), bp)

        #####################################################################################################
        #   ERROR WHEN USING COMMAND WRITE_MEM_FENCE()                                                      #
        #   clearQuest ticket number - CSSD100007351                                                        #
        #   all following comment lines should be executed after the bug will be fixed                      #
        #   need to fix barriers_and_mem_sync.cl file                                                       #
        #####################################################################################################

        # go before 5 mem fence command
        #bp = (self.CLNAME, self.FENCE_BREAKPOINT + 4)
        #self.assertEqual(self.client.debug_run([bp]), bp)
         # go before 6 mem fence command
        #bp = (self.CLNAME, self.FENCE_BREAKPOINT + 5)
        #self.assertEqual(self.client.debug_run([bp]), bp)
        # go before 7 mem fence command
        #bp = (self.CLNAME, self.FENCE_BREAKPOINT + 6)
        #self.assertEqual(self.client.debug_run([bp]), bp)

        # pass second barrier
        bp = (self.CLNAME, self.SECOND_BARRIER_ROW)
        self.assertEqual(self.client.debug_run([bp]), bp)
        # initiallized to 0,1,2,3 and cell 0 was incremented 32 times by each WI
        # GDB and simulator clients print arrays with different notations...
        if self.use_gdb or self.use_cdb:
            self.assertEqual(self.client.var_query_value('local_arr'), '32,1,1,1')
        else:
            self.assertEqual(self.client.var_query_value('local_arr'), '[32|1|1|1]')
        self.client.debug_run_finish()
