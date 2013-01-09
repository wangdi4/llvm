// vim:ts=2:sw=2:et:

include "opencl-builtin.td"
include "opencl-v1.2.td"

include "ll_intrinsics.td"

class Generic : TargetImpl {


// shared between CPU and MIC
include "GENERIC_common.td"

include "GENERIC/integer.td"
include "GENERIC/conversion.td"
include "GENERIC/conversion_sat.td"
include "GENERIC/atomic.td"
include "GENERIC/relational.td"
include "GENERIC/geometric.td"
include "GENERIC/misc.td"
}
