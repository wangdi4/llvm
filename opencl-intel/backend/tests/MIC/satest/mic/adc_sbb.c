
__kernel void A(__global uint *out, __global const uint *a,
                __global const uint *b) {
  unsigned i = get_global_id(0);
  out[i] = (a[i] != 0) + b[i];
}

__kernel void B(__global uint *out, __global const uint *a,
                __global const uint *b) {
  unsigned i = get_global_id(0);
  out[i] = (a[i] == 0) + b[i];
}

__kernel void C(__global uint *out, __global const uint *a,
                __global const uint *b) {
  unsigned i = get_global_id(0);
  out[i] = b[i] - (a[i] != 0);
}

__kernel void D(__global uint *out, __global const uint *a,
                __global const uint *b) {
  unsigned i = get_global_id(0);
  out[i] = b[i] - (a[i] != 0);
}

__kernel void E(__global uint *out, __global const uint *a,
                __global const uint *b) {
  unsigned i = get_global_id(0);
  out[i] = b[i] - (a[i] == 0);
}
