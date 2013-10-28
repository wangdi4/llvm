// vim:ts=2:sw=2:et:

include "GENERIC/opencl-builtin.td"
include "GENERIC/WorkGroupSnippets.td"
include "GENERIC/opencl-v1.2.td"
include "GENERIC/opencl-v2.0WG.td"
include "GENERIC/ll_intrinsics.td"

class Generic : TargetImpl {
include "GENERIC/GENERIC_common.td"
}
