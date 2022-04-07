void do_add(__global uchar* buf)
{
    __global unsigned int* bufuip = &buf[0];
    atomic_add(bufuip, 1);
    barrier(CLK_LOCAL_MEM_FENCE);
}

void add_twice(__global uchar* buf)
{
    do_add(buf);
    do_add(buf);
}

__kernel void main_kernel(__global uchar* buf_in, __global uchar* buf_out)
{
    // Initialize once (to 517)
    if (get_global_id(0) == 20) {
        buf_out[0] = 5;
        buf_out[1] = 2;
        buf_out[2] = 0;
        buf_out[3] = 0;
    }

    add_twice(buf_out);
    uint val2 = *((__global uint*) &buf_out[0]);

    buf_out[6] = 0;
}
