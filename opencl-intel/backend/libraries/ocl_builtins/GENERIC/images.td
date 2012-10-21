OclBuiltinImpl read_imagei_decl = OclBuiltinImpl<
  read_imagei,
  [v2i32, v2f32],
  1,
  [{
  }]>;

OclBuiltinImpl read_imagef_decl = OclBuiltinImpl<
  read_imagef,
  [v2i32, v2f32],
  1,
  [{
  }]>;

OclBuiltinImpl read_imageui_decl = OclBuiltinImpl<
  read_imageui,
  [v2i32, v2f32],
  1,
  [{
  }]>;

OclBuiltinImpl mask_read_imagei_impl = OclBuiltinImpl<
  mask_read_imagei,
  [v2i32, v2f32],
  0,
  [{
    if(mask) return read_imagei(img, sampler, coord);
    return ($ReturnType)(0,0,0,0);
  }]>;

OclBuiltinImpl mask_read_imageui_impl = OclBuiltinImpl<
  mask_read_imageui,
  [v2i32, v2f32],
  0,
  [{
    if(mask) return read_imageui(img, sampler, coord);
    return ($ReturnType)(0,0,0,0);
  }]>;

OclBuiltinImpl mask_read_imagef_impl = OclBuiltinImpl<
  mask_read_imagef,
  [v2i32, v2f32],
  0,
  [{
    if(mask) return read_imagef(img, sampler, coord);
    return ($ReturnType)(0,0,0,0);
  }]>;
