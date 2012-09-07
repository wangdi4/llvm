// vim:ts=2:sw=2:et:

include "SSE42.td"

class AVX : SSE42 {
  
  // copy paste from SSE42 plus AVX native types
  // TODO: how to use Native types from parent class SSE42 w/o copy-paste
  list<OclType> NativeTypes = [
      // SSE42
      v16i8, v16u8, v8i16, v8u16, v4i32, v4u32, v2i64, v2u64, v4f32, v2f64,
      // AVX
      v16i16, v16u16, v8i32, v8u32, v4i64, v4u64, v8f32, v4f64];
  include "avx/math.td"
}
