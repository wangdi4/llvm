void setLocalVal(__local int *p_local, int val) { *p_local = val; }

void addLocal2(int *sum, __local int *p_local2) {
  if (get_local_id(0) == 0)
    setLocalVal(p_local2, 66);

  // Make sure local value is valid for all work-item in this work-group.
  barrier(CLK_LOCAL_MEM_FENCE);
  *sum += *p_local2;
}

void addLocal1(int *sum, __local int *p_local1, __local int *p_local2) {
  if (get_local_id(0) == 0)
    setLocalVal(p_local1, 33);

  // Make sure local value is valid for all work-item in this work-group.
  barrier(CLK_LOCAL_MEM_FENCE);
  *sum += *p_local1;

  addLocal2(sum, p_local2);
}

__kernel void test_local_buffers(__global int *result) {
  // Declaration
  __local int v_local1;
  __local int v_local2;

  // Addressing
  __local int *p_local1 = &v_local1;
  __local int *p_local2 = &v_local2;

  // Calculate (v_local1 + v_local2)
  int sum = 0;
  addLocal1(&sum, p_local1, p_local2);

  // Verify
  result[get_global_id(0)] = (sum == 99);
}

/* This kernel is used to confirm that the local buffers work well when
 * the local values passed to the same function have different offset in the
 * callers. */
__kernel void dummy(__global int *result) {
  __local int v_local;
  __local int *p_local;
  int sum = 0;
  addLocal2(&sum, p_local);
  result[get_global_id(0)] = (sum == 66);
}
