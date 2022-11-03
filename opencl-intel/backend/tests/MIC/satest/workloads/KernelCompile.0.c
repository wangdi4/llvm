__kernel void Triad(__global const float *memA, __global const float *memB,
                    __global float *memC) {
  int gid = get_global_id(0);
  memC[gid] = memA[gid] + memB[gid];
}
