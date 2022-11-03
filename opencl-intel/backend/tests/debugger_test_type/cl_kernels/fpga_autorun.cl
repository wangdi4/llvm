#pragma OPENCL EXTENSION cl_intel_channels : enable
channel int arr[N + 1];
channel int in, out;

__attribute__((max_global_work_dim(0))) __attribute__((autorun)) __kernel void
single_plus() {
  int a = read_channel_intel(in);
  write_channel_intel(out, a + 1);
}

__attribute__((max_global_work_dim(0)))
__attribute__((num_compute_units(N, 1, 1))) __attribute__((autorun))
__kernel void
chained_plus() {
  size_t id = get_compute_id(0);
  int a = read_channel_intel(arr[id]);
  write_channel_intel(arr[id + 1], a + 1);
}

__kernel void reader_for_single_plus(int n, __global int *data) {
  for (int i = 0; i < n; ++i) {
    data[i] = read_channel_intel(out);
  }
}

__kernel void writer_for_single_plus(int n, __global int *data) {
  for (int i = 0; i < n; ++i) {
    write_channel_intel(in, data[i]);
  }
}

__kernel void reader_for_chained_plus(int n, __global int *data) {
  for (int i = 0; i < n; ++i) {
    data[i] = read_channel_intel(arr[N]);
  }
}

__kernel void writer_for_chained_plus(int n, __global int *data) {
  for (int i = 0; i < n; ++i) {
    write_channel_intel(arr[0], data[i]);
  }
}
