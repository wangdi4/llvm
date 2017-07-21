// vim:ts=2:sw=2:et:

//promote and expand

  OclBuiltinImpl step_expand  = OclBuiltinImpl <step, [v8f32, v16f32, v3f64, v4f64, v8f64], 0, Expand2>;
  OclBuiltinImpl sign_expand  = OclBuiltinImpl <sign, [v8f32, v16f32, v3f64, v4f64, v8f64], 0, Expand1>;
