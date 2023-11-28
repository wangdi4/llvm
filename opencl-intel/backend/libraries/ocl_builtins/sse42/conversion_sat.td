// Copyright (c) 2006 Intel Corporation
// Conversions (_sat) Built-In Functions
// SSE42
// convert_ushortn - ints
OclBuiltinImpl sse42_v8i32_conv_u16_sat = OclBuiltinImpl<convert_u16_sat, [v8i32] , 0, [{
    return as_$ReturnType(_mm_packus_epi32((__m128i)$Arg0VarName.lo, (__m128i)$Arg0VarName.hi));
  }]>;

