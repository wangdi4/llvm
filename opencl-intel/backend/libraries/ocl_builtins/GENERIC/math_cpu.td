// vim:ts=2:sw=2:et:

// NOTE:
// CPU only math built-ins


list<OclType> int32_types = [v1i32, v4i32, v8i32];

list<OclType> uint32_types = [v1u32, v4u32, v8u32];


// integer remainder
OclBuiltinImpl irem_svml = OclBuiltinImpl<irem, int32_types, 0, [{
    return __ocl_svml_$Target_irem$VecLength($Arg0VarName, $Arg1VarName);
  }]>;

// unsigned integer remainder
OclBuiltinImpl urem_svml = OclBuiltinImpl<urem, uint32_types, 0, [{
    return __ocl_svml_$Target_urem$VecLength($Arg0VarName, $Arg1VarName);
  }]>;

// integer division
OclBuiltinImpl idiv_svml = OclBuiltinImpl<idiv, int32_types, 0, [{
    return __ocl_svml_$Target_idiv$VecLength($Arg0VarName, $Arg1VarName);
  }]>;

// unsigned integer division
OclBuiltinImpl udiv_svml = OclBuiltinImpl<udiv, uint32_types, 0, [{
    return __ocl_svml_$Target_udiv$VecLength($Arg0VarName, $Arg1VarName);
  }]>;

OclBuiltinImpl idiv_expand = OclBuiltinImpl<idiv, [v16i32, v32i32, v64i32],
                                            0, Expand2>;
OclBuiltinImpl irem_expand = OclBuiltinImpl<irem, [v16i32, v32i32, v64i32],
                                            0, Expand2>;
OclBuiltinImpl udiv_expand = OclBuiltinImpl<udiv, [v16u32, v32u32, v64u32],
                                            0, Expand2>;
OclBuiltinImpl urem_expand = OclBuiltinImpl<urem, [v16u32, v32u32, v64u32],
                                            0, Expand2>;

OclBuiltinImpl idiv_promote = OclBuiltinImpl<idiv, [v2i32, v3i32], 0, Promote2>;
OclBuiltinImpl irem_promote = OclBuiltinImpl<irem, [v2i32, v3i32], 0, Promote2>;
OclBuiltinImpl udiv_promote = OclBuiltinImpl<udiv, [v2u32, v3u32], 0, Promote2>;
OclBuiltinImpl urem_promote = OclBuiltinImpl<urem, [v2u32, v3u32], 0, Promote2>;

