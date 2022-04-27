from testlib.debuggertestcase import DebuggerTestCase


class TypeNames(DebuggerTestCase):
    CLNAME = 'type_names.cl'

    def _assert_typename(self, varname, typename):
        self.assertEqual(self.client.var_query_type(varname), typename)

    def test_type_names(self):
        self.client.execute_debuggee(
            hostprog_name='ndrange_inout',
            cl_name=self.CLNAME)
        self.client.connect_to_server()
        self.client.start_session(0, 0, 0)

        bps = [(self.CLNAME, 42)]

        file, line = self.client.debug_run(bps)
        self.assertEqual((file, line), bps[0])

        self._assert_typename('bb', 'bool')
        self._assert_typename('ss', 'short')
        self._assert_typename('ii', 'int')

        self._assert_typename('uss', 'unsigned short')
        self._assert_typename('uii', 'unsigned int')

        if self.use_gdb:
            self._assert_typename('ll', 'long')
            self._assert_typename('ull', 'unsigned long')
        elif self.use_cdb:
            self._assert_typename('ll', '__int64')
            self._assert_typename('ull', 'unsigned __int64')
        else:
            self._assert_typename('ll', 'long int')
            self._assert_typename('ull', 'long unsigned int')

        if self.use_cdb:
            self._assert_typename('gid', 'unsigned __int64') # TODO: 32 bit support
            self._assert_typename('somestruct', 'main_kernel::SomeStruct')
            self._assert_typename('anonstruct', 'main_kernel::<unnamed-tag>')
            self._assert_typename('ff4', 'float [4]')
            self._assert_typename('dd2', 'double [2]')
            self._assert_typename('cc3', 'char [3]')
            self._assert_typename('ss16', 'short [16]')
            self._assert_typename('buf_in', 'unsigned char *')
            self._assert_typename('buf_out', 'unsigned char *')
            self._assert_typename('ppuii', 'unsigned int * *')
            self._assert_typename('uuaa', 'unsigned int [3][4]')
            self._assert_typename('auarr', 'unsigned int [4]')
        else:
            self._assert_typename('gid', 'size_t')
            self._assert_typename('somestruct', 'struct SomeStruct')
            self._assert_typename('anonstruct', 'struct <unnamed>')
            self._assert_typename('ff4', 'float4')
            self._assert_typename('dd2', 'double2')
            self._assert_typename('cc3', 'char3')
            self._assert_typename('ss16', 'short16')
            self._assert_typename('buf_in', 'uchar*')
            self._assert_typename('buf_out', 'uchar*')
            self._assert_typename('ppuii', 'uint**')
            self._assert_typename('uuaa', 'uint[3][4]')
            self._assert_typename('auarr', 'uint[4]')

        self.client.debug_run_finish(bps)
