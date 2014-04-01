// Test for get_local_size in SATest OpenCL Reference
__kernel void test_get_local_size(__global int4* a)
{
    size_t linear_gid = get_global_linear_id();

    a[linear_gid].x = get_local_size(0);
    a[linear_gid].y = get_local_size(1);
    a[linear_gid].z = get_local_size(2);
}
