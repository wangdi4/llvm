__kernel void test_fn(__global ulong *src, __global ulong *dst) {
  __local ulong a[100];
  int j = 15;
#pragma nounroll
  for (int i = 0; i < 64; i++) // Checks SYCLAliasAnalysis takes effect,
    a[i] = src[j];             // and this src[j] is moved outside the loop.
#pragma nounroll
  for (int i = 0; i < 64; i++)
    dst[i] = a[i];
}
