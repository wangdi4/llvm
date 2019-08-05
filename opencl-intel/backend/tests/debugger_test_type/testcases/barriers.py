from testlib.debuggertestcase import DebuggerTestCase, expectedFailureCDB


class Barriers(DebuggerTestCase):

    def test_simple_barrier(self):
        # Tests a kernel with a simple barrier that doesn't do anything
        # interesting.
        CLNAME = 'barriers1.cl'
        self.client.execute_debuggee(
            hostprog_name='ndrange_inout',
            cl_name=CLNAME)
        self.client.connect_to_server()
        self.client.start_session(0, 0, 0)

        bps = [(CLNAME, 4), (CLNAME, 7)]

        # reach first BP
        self.assertEqual(self.client.debug_run(bps), bps[0])
        self.assertEqual(self.client.var_query_value('ii'), '170')
        self.assertEqual(self.client.var_query_value('jj'), '400')

        # second BP
        self.assertEqual(self.client.debug_run(bps), bps[1])
        self.assertEqual(self.client.var_query_value('ii'), '200')
        self.assertEqual(self.client.var_query_value('jj'), '400')

        self.client.debug_run_finish(bps)

    def test_barrier_atomic_add(self):
        # Tests a kernel that uses a barrier to ensure all atomic adds were
        # completed.
        # Set both global and local work sizes to 32.
        # 32 work items in a single work-group
        #
        CLNAME = 'barriers2.cl'
        self.client.execute_debuggee(
            hostprog_name='ndrange_inout',
            cl_name=CLNAME,
            extra_args=['1024', '1', '32', '32'])
        self.client.connect_to_server()
        self.client.start_session(1, 0, 0)

        bps = [(CLNAME, 18), (CLNAME, 30)]

        # reach first BP
        self.assertEqual(self.client.debug_run(bps), bps[0])
        self.assertEqual(self.client.var_query_value('globsize'), '32')
        self.assertEqual(self.client.var_query_value('locsize'), '32')
        self.assertEqual(self.client.var_query_value('numgroups'), '1')
        self.assertEqual(self.client.var_query_value('val1'), '517')

        # second BP
        # all 32 WIs have incremented the value
        self.assertEqual(self.client.debug_run(bps), bps[1])
        self.assertEqual(self.client.var_query_value('val2'), '549')

        self.client.debug_run_finish(bps)

    def test_barrier_in_if_and_for(self):
        # Tests a barrier call inside an 'if' and 'for' statement
        #
        # Set both global and local work sizes to 32.
        # 32 work items in a single work-group
        #
        CLNAME = 'barriers3.cl'
        self.client.execute_debuggee(
            hostprog_name='ndrange_inout',
            cl_name=CLNAME,
            extra_args=['1024', '1', '32', '32'])
        self.client.connect_to_server()
        self.client.start_session(1, 0, 0)

        bps = [(CLNAME, 31)]
        self.assertEqual(self.client.debug_run(bps), bps[0])
        self.assertEqual(self.client.var_query_value('val2'), '65192')
        self.client.debug_run_finish(bps)

    def test_barriers_consecutive(self):
        # Tests consecutive barrier calls
        #
        CLNAME = 'barriers4.cl'
        self.client.execute_debuggee(
            hostprog_name='ndrange_inout',
            cl_name=CLNAME)
        self.client.connect_to_server()
        self.client.start_session(1, 0, 0)

        # check we stop on all BPs in order
        bps = [(CLNAME, lineno) for lineno in (4, 5, 6, 8)]
        self.assertEqual(self.client.debug_run(bps), bps[0])
        self.assertEqual(self.client.debug_run(bps), bps[1])
        self.assertEqual(self.client.debug_run(bps), bps[2])
        self.assertEqual(self.client.debug_run(bps), bps[3])

        # sanity check
        self.assertEqual(self.client.var_query_value('b'), '13')
        self.client.debug_run_finish(bps)

    def test_barrier_in_function(self):
        # Tests a barrier call inside a function
        #
        # Set both global and local work sizes to 32.
        # 32 work items in a single work-group
        #
        CLNAME = 'barriers5.cl'
        self.client.execute_debuggee(
            hostprog_name='ndrange_inout',
            cl_name=CLNAME,
            extra_args=['1024', '1', '32', '32'])
        self.client.connect_to_server()
        self.client.start_session(1, 0, 0)

        # just make sure we can stop inside do_add
        bp = (CLNAME, 5)
        self.assertEqual(self.client.debug_run([bp]), bp)

        # ... and we can stop inside add_twice
        bp = (CLNAME, 11)
        self.assertEqual(self.client.debug_run([bp]), bp)

        # check result: 32 added twice to 517 is 581
        bps = [(CLNAME, 27)]
        self.assertEqual(self.client.debug_run(bps), bps[0])
        self.assertEqual(self.client.var_query_value('val2'), '581')
        self.client.debug_run_finish(bps)
