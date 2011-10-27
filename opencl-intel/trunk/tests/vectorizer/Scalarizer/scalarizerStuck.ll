; RUN: llvm-as %s -o %t.bc
; RUN: opt  -runtimelib %p/../Full/runtime.bc -scalarize %t.bc -S

target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64"
target triple = "x86_64-pc-win32"

%opencl_metadata_type = type <{ i8*, i8*, [4 x i32], [4 x i32], i8*, i8* }>

@opencl_math_kernel2_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata"
@opencl_math_kernel2_parameters = appending global [129 x i8] c"float2 __attribute__((address_space(1))) *, float2 __attribute__((address_space(1))) *, int2 __attribute__((address_space(1))) *\00", section "llvm.metadata"
@opencl_metadata = appending global [1 x %opencl_metadata_type] [%opencl_metadata_type <{ i8* bitcast (void (<2 x float> addrspace(1)*, <2 x float> addrspace(1)*, <2 x i32> addrspace(1)*)* @math_kernel2 to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_math_kernel2_locals to i8*), i8* getelementptr inbounds ([129 x i8]* @opencl_math_kernel2_parameters, i32 0, i32 0) }>], section "llvm.metadata"

define void @math_kernel2(<2 x float> addrspace(1)* nocapture %out, <2 x float> addrspace(1)* nocapture %in1, <2 x i32> addrspace(1)* nocapture %in2) nounwind {

  %1 = tail call i64 @get_global_id(i32 0) nounwind
  %sext = shl i64 %1, 32
  %2 = ashr i64 %sext, 32
  %3 = getelementptr inbounds <2 x float> addrspace(1)* %in1, i64 %2
  %4 = load <2 x float> addrspace(1)* %3, align 8
  %5 = getelementptr inbounds <2 x i32> addrspace(1)* %in2, i64 %2
  %6 = load <2 x i32> addrspace(1)* %5, align 8
  %tmp8 = bitcast <2 x float> %4 to <1 x double>
  %tmp7 = extractelement <1 x double> %tmp8, i32 0
  %tmp5 = bitcast <2 x i32> %6 to <1 x double>
  %tmp4 = extractelement <1 x double> %tmp5, i32 0
  %7 = tail call double @_Z5ldexpDv2_fDv2_i(double %tmp7, double %tmp4) nounwind
  %tmp2 = bitcast double %7 to i64
  %tmp1 = bitcast i64 %tmp2 to <2 x float>
  %8 = getelementptr inbounds <2 x float> addrspace(1)* %out, i64 %2
  store <2 x float> %tmp1, <2 x float> addrspace(1)* %8, align 8
  ret void
}

declare i64 @get_global_id(i32)

declare double @_Z5ldexpDv2_fDv2_i(double, double)