#pragma OPENCL EXTENSION cl_khr_subgroups: enable

__attribute__((overloadable)) uint get_sub_group_size();
__attribute__((overloadable)) uint get_max_sub_group_size();
__attribute__((overloadable)) uint get_num_sub_groups();

kernel void sub_groups_main(global uint* sub_group_size, global uint* max_sub_group_size, global uint* num_sub_groups)
{
    size_t id = get_global_id(0);
    sub_group_size[id] = get_sub_group_size();
    max_sub_group_size[id] = get_max_sub_group_size();
    num_sub_groups[id] = get_num_sub_groups();
}
