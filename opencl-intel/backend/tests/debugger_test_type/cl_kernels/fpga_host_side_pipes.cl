#pragma OPENCL EXTENSION cl_intel_fpga_host_pipe : enable

__kernel void main_kernel(read_only pipe int pin
                          __attribute__((intel_host_accessible)),
                          write_only pipe int pout
                          __attribute__((intel_host_accessible)),
                          int iters) {
  for (int i = 0; i < iters; ++i) {
    int val = 0;
    while (read_pipe(pin, &val)) {
    }
    while (write_pipe(pout, &val)) {
    }
  }
}
