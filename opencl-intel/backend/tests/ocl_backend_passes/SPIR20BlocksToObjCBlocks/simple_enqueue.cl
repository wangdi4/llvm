; FIXME: https://jira01.devtools.intel.com/browse/CORC-1357
; XFAIL: *

void __kernel device_kernel(__global float * inout) {
  *inout = cos(*inout);
}

void __kernel host_kernel(__global float * inout) {
  enqueue_kernel(get_default_queue(),
                 CLK_ENQUEUE_FLAGS_WAIT_KERNEL,
                 ndrange_1D(1),
                 ^{ device_kernel(inout); });
}
