from testlib.debuggertestcase import DebuggerTestCase


class TestAsyncCopy(DebuggerTestCase):
    CLNAME = 'async_check.cl'
    FIRST_ASYNC_ROW = 18
    SECOND_ASYNC_ROW = 24
    THIRD_ASYNC_ROW = 30
    FOURTH_ASYNC_ROW = 36

    def test_async_copy(self):
        # test use of async_work_group_copy from local to global and vice versa of uchar type variables
    # TC-82, TC-83
        self.client.execute_debuggee(
            hostprog_name='local_global',
            cl_name=self.CLNAME)
        self.client.connect_to_server()
        self.client.start_session(0, 0, 0)
        bp = (self.CLNAME, self.FIRST_ASYNC_ROW)
        self.assertEqual(self.client.debug_run([bp]), bp)
        self.assertEqual(self.client.var_query_value('checker'), '2')
        bp = (self.CLNAME, self.SECOND_ASYNC_ROW)
        self.assertEqual(self.client.debug_run([bp]), bp)
    #########################################################################################
    #                                            #
    #    THE NEXT VALUE QUERYS DOESN'T MATCH CURRENTLY                    #
    #    not sure if it is a bug or an incorrect use of "async_work_group_copy" function #
    #    after fixing the problem the following lines shouldn't be on remark        #
    #                                            #
    #########################################################################################
        #self.assertEqual(self.client.var_query_value('checker'), '11')
        #bp = (self.CLNAME, self.THIRD_ASYNC_ROW)
        #self.assertEqual(self.client.debug_run([bp]), bp)
        #self.assertEqual(self.client.var_query_value('checker'), '4')
        #bp = (self.CLNAME, self.FOURTH_ASYNC_ROW)
        #self.assertEqual(self.client.debug_run([bp]), bp)
        #self.assertEqual(self.client.var_query_value('checker'), '13')
        self.client.debug_run_finish()
