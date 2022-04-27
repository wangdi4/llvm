#pragma OPENCL EXTENSION cl_intel_channels : enable

channel int ch;

__kernel void channel_writer(int size) {
  for (int i = 0; i < size; ++i)
    write_channel_intel(ch, i);
}

__kernel void channel_reader(int size, __global uint *out) {
  for (int i = 0; i < size; ++i)
    out[i] = read_channel_intel(ch);
}
