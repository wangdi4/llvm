// vim:ts=2:sw=2:et:
// NOTE
// Purporse of the file GENERIC_CPU.td is to use during CPU built ins transition to tblgen
// When transition is done it will be the same as GENERIC.td
// In this file Gradually we enable "include" lines for categories of BuiltIns including math, common, converions, etc
// Other parts of file should be the same as GENERIC.td. Otherwise we can get build error 


include "opencl-builtin.td"
include "opencl-v1.2.td"

include "ll_intrinsics.td"
include "SVML.td"

class Generic : TargetImpl {

include "GENERIC_common.td"
//include "GENERIC/integer.td"
//include "GENERIC/common.td"
//include "GENERIC/conversion.td"
//include "GENERIC/conversion_sat.td"
//include "GENERIC/atomic.td"
}
