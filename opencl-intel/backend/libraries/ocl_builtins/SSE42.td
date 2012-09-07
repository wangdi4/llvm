// vim:ts=2:sw=2:et:

include "GENERIC_CPU.td"

class SSE42 : Generic {
// Native types for SSE42. Used in "Promote" calls
list<OclType> NativeTypes = [v16i8, v16u8, v8i16, v8u16, v4i32, v4u32, v2i64, v2u64, v4f32, v2f64];

// TODO: uncomment when common and geom are ready  
//include "sse42/common.td"
//include "sse42/geometric.td"
//include "sse42/soa_geometric.td"
include "sse42/math.td"
}
