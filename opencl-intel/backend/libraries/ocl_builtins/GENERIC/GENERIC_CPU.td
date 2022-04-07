// vim:ts=2:sw=2:et:
// NOTE
// Purporse of the file GENERIC_CPU.td is to use during CPU built ins transition to tblgen
// When transition is done it will be the same as GENERIC.td
// In this file Gradually we enable "include" lines for categories of BuiltIns including math, common, converions, etc
// Other parts of file should be the same as GENERIC.td. Otherwise we can get build error


include "GENERIC/opencl-builtin.td"
include "GENERIC/matrix_layout_transform.td"
include "GENERIC/opencl-v1.2.td"
include "GENERIC/opencl-v2.0.td"
include "GENERIC/opencl-v2.0WG.td"
include "GENERIC/sub_group_builtin.td"
include "GENERIC/ll_intrinsics.td"
include "GENERIC/helper_function.td"
include "GENERIC/sycl-spirv.td"
include "GENERIC/code_helpers.td"

class Generic : TargetImpl {
include "GENERIC/GENERIC_common.td"
include "GENERIC/math_cpu.td"
}
