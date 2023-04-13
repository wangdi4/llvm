#if defined(cl_intel_required_subgroup_size)
#pragma OPENCL EXTENSION cl_intel_required_subgroup_size : enable
kernel void test() {
}
#else
#error cl_intel_required_subgroup_size extension is unsupported on tested device.
#endif
