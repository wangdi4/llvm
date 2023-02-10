__kernel void test(__global short *x) {
  size_t gid = get_global_id(0);
  // Tells the compiler to assume gid fits in i32 range.
  __builtin_assume(gid < 65535);
  // The actual dimension size is 8.
  if (gid >= 8)
    return;

  // y is aligned to 2-byte boundary, which would trigger dynamic loop peeling.
  // The dynamic loop peel size determined by the formula would be 15 in this
  // case, while the actual dimension size is only 8.
  // We need to make sure the actual loop peel size <= actual dimension size,
  // otherwise a negative vector loop size (8 - 15 == -7) will lead to segment
  // fault (accessing out-of-bound memory).
  __global short *y = x + 1;
  y[gid] = (short)gid;
}
