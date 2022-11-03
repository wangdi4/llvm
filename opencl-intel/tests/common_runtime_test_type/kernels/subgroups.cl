#pragma OPENCL EXTENSION cl_khr_subgroups : enable

kernel void sub_groups_main(global uint *sub_group_size,
                            global uint *max_sub_group_size,
                            global uint *num_sub_groups) {
  size_t id = get_global_id(0);
  sub_group_size[id] = get_sub_group_size();
  max_sub_group_size[id] = get_max_sub_group_size();
  num_sub_groups[id] = get_num_sub_groups();
}
