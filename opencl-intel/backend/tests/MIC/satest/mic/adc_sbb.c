// RUN: pwd
// RUN: clang -cc1 -cl-kernel-arg-info -emit-llvm-bc -x cl -I ../../../../../../../src/backend/clang_headers/ \
// RUN: -include opencl_.h -o %s.bc %s
// RUN: python ../../bin/SATest.py -config=%s.cfg.xml -tsize=1

__kernel void A(__global uint *out, __global const uint *a, __global const uint *b) {
    unsigned i = get_global_id(0);
    out[i] = (a[i]!=0)+b[i];
}

__kernel void B(__global uint *out, __global const uint *a, __global const uint *b) {
    unsigned i = get_global_id(0);
    out[i] = (a[i]==0)+b[i];
}

__kernel void C(__global uint *out, __global const uint *a, __global const uint *b) {
    unsigned i = get_global_id(0);
    out[i] = b[i]-(a[i]!=0);
}

__kernel void D(__global uint *out, __global const uint *a, __global const uint *b) {
    unsigned i = get_global_id(0);
    out[i] = b[i]-(a[i]!=0);
}

__kernel void E(__global uint *out, __global const uint *a, __global const uint *b) {
    unsigned i = get_global_id(0);
    out[i] = b[i]-(a[i]==0);
}


