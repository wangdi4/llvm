// vim:ts=2:sw=2:et:

include "GENERIC/opencl-builtin.td"
include "GENERIC/opencl-v1.2.td"
include "GENERIC/opencl-v2.0WG.td"
include "GENERIC/ll_intrinsics.td"

class Generic : TargetImpl {
include "GENERIC/GENERIC_common.td"
include "GENERIC/common_shared.td"
include "GENERIC/math_shared.td"
include "GENERIC/conversion_shared.td"
include "GENERIC/conversion_sat_shared.td"
include "GENERIC/integer_shared.td"
include "GENERIC/geometric.td"
include "GENERIC/relational_shared.td"
}
