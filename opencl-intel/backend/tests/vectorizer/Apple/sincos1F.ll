; RUN: llvm-as %s -o %t.bc
; RUN: opt -runtimelib %p/../Full/apple_only_dcls32.ll -CLBltnPreVec -scalarize -packetize -packet-size=4 -resolve -verify %s -S -o - \
; RUN: | FileCheck %s

;; This test:
;; 1. Based on Apple clang calling convention
;; 2. Checks that vectorizer will vectorize non-uniform sincos instruction.

; CHECK: __Vectorized_.math_kernel
; CHECK-NOT: sincos
; CHECK: call void @_Z14sincos_ret2ptrDv4_fPS_S0_
; CHECK-NOT: sincos
; CHECK: ret

  ; ModuleID = '-'
  target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a64:64:64-s0:64:64-f80:128:128-n8:16:32:64"
  target triple = "x86_64-applecl-darwin11"
  
  @sgv = internal constant [4 x i8] c"222\00"
  @fgv = internal constant [0 x i8] zeroinitializer
  @lvgv = internal constant [0 x i8*] zeroinitializer
  @llvm.global.annotations = appending global [1 x { i8*, i8*, i8*, i8*, i32 }] [{ i8*, i8*, i8*, i8*, i32 } { i8* bitcast (void (float addrspace(1)*, float addrspace(1)*, float addrspace(1)*)* @math_kernel to i8*), i8* getelementptr inbounds ([4 x i8]* @sgv, i32 0, i32 0), i8* getelementptr inbounds ([0 x i8]* @fgv, i32 0, i32 0), i8* bitcast ([0 x i8*]* @lvgv to i8*), i32 0 }], section "llvm.metadata"
  
  declare void @math_kernel(float addrspace(1)* nocapture %out, float addrspace(1)* %out2, float addrspace(1)* nocapture %in) nounwind
  
  declare i64 @get_global_id(i32)
  
  declare float @_Z6sincosfPU3AS1f(float, float addrspace(1)*)
  
  declare i64 @get_local_size(i32)
  
  declare i64 @get_base_global_id.(i32)
  
  define void @__Vectorized_.math_kernel(float addrspace(1)* nocapture %out, float addrspace(1)* %out2, float addrspace(1)* nocapture %in) nounwind {
     %1 = tail call i64 @get_global_id(i32 0) nounwind
     %sext = shl i64 %1, 32
     %2 = ashr exact i64 %sext, 32
     %3 = getelementptr inbounds float addrspace(1)* %in, i64 %2
     %4 = load float addrspace(1)* %3, align 4, !tbaa !4
     %5 = getelementptr inbounds float addrspace(1)* %out2, i64 %2
     %6 = tail call float @_Z6sincosfPU3AS1f(float %4, float addrspace(1)* %5) nounwind
     %7 = getelementptr inbounds float addrspace(1)* %out, i64 %2
     store float %6, float addrspace(1)* %7, align 4, !tbaa !4
     ret void
 }
  
  !opencl.kernels = !{!0}
  !opencl.kernel_info = !{!7}
  
  !0 = metadata !{void (float addrspace(1)*, float addrspace(1)*, float addrspace(1)*)* @math_kernel, metadata !1}
  !1 = metadata !{metadata !"apple.cl.arg_metadata", metadata !2, metadata !2, metadata !2}
  !2 = metadata !{metadata !"stream", metadata !"write", metadata !"global"}
  !3 = metadata !{metadata !"no_barrier_path", i1 true}
  !4 = metadata !{metadata !"float", metadata !5}
  !5 = metadata !{metadata !"omnipotent char", metadata !6}
  !6 = metadata !{metadata !"Simple C/C++ TBAA"}

  !7 = metadata !{void (float addrspace(1)*, float addrspace(1)*, float addrspace(1)*)* @math_kernel, metadata !8}
  !8 = metadata !{metadata !3}