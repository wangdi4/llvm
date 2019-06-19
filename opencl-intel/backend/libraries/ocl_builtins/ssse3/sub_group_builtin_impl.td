list<OclType> sub_group_types_vec = [v4i32, v8i32, v16i32,
                                     v4u32, v8u32, v16u32,
                                     v4i64, v8i64, v16i64,
                                     v4u64, v8u64, v16u64,
                                     v4f32, v8f32, v16f32,
                                     v4f64, v8f64, v16f64];

// broadcast

OclBuiltinImpl sub_group_broadcast_vector = OclBuiltinImpl<sub_group_broadcast, sub_group_types_vec, 0,
  [{
    return $Arg0VarName[$Arg1VarName];
  }]>;

