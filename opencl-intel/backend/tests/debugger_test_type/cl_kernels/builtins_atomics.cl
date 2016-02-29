__kernel void main_kernel(__global uchar* buf_in, __global uchar* buf_out)
{
    if (get_global_id(0) != 0) // avoid races
        return;

    buf_out[0] = 5;
    buf_out[1] = 2;
    buf_out[2] = 0;
    buf_out[3] = 0;
    uint val1 = *((__global uint*) &buf_out[0]);

    __global unsigned int* bufuip = &buf_out[0];
    if (get_global_id(0) == 0)
        atomic_add(bufuip, 70);

    uint val2 = *((__global uint*) &buf_out[0]);
    buf_out[0] = 0;
}
