// INTEL CONFIDENTIAL
//
// Copyright 2022 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

////////////////////////////////////////////////////////////////////////////////
//                SVML ABI hacks for SSE42
// Problem is with ABI to SVML. We cannot pass some argument types. Example:
// double16
// We have to route call to lower datatype SVML function.
// What you see below is series of dirty hacks to substitute calls to failing
// ABI SVML functions with functions which are legal to call.
// float3, double3 are promoted to call float4, double4 versions
// float16, sometimes float8, double16 are expanded to lower versions of SVML
// The whole code section below should be gone when ABI to SVML is fixed.
// ABI for SVML does not work for all supported types
////////////////////////////////////////////////////////////////////////////////

  // promote f(v3) to v4. common code for float3 and double3
  code Promote1_v3_to_v4 = [{
    $Arg0BaseType#4 n$Arg0VarName;
    n$Arg0VarName.s012 = $Arg0VarName;
    return $Func(n$Arg0VarName).s012;
  }];

  // promote f(v3, v3) to v4. common code for float3 and double3
  code Promote2_v3_to_v4 = [{
    $Arg0BaseType#4 n$Arg0VarName;
    n$Arg0VarName.s012 = $Arg0VarName;
    $Arg1BaseType#4  n$Arg1VarName;
    n$Arg1VarName.s012 = $Arg1VarName;
    $Arg0BaseType#4 $ReturnVarName = $Func(n$Arg0VarName, n$Arg1VarName);
    return $ReturnVarName.s012;
  }];

  // The correct way to promote is to fill 4th component with a
  // performance-tolerant value obtained from SVML docs that is special to every
  // SVML function, so in ideal case every promotion to the bigger vector must
  // be performed individually for every BI that requires it.
  //
  // promote f(v3, v3) to v4. performance-tolerant code for float3 and double3
  code Promote2_tolerant_v3_to_v4 = [{
    $Arg0BaseType#4 n$Arg0VarName;
    n$Arg0VarName.s012 = $Arg0VarName;
    n$Arg0VarName.s3 = $TolerantValue0;
    $Arg1BaseType#4  n$Arg1VarName;
    n$Arg1VarName.s012 = $Arg1VarName;
    n$Arg1VarName.s3 = $TolerantValue1;
    $Arg0BaseType#4 $ReturnVarName = $Func(n$Arg0VarName, n$Arg1VarName);
    return $ReturnVarName.s012;
  }];

  // promote f(v3, v3, v3) to v4. common code for float3 and double3
  code Promote3_v3_to_v4 = [{
    $Arg0BaseType#4 n$Arg0VarName;
    n$Arg0VarName.s012 = $Arg0VarName;
    $Arg1BaseType#4 n$Arg1VarName;
    n$Arg1VarName.s012 = $Arg1VarName;
    $Arg2BaseType#4 n$Arg2VarName;
    n$Arg2VarName.s012 = $Arg2VarName;
    $Arg0BaseType#4 $ReturnVarName = $Func(n$Arg0VarName, n$Arg1VarName,n$Arg2VarName);
    return $ReturnVarName.s012;
  }];

  // promote f(v3, pv3) to v4. common code for float3 and double3
  code Promote_2Ptr_v3_to_v4 = [{
    $Arg0BaseType#4 n$Arg0VarName = ($Arg0BaseType#4)0;
    $Arg1BaseType#4 p$Arg1VarName;
    n$Arg0VarName.s012 = $Arg0VarName;
    $Arg0BaseType#4 $ReturnVarName = $Func(n$Arg0VarName, &p$Arg1VarName);
    *$Arg1VarName = p$Arg1VarName.s012;
    return $ReturnVarName.s012;
  }];

  // promote f(v3, v3, pv3) to v4. common code for float3 and double3
  code Promote_remquo_v3_to_v4 = [{
    $Arg0BaseType#4 n$Arg0VarName;
    $Arg1BaseType#4 n$Arg1VarName;
    $Arg2BaseType#4 p$Arg2VarName;
    n$Arg0VarName.s012 = $Arg0VarName;
    n$Arg1VarName.s012 = $Arg1VarName;
    $Arg0BaseType#4 $ReturnVarName = $Func(n$Arg0VarName, n$Arg1VarName, &p$Arg2VarName);
    *$Arg2VarName = p$Arg2VarName.s012;
    return $ReturnVarName.s012;
  }];
