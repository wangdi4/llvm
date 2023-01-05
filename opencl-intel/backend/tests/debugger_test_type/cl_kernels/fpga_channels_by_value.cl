#pragma OPENCL EXTENSION cl_intel_channels : enable

channel int ch;

__attribute__((noinline)) void channel_writer_func(channel int ch, int data) {
  write_channel_intel(ch, data);
}

__attribute__((noinline)) int channel_reader_func(channel int ch) {
  return read_channel_intel(ch);
}

__kernel void channel_writer(int size) {
  for (int i = 0; i < size; ++i)
    channel_writer_func(ch, i);
}

__kernel void channel_reader(int size, __global uint *out) {
  for (int i = 0; i < size; ++i)
    out[i] = channel_reader_func(ch);
}
