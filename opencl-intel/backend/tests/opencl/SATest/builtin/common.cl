kernel void test(global TYPE *a, global TYPE *b, global TYPE *c,
                 global TYPE *dst) {
  size_t i = get_global_id(0);
  dst[i] = clamp(a[i], b[i], c[i]) + degrees(a[i]) + max(a[i], b[i]) +
           min(a[i], b[i]) + mix(a[i], b[i], c[i]) + radians(a[i]) +
           step(a[i], b[i]) + smoothstep(a[i], b[i], c[i]) + sign(a[i]);
#ifdef MASKED
  // Add subgroup call in order to enable masked vectorized kernel.
  dst[i] += get_sub_group_size();
#endif
}
