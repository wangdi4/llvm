// Copyright (C) 2022 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may
// not use, modify, copy, publish, distribute, disclose or transmit this
// software or the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

defvar NoCvt = [{
    return $Arg0VarName;
}];

defvar BitCast = [{
    return __builtin_astype($Arg0VarName, $ReturnType);
  }];

defvar BitSExt = [{
    return __builtin_astype(__ocl_sext_$Arg0Symigentype_$ReturnSymigentype(__builtin_astype($Arg0VarName, $Arg0igentype)), $ReturnType);
  }];

defvar BitZExt = [{
    return __builtin_astype(__ocl_zext_$Arg0Symigentype_$ReturnSymigentype(__builtin_astype($Arg0VarName, $Arg0igentype)), $ReturnType);
  }];

defvar BitTrunc = [{
    return __builtin_astype(__ocl_trunc_$Arg0Symigentype_$ReturnSymigentype(__builtin_astype($Arg0VarName, $Arg0igentype)), $ReturnType);
  }];

defvar PromoteI32 = [{
    return __builtin_astype(__ocl_trunc_$Arg0Symi32gentype_$ReturnSymigentype(convert_$Arg0i32gentype$Round($Arg0VarName)), $ReturnType);
  }];

defvar PromoteU32 = [{
    return __builtin_astype(__ocl_trunc_$Arg0Symi32gentype_$ReturnSymigentype(__builtin_astype(convert_$Arg0u32gentype$Round($Arg0VarName), $Arg0i32gentype)), $ReturnType);
  }];

defvar PromoteI32_ForU16 = [{
    return __builtin_astype(__ocl_trunc_$Arg0Symi32gentype_$ReturnSymigentype(__builtin_astype(convert_$Arg0i32gentype$Round($Arg0VarName), $Arg0i32gentype)), $ReturnType);
  }];

// TODO replace PromoteF16F32 and PromoteF16F64 with SVML functions when SVML
// has fp16 support.
defvar PromoteF16F32 = [{
    return convert_$ReturnType$Round(__ocl_fpext_$Arg0Symgentype_$Arg0Symf32gentype($Arg0VarName));
  }];

defvar PromoteF16F64 = [{
    return convert_$ReturnType$Round(__ocl_fpext_$Arg0Symgentype_$Arg0Symf64gentype($Arg0VarName));
  }];

defvar TruncF16 = [{
  return __ocl_fptrunc_$Arg0Symgentype_$ReturnSymgentype($Arg0VarName);
}];

// TODO honor rounding mode for converting to fp16.
// Replace CvtF32TruncF16Single and CvtF32TruncF16 with SVML functions when SVML
// has fp16 support.
defvar CvtF32TruncF16 = [{
  return __ocl_fptrunc_$Arg0Symf32gentype_$ReturnSymgentype(convert_$Arg0f32gentype($Arg0VarName));
}];

defvar ExtFp = [{
  return __ocl_fpext_$Arg0Symgentype_$ReturnSymgentype($Arg0VarName);
}];

// SVML

defvar FPToI64 = [{
    return __ocl_svml_$Target_cvtfptoi64$Round$SVMLSuffix($Arg0VarName);
  }];

defvar FPToU64 = [{
    return __ocl_svml_$Target_cvtfptou64$Round$SVMLSuffix($Arg0VarName);
  }];
