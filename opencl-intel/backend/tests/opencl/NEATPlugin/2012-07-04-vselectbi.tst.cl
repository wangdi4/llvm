// Debug fails release passes, sor tst fail is not added to test
// Assertion failed: it0 != ArgVals.end(), file
// C:\Projects\OpenCL_32_migration\src\backend\validations\PlugInNEAT\PlugInNEAT.cpp,
// line 1986
__kernel void test_vselectbi(__global char4 *out, __global const char4 *in1,
                             __global const char4 *in2,
                             __global const char4 *in3) {
  int index = get_global_id(0);
  out[index] = select(in1[index], in2[index], in3[index]);
}
