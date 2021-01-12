from testlib.debuggertestcase import DebuggerTestCase, skipNotGDB

class SGEmulations(DebuggerTestCase):

    # This is coupled with the 'intel_reqd_sub_group_size' attribute in .cl sources
    sg_size = 16

    def __init__(self, *args, **kwargs):
        super(SGEmulations, self).__init__(*args, **kwargs)

        self.data_size = 1024
        self.ndim = 1
        self.global_sizes = [32]
        self.local_sizes = [32]
        self.offsets = [0]

        def stringify(arg):
            if isinstance(arg, str):
                return arg

            if isinstance(arg, list):
                return ",".join(map(str, arg))

            return str(arg)

        # extra args for ndrange_inout program
        self.extra_args = [
            stringify(self.data_size),
            stringify(self.ndim),
            stringify(self.global_sizes),
            stringify(self.local_sizes),
            stringify(self.offsets)
        ]

        self.options = {'build_opts': '-cl-std=CL2.0'}

    def _launch(self, cl_name, **kwargs):
        hostprog_name = kwargs.get('hostprog_name', 'ndrange_inout')
        extra_args = kwargs.get('extra_args', self.extra_args)
        workitem_ids = kwargs.get('workitem_ids', (0, 0, 0))
        options = kwargs.get('options', self.options)

        self.client.execute_debuggee(
            hostprog_name=hostprog_name,
            cl_name=cl_name,
            extra_args=extra_args,
            options=options
        )
        self.client.connect_to_server()
        self.client.start_session(*workitem_ids)

    @skipNotGDB
    def test_break_workitem(self):
        """
        Test whether we could break on arbitrary workitem with subgroup emulation enabled
        """

        CLNAME = 'sg_emu_basic.cl'

        # (3,0,0) in the first subgroup
        ids = (3, 0, 0)
        self._launch(CLNAME, workitem_ids=ids)

        bp = (CLNAME, 5)    # x = sub_group_scan_inclusive_add(a)
        self.assertEqual(self.client.debug_run([bp]), bp)
        self.assertEqual(self.client.var_query_value('a'), str(ids[0]))

    @skipNotGDB
    def test_basic(self):
        """
        Test values during basic subgroup emulation
        """

        CLNAME = 'sg_emu_basic.cl'
        self._launch(CLNAME)

        bp = (CLNAME, 5)
        offset = 16

        # emulation loop before sub_group_scan_inclusive_add()
        psum = []
        s = 0
        for i in range(self.sg_size):
            self.client.relocate_workitem(i+offset, 0, 0)
            self.assertEqual(self.client.debug_run([bp]), bp)
            self.assertEqual(self.client.var_query_value('a'), str(i+offset))
            s += i+offset
            psum.append(s)

        # emulation loop after sub_group_scan_inclusive_add()
        bp = (CLNAME, 6)
        for i in range(self.sg_size):
            self.client.relocate_workitem(i+offset, 0, 0)
            self.assertEqual(self.client.debug_run([bp]), bp)
            self.assertEqual(self.client.var_query_value('x'), str(psum[i]))

    @skipNotGDB
    def test_basic_with_barrier(self):
        """
        Test kernel with barrier & subgroup builtins
        """

        CLNAME = 'sg_emu_basic_with_barrier.cl'
        self._launch(CLNAME)

        bps = [(CLNAME, 4), (CLNAME, 7)]
        for i in range(self.local_sizes[0]):
            self.client.relocate_workitem(i, 0, 0)
            self.assertEqual(self.client.debug_run(bps), bps[0])
            self.assertEqual(self.client.var_query_value('lid'), str(i))

        for i in range(self.local_sizes[0]):
            self.client.relocate_workitem(i, 0, 0)
            self.assertEqual(self.client.debug_run(bps), bps[1])
            self.assertEqual(self.client.var_query_value('a'), str(i))

    @skipNotGDB
    def test_basic_with_mask(self):
        """
        Test kernel of irregular group size (not multiple of sg_size)
        """

        wg_size = 19
        CLNAME = 'sg_emu_basic.cl'
        self._launch(CLNAME, extra_args=['1024', '1', str(wg_size), str(wg_size)])

        bps = [(CLNAME, 6)]
        s = 0
        psum = []
        for i in range(wg_size):
            if i % self.sg_size == 0:
                s = 0
            s += i
            psum.append(s)

        for i in range(wg_size):
            self.client.relocate_workitem(i, 0, 0)
            self.assertEqual(self.client.debug_run(bps), bps[0])
            self.assertEqual(self.client.var_query_value('x'), str(psum[i]))

        self.client.debug_run_finish()

    @skipNotGDB
    def test_basic_with_workgroup_call(self):
        """
        Test kernel with workgroup builtins
        """

        CLNAME = 'sg_emu_basic_with_wgcall.cl'
        self._launch(CLNAME)

        bps = [(CLNAME, 4), (CLNAME, 6)]
        rsum = sum(range(self.local_sizes[0]))
        for i in range(self.local_sizes[0]):
            self.client.relocate_workitem(i, 0, 0)
            self.assertEqual(self.client.debug_run(bps), bps[0])
            self.assertEqual(self.client.var_query_value('lid'), str(i))

        for i in range(self.sg_size):
            self.client.relocate_workitem(i, 0, 0)
            self.assertEqual(self.client.debug_run(bps), bps[1])
            self.assertEqual(self.client.var_query_value('x'), str(rsum))
            self.assertEqual(self.client.var_query_value('y'), str(rsum * (i+1)))

    @skipNotGDB
    def test_subroutine(self):
        """
        Test kernel calling sub-functions
        """
        CLNAME = 'sg_emu_subroutine.cl'
        self._launch(CLNAME)

        bps = [(CLNAME, 10), (CLNAME, 2), (CLNAME, 11)]
        # int b = foo(a)
        for i in range(self.sg_size):
            self.client.relocate_workitem(i, 0, 0)
            self.assertEqual(self.client.debug_run(bps), bps[0])
            self.assertEqual(self.client.var_query_value('a'), str(i))

        # int y = sub_group_scan_inclusive_add(x)
        for i in range(self.sg_size):
            self.client.relocate_workitem(i, 0, 0)
            self.assertEqual(self.client.debug_run(bps), bps[1])
            self.assertEqual(self.client.var_query_value('x'), str(i))

        # buf_out[lid] = convert_uchar(b)
        s = 0
        for i in range(self.sg_size):
            s += i
            self.client.relocate_workitem(i, 0, 0)
            self.assertEqual(self.client.debug_run(bps), bps[2])
            self.assertEqual(self.client.var_query_value('b'), str(s))

    @skipNotGDB
    def test_subroutine_with_pointer_arg(self):
        """
        Test kernel calling sub-function with pointer args
        """
        CLNAME = 'sg_emu_subroutine_with_pointer_arg.cl'
        self._launch(CLNAME)

        bps = [(CLNAME, 11), (CLNAME, 2), (CLNAME, 12)]
        # int b = foo(a)
        for i in range(self.sg_size):
            self.client.relocate_workitem(i, 0, 0)
            self.assertEqual(self.client.debug_run(bps), bps[0])
            self.assertEqual(self.client.var_query_value('a'), str(i))

        # int y = sub_group_scan_inclusive_add(x)
        for i in range(self.sg_size):
            self.client.relocate_workitem(i, 0, 0)
            self.assertEqual(self.client.debug_run(bps), bps[1])
            self.assertEqual(self.client.var_query_value('*x'), str(i))

        # buf_out[lid] = convert_uchar(b)
        s = 0
        for i in range(self.sg_size):
            s += i
            self.client.relocate_workitem(i, 0, 0)
            self.assertEqual(self.client.debug_run(bps), bps[2])
            self.assertEqual(self.client.var_query_value('b'), str(s))

    @skipNotGDB
    def test_subroutine_with_implicit_pointer_arg(self):
        """
        Test kernel calling sub-function with pointer args
        """
        CLNAME = 'sg_emu_subroutine_with_implicit_pointer_arg.cl'
        self._launch(CLNAME)

        bps = [(CLNAME, 11), (CLNAME, 2), (CLNAME, 12)]
        # int b = foo(a)
        for i in range(self.sg_size):
            self.client.relocate_workitem(i, 0, 0)
            self.assertEqual(self.client.debug_run(bps), bps[0])
            self.assertEqual(self.client.var_query_value('a'), str(i))

        # int y = sub_group_scan_inclusive_add(x)
        for i in range(self.sg_size):
            self.client.relocate_workitem(i, 0, 0)
            self.assertEqual(self.client.debug_run(bps), bps[1])
            self.assertEqual(self.client.var_query_value('*x'), str(i))

        # buf_out[lid] = convert_uchar(b)
        s = 0
        for i in range(self.sg_size):
            s += i
            self.client.relocate_workitem(i, 0, 0)
            self.assertEqual(self.client.debug_run(bps), bps[2])
            self.assertEqual(self.client.var_query_value('b'), str(s))

    @skipNotGDB
    def test_subroutine_with_barrier(self):
        """
        Test subroutine with barrier & subgroup builtins
        """

        CLNAME = 'sg_emu_subroutine_with_barrier.cl'
        self._launch(CLNAME)

        bps = [(CLNAME, 13), (CLNAME, 3), (CLNAME, 6)]
        # int b = foo(a)
        for i in range(self.local_sizes[0]):
            self.client.relocate_workitem(i, 0, 0)
            self.assertEqual(self.client.debug_run(bps), bps[0])
            self.assertEqual(self.client.var_query_value('a'), str(i))

        # int gid = get_global_id(0)
        for i in range(self.local_sizes[0]):
            self.client.relocate_workitem(i, 0, 0)
            self.assertEqual(self.client.debug_run(bps), bps[1])
            self.assertEqual(self.client.var_query_value('lid'), str(i))

        # return y
        s = 0
        for i in range(self.local_sizes[0]):
            if i % self.sg_size == 0:
                s = 0
            s += i
            self.client.relocate_workitem(i, 0, 0)
            self.assertEqual(self.client.debug_run(bps), bps[2])
            self.assertEqual(self.client.var_query_value('y'), str(s))

    @skipNotGDB
    def test_subroutine_with_wgcall(self):
        """
        Test subroutine with workgroup & subgroup builtins
        """

        CLNAME = 'sg_emu_subroutine_with_wgcall.cl'
        self._launch(CLNAME)

        bps = [(CLNAME, 10), (CLNAME, 2), (CLNAME, 3), (CLNAME, 12)]
        # int b = foo(a)
        for i in range(self.local_sizes[0]):
            self.client.relocate_workitem(i, 0, 0)
            self.assertEqual(self.client.debug_run(bps), bps[0])
            self.assertEqual(self.client.var_query_value('a'), str(i))

        # int y = work_group_reduce_add(x)
        for i in range(self.local_sizes[0]):
            self.client.relocate_workitem(i, 0, 0)
            self.assertEqual(self.client.debug_run(bps), bps[1])
            self.assertEqual(self.client.var_query_value('x'), str(i))

        # return y
        rsum = sum(range(self.local_sizes[0]))
        for i in range(self.local_sizes[0]):
            self.client.relocate_workitem(i, 0, 0)
            self.assertEqual(self.client.debug_run(bps), bps[2])
            self.assertEqual(self.client.var_query_value('y'), str(rsum))

        # buf_out[lid] = convert_uchar(c);
        s = 0
        for i in range(self.local_sizes[0]):
            if i % self.sg_size == 0:
                s = 0
            s += rsum
            self.client.relocate_workitem(i, 0, 0)
            self.assertEqual(self.client.debug_run(bps), bps[3])
            self.assertEqual(self.client.var_query_value('c'), str(s))
