// vim:ts=2:sw=2:et:

include "ssse3/SSSE3.td"

class SSE42 : SSSE3 {
// Native types for SSE42. Used in "Promote" calls
let NativeTypes = [v16i8, v16u8, v8i16, v8u16, v4i32, v4u32, v2i64, v2u64, v4f32, v2f64];

include "sse42/geometric.td"
include "sse42/integers.td"
include "sse42/math.td"
include "sse42/conversion_sat.td"
include "sse42/relational.td"
include "sse42/transpose.td"
include "sse42/sub_group_builtin_impl.td"
}
