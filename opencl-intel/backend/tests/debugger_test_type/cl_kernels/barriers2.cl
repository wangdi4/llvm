__kernel void main_kernel(__global uchar* buf_in, __global uchar* buf_out)
{
    size_t globsize = get_global_size(0);
    size_t locsize = get_local_size(0);
    size_t numgroups = get_num_groups(0);

    // Initialize once (to 517)
    if (get_global_id(0) == 20) {
        buf_out[0] = 5;
        buf_out[1] = 2;
        buf_out[2] = 0;
        buf_out[3] = 0;
    }

    // Make sure initialization happened before we go on
    barrier(CLK_LOCAL_MEM_FENCE);
    uint val1 = *((__global uint*) &buf_out[0]);
    int dummy = 128; // we'll stop here being sure no one has incremented yet
                     // because of the next barrier
    barrier(CLK_LOCAL_MEM_FENCE);

    // Now each WI increments the value atomically
    __global unsigned int* bufuip = &buf_out[0];
    atomic_add(bufuip, 1);

    // Make sure all WIs done incrementing
    barrier(CLK_LOCAL_MEM_FENCE);
    uint val2 = *((__global uint*) &buf_out[0]);

    buf_out[6] = 0;
}
