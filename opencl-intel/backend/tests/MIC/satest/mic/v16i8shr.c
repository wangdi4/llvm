// RUN: pwd
// RUN: clang -cc1 -cl-kernel-arg-info -emit-llvm-bc -x cl -I ../../../../../../../src/backend/clang_headers/ \
// RUN: -include opencl_.h -o %s.bc %s
// RUN: python ../../bin/SATest.py -config=%s.cfg.xml -tsize=1
// XFAIL: win32

__kernel void test(__global char16* in)
{
    int id = get_global_id(0);
    in[id] = in[id]*in[id];
}

