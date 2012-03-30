// vim:ts=2:sw=2:et:

// Conversions optimized for MIC

OclBuiltinImpl mic_cvt_f64_f32 = OclBuiltinImpl<convert_f64,
                                                [v1f32, v2f32, v3f32, v4f32, v8f32, v16f32], 0, [{
    // TODO
    return ($ReturnType)0;
  }]>;

OclBuiltinImpl mic_rte_cvt_f64_f32 = OclBuiltinImpl<convert_f64_rte,
                                                    [v1f32, v2f32, v3f32, v4f32, v8f32, v16f32], 0, [{
    // TODO
    return ($ReturnType)0;
  }]>;

OclBuiltinImpl mic_rtz_cvt_f64_f32 = OclBuiltinImpl<convert_f64_rtz,
                                                    [v1f32, v2f32, v3f32, v4f32, v8f32, v16f32], 0, [{
    // TODO
    return ($ReturnType)0;
  }]>;

OclBuiltinImpl mic_rtp_cvt_f64_f32 = OclBuiltinImpl<convert_f64_rtp,
                                                    [v1f32, v2f32, v3f32, v4f32, v8f32, v16f32], 0, [{
    // TODO
    return ($ReturnType)0;
  }]>;

OclBuiltinImpl mic_rtn_cvt_f64_f32 = OclBuiltinImpl<convert_f64_rtn,
                                                    [v1f32, v2f32, v3f32, v4f32, v8f32, v16f32], 0, [{
    // TODO
    return ($ReturnType)0;
  }]>;
