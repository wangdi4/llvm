from testlib.debuggertestcase import DebuggerTestCase


# Test a simple usage of steps for TC-10-24
#
class TestSteps(DebuggerTestCase):
    CLNAME1 = 'nested_calls2.cl'
    CLNAME2 = 'simple_program.cl'
    CLNAME3 = 'simple_function_call.cl'
    ERROR_MSG = 'Expected RUN to have been executed at least once\n'
    def test_simple_step_in_and_incorrect_stack_trace_use(self):
    #
    # TC-10, TC-25
    # test the usage of steps in, of a program with no function calls
    # test trying to get stack trace without setting a breakpoint

        from testlib.clientsimulator import SimulatorError
        self.client.execute_debuggee(
            hostprog_name='ndrange_inout',
            cl_name=self.CLNAME2)
        self.client.connect_to_server()
        self.client.start_session(0, 0, 0)
        try:
            self.client.get_stack_trace()
        except SimulatorError as e:
            import StringIO
            message = StringIO.StringIO()
            print >>message, e
            self.assertEqual(message.getvalue(), self.ERROR_MSG)
            message.close()
        else:
            self.assertEqual('this should not happen', '!!!')
        self.client.debug_run_finish()
        
    def test_step_in_no_function_calls(self):
    #
    # TC-12
    # test the usage of steps in, of a program with no function calls
    #
        self.client.execute_debuggee(
            hostprog_name='ndrange_inout',
            cl_name=self.CLNAME2)
        self.client.connect_to_server()
        self.client.start_session(0, 0, 0)
        # add some breakpoints and go to them with the step in mechanism
        bp = (self.CLNAME2, 3)
        bps = [bp, (self.CLNAME2, 4), (self.CLNAME2, 5), (self.CLNAME2, 6)]
        self.assertEqual(self.client.debug_run(bps), bp)
        bp = (self.CLNAME2, 4)
        self.assertEqual(self.client.debug_step_in(), bp)
        bp = (self.CLNAME2, 5)
        self.assertEqual(self.client.debug_step_in(), bp)
        bp = (self.CLNAME2, 6)
        self.assertEqual(self.client.debug_step_in(), bp)
        bp = (self.CLNAME2, 5)
        self.assertEqual(self.client.debug_step_in(), bp)
        bp = (self.CLNAME2, 6)
        self.assertEqual(self.client.debug_step_in(), bp)
        bp = (self.CLNAME2, 7)
        self.assertEqual(self.client.debug_run([bp]), bp)
        bp = (self.CLNAME2, 8)
        self.assertEqual(self.client.debug_step_in(), bp)
        bp = (self.CLNAME2, 9)
        self.assertEqual(self.client.debug_step_in(), bp)
        bp = (self.CLNAME2, 10)
        self.assertEqual(self.client.debug_step_in(), bp)
        self.client.debug_run_finish()
    
    def test_start_with_step_in(self):
    #
    # starting with step-in
    #
        from testlib.clientsimulator import SimulatorError
        self.client.execute_debuggee(
            hostprog_name='ndrange_inout',
            cl_name=self.CLNAME2)
        self.client.connect_to_server()
        self.client.start_session(0, 0, 0)
        
        # starting with step_in is supported
        self.assertEqual(self.client.debug_step_in(), (self.CLNAME2, 1))
        self.client.debug_run_finish()

    def test_simple_step_over2(self):
    #
    # TC-15, TC-26
    # test the usage of steps over, first command in program is a function calls
    #
    
        self.client.execute_debuggee(
            hostprog_name='ndrange_inout',
            cl_name=self.CLNAME3)
        self.client.connect_to_server()
        self.client.start_session(0, 0, 0)
        bp = (self.CLNAME3, 11)
        self.assertEqual(self.client.debug_run([bp]), bp)
        bp = (self.CLNAME3, 12)
        self.assertEqual(self.client.debug_step_over(), bp)
        self.client.get_stack_trace()
        self.assertEqual(self.client.stack_query_func_name(0), 'main_kernel')
        self.client.debug_run_finish()

    def test_step_over_no_function_calls(self):
    # 
    # TC-16
    # test the usage of steps over, of a program with no function calls
    #
        self.client.execute_debuggee(
            hostprog_name='ndrange_inout',
            cl_name=self.CLNAME2)
        self.client.connect_to_server()
        self.client.start_session(0, 0, 0)
        # add some breakpoints and go to them with the step over mechanism
        bp = (self.CLNAME2, 3)
        bps = [bp, (self.CLNAME2, 4), (self.CLNAME2, 5), (self.CLNAME2, 6)]
        self.assertEqual(self.client.debug_run(bps), bp)
        bp = (self.CLNAME2, 4)
        self.assertEqual(self.client.debug_step_over(), bp)
        bp = (self.CLNAME2, 5)
        self.assertEqual(self.client.debug_step_over(), bp)
        bp = (self.CLNAME2, 6)
        self.assertEqual(self.client.debug_step_over(), bp)
        bp = (self.CLNAME2, 5)
        self.assertEqual(self.client.debug_step_over(), bp)
        bp = (self.CLNAME2, 6)
        self.assertEqual(self.client.debug_step_over(), bp)
        bp = (self.CLNAME2, 7)
        self.assertEqual(self.client.debug_run([bp]), bp)
        bp = (self.CLNAME2, 8)
        self.assertEqual(self.client.debug_step_over(), bp)
        bp = (self.CLNAME2, 9)
        self.assertEqual(self.client.debug_step_over(), bp)
        bp = (self.CLNAME2, 10)
        self.assertEqual(self.client.debug_step_over(), bp)
        self.client.debug_run_finish()

    def test_step_in_first_command_is_function_call(self):
    #
    # TC-13, TC-28
    # test the usage of step in, in a program with a function call at its first line, and some nested
    # function calls. test also the usage of get trace after sending a step in command
        self.client.execute_debuggee(
            hostprog_name='ndrange_inout',
            cl_name=self.CLNAME1)
        self.client.connect_to_server()
        self.client.start_session(0, 0, 0)
        bp = (self.CLNAME1, 27)
        self.assertEqual(self.client.debug_run([bp]), bp)
        bp = (self.CLNAME1, 19)
        self.assertEqual(self.client.debug_step_in(), bp)
        bp = (self.CLNAME1, 20)
        self.assertEqual(self.client.debug_step_in(), bp)
        bp = (self.CLNAME1, 21)
        self.assertEqual(self.client.debug_step_in(), bp)
        bp = (self.CLNAME1, 28)
        self.assertEqual(self.client.debug_step_in(), bp)
        bp = (self.CLNAME1, 29)
        self.assertEqual(self.client.debug_step_in(), bp)
        bp = (self.CLNAME1, 3)
        self.assertEqual(self.client.debug_step_in(), bp)
        bp = (self.CLNAME1, 4)
        self.assertEqual(self.client.debug_step_in(), bp)
        bp = (self.CLNAME1, 5)
        self.assertEqual(self.client.debug_step_in(), bp)
        bp = (self.CLNAME1, 11)
        self.assertEqual(self.client.debug_step_in(), bp)
        bp = (self.CLNAME1, 12)
        self.assertEqual(self.client.debug_step_in(), bp)
        bp = (self.CLNAME1, 13)
        self.assertEqual(self.client.debug_step_in(), bp)
        bp = (self.CLNAME1, 19)
        self.assertEqual(self.client.debug_step_in(), bp)
        bp = (self.CLNAME1, 20)
        self.assertEqual(self.client.debug_step_in(), bp)
        # in inner function, get the trace
        self.client.get_stack_trace()
        self.assertEqual(self.client.stack_query_func_name(0), 'test3')
        self.assertEqual(self.client.stack_query_func_name(1), 'test2')
        self.assertEqual(self.client.stack_query_func_name(2), 'test1')
        self.assertEqual(self.client.stack_query_func_name(3), 'main_kernel')
        bp = (self.CLNAME1, 19)
        self.assertEqual(self.client.debug_run([bp]), bp)
        self.client.get_stack_trace()
        # get trace after break point
        self.assertEqual(self.client.stack_query_func_name(0), 'test3')
        self.assertEqual(self.client.stack_query_func_name(1), 'test2')
        self.assertEqual(self.client.stack_query_func_name(2), 'main_kernel')
        bp = (self.CLNAME1, 20)
        self.assertEqual(self.client.debug_step_in(), bp)
        bp = (self.CLNAME1, 21)
        self.assertEqual(self.client.debug_step_in(), bp)
        bp = (self.CLNAME1, 14)
        self.assertEqual(self.client.debug_step_in(), bp)
        bp = (self.CLNAME1, 31)
        self.assertEqual(self.client.debug_step_in(), bp)
        bp = (self.CLNAME1, 33)
        self.assertEqual(self.client.debug_run([bp]), bp)
        bp = (self.CLNAME1, 34)
        self.assertEqual(self.client.debug_step_in(), bp)
        self.client.debug_run_finish()

    def test_step_over_first_command_is_function_call(self):
    #
    # TC-17, TC-29
    # test the usage of step over throw the whole program, in a program with a function call at its first line, and some nested
    # function calls. test also the usage of get trace after sending a step over command
        self.client.execute_debuggee(
            hostprog_name='ndrange_inout',
            cl_name=self.CLNAME1)
        self.client.connect_to_server()
        self.client.start_session(0, 0, 0)
        bp = (self.CLNAME1, 27)
        self.assertEqual(self.client.debug_run([bp]), bp)
        bp = (self.CLNAME1, 28)
        self.assertEqual(self.client.debug_step_over(), bp)
        self.client.get_stack_trace()
        self.assertEqual(self.client.stack_query_func_name(0), 'main_kernel')
        bp = (self.CLNAME1, 29)
        self.assertEqual(self.client.debug_step_over(), bp)
        bp = (self.CLNAME1, 30)
        self.assertEqual(self.client.debug_step_over(), bp)
        bp = (self.CLNAME1, 31)
        self.assertEqual(self.client.debug_step_over(), bp)
        bp = (self.CLNAME1, 32)
        self.assertEqual(self.client.debug_step_over(), bp)
        bp = (self.CLNAME1, 33)
        self.assertEqual(self.client.debug_step_over(), bp)
        bp = (self.CLNAME1, 35)
        self.client.debug_run_finish()

    def test_step_over_function_call_in_function(self):
    #
    #
    # TC-18, TC-19
    # test the usage of step over in function code, in a program with a function call at its first line, and some nested
    # function calls. also trying to step over a breakpoint
        self.client.execute_debuggee(
            hostprog_name='ndrange_inout',
            cl_name=self.CLNAME1)
        self.client.connect_to_server()
        self.client.start_session(0, 0, 0)
        bp = (self.CLNAME1, 3)
        self.assertEqual(self.client.debug_run([bp]), bp)
        bp = (self.CLNAME1, 4)
        self.assertEqual(self.client.debug_step_over(), bp)
        bp = (self.CLNAME1, 5)
        self.assertEqual(self.client.debug_step_over(), bp)
        bp = (self.CLNAME1, 6)
        self.assertEqual(self.client.debug_step_over(), bp)
        bp = (self.CLNAME1, 30)
        self.assertEqual(self.client.debug_step_over(), bp)
        bp = (self.CLNAME1, 13)
        bps = [bp, (self.CLNAME1, 20)]
        self.assertEqual(self.client.debug_run(bps), bp)
        bp = (self.CLNAME1, 20)
        # try to step over a breakpoint 
        self.assertEqual(self.client.debug_step_over(), bp)
        self.client.debug_run_finish()


    def test_simple_stack_trace(self):
    #
    # TC-27
    # test the usage of step out, first command in program is a function calls
    #
    
        self.client.execute_debuggee(
            hostprog_name='ndrange_inout',
            cl_name=self.CLNAME3)
        self.client.connect_to_server()
        self.client.start_session(0, 0, 0)
        # add some breakpoints and go to them with the step in mechanism
        bp = (self.CLNAME3, 3)
        self.assertEqual(self.client.debug_run([bp]), bp)        
        self.client.get_stack_trace()
        self.assertEqual(self.client.stack_query_func_name(0), 'foo')
        self.assertEqual(self.client.stack_query_func_name(1), 'main_kernel')
        self.client.debug_run_finish()

        
    def test_simple_step_out(self):
    #
    # TC-24
    # test the usage of step out, first command in program is a function calls
    #
    
        self.client.execute_debuggee(
            hostprog_name='ndrange_inout',
            cl_name=self.CLNAME3)
        self.client.connect_to_server()
        self.client.start_session(0, 0, 0)
        # add some breakpoints and go to them with the step in mechanism
        bp = (self.CLNAME3, 3)
        self.assertEqual(self.client.debug_run([bp]), bp)        
        bp = (self.CLNAME3, 10)
        self.assertEqual(self.client.debug_step_out(), bp)
        self.client.debug_run_finish()
     
       
    def test_step_out(self):
    #
    # TC-22, TC-23, TC-24, TC-30
    # test the usage of step out, in a program with a function call at its first line, and some nested
    # function calls.
    # trying to step out a breakpoint
    # getting trace after sending step out command and after breakpoint
    #
        self.client.execute_debuggee(
            hostprog_name='ndrange_inout',
            cl_name=self.CLNAME1)
        self.client.connect_to_server()
        self.client.start_session(0, 0, 0)
        bp = (self.CLNAME1, 29)
        self.assertEqual(self.client.debug_run([bp]), bp)
        bp = (self.CLNAME1, 19)
        self.assertEqual(self.client.debug_run([bp]), bp)
        # in inner function
        bp = (self.CLNAME1, 14)
        self.assertEqual(self.client.debug_step_out(), bp)
        bp = (self.CLNAME1, 6)
        self.assertEqual(self.client.debug_step_out(), bp)
        bp = (self.CLNAME1, 30)
        self.assertEqual(self.client.debug_step_out(), bp)
        # back in main, get trace
        self.client.get_stack_trace()
        self.assertEqual(self.client.stack_query_func_name(0), 'main_kernel')
        bp = (self.CLNAME1, 19)
        bps = [bp, (self.CLNAME1, 20)]
        self.assertEqual(self.client.debug_run(bps), bp)
        # after reaching breakpoint in inner function again, get trace 
        self.client.get_stack_trace()
        self.assertEqual(self.client.stack_query_func_name(0), 'test3')
        self.assertEqual(self.client.stack_query_func_name(1), 'test2')
        self.assertEqual(self.client.stack_query_func_name(2), 'main_kernel')
        # try to step out a breakpoint
        bp = (self.CLNAME1, 20)
        self.assertEqual(self.client.debug_step_out(), bp)
        self.client.debug_run_finish()
