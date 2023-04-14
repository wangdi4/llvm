#pragma OPENCL EXTENSION cl_intel_subgroups : enable
__kernel void test() {}

void func() {
  get_sub_group_size();
  get_max_sub_group_size();
  get_num_sub_groups();
  get_enqueued_num_sub_groups();
  get_sub_group_id();
  get_sub_group_local_id();
  sub_group_barrier(CLK_LOCAL_MEM_FENCE);
  sub_group_barrier(CLK_LOCAL_MEM_FENCE, memory_scope_sub_group);
}
