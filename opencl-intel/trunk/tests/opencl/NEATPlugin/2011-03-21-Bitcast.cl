__kernel
void bitcast(__global float * input,
                  __global float * output,
                  const    uint  buffer_size)
{
    uint tid = get_global_id(0);
    float f = as_float(tid);
}
