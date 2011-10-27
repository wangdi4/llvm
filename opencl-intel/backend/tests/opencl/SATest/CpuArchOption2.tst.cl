__kernel void Test(global float* in, global float* out)
{
    out[get_global_id(0)] = in[get_global_id(0)];
}