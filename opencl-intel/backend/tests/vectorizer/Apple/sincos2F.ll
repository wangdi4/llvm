; RUN: llvm-as %s -o %t.bc
; RUN: opt -runtimelib %p/../Full/apple_only_dcls32.ll -CLBltnPreVec -scalarize -packetize -packet-size=4 -resolve -verify %s -S -o - \
; RUN: | FileCheck %s

;; This test:
;; 1. Based on Apple clang calling convention
;; 2. Checks that vectorizer will vectorize non-uniform sincos instruction.

; CHECK: __Vectorized_.math_kernel2
; CHECK-NOT: sincos
; CHECK: call void @_Z14sincos_ret2ptrDv4_fPS_S0_
; CHECK: call void @_Z14sincos_ret2ptrDv4_fPS_S0_
; CHECK-NOT: sincos
; CHECK: ret

  ; ModuleID = '-'
  target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-s0:64:64-f80:128:128-n8:16:32:64"
  target triple = "x86_64-applecl-darwin11"
  
  @sgv = internal constant [4 x i8] c"222\00"
  @fgv = internal constant [0 x i8] zeroinitializer
  @lvgv = internal constant [0 x i8*] zeroinitializer
  @llvm.global.annotations = appending global [1 x { i8*, i8*, i8*, i8*, i32 }] [{ i8*, i8*, i8*, i8*, i32 } { i8* bitcast (void (<2 x float> addrspace(1)*, <2 x float> addrspace(1)*, <2 x float> addrspace(1)*)* @math_kernel2 to i8*), i8* getelementptr inbounds ([4 x i8]* @sgv, i32 0, i32 0), i8* getelementptr inbounds ([0 x i8]* @fgv, i32 0, i32 0), i8* bitcast ([0 x i8*]* @lvgv to i8*), i32 0 }], section "llvm.metadata"
  
  declare void @math_kernel2(<2 x float> addrspace(1)* nocapture %out, <2 x float> addrspace(1)* %out2, <2 x float> addrspace(1)* nocapture %in) nounwind
  
  declare i64 @_Z13get_global_idj(i32)
  
  declare <4 x float> @_Z6sincosDv2_fPU3AS1S_(double, <2 x float> addrspace(1)*)
  
  declare i64 @_Z14get_local_sizej(i32)
  
  declare i64 @get_base_global_id.(i32)
  
  define void @__Vectorized_.math_kernel2(<2 x float> addrspace(1)* nocapture %out, <2 x float> addrspace(1)* %out2, <2 x float> addrspace(1)* nocapture %in) nounwind {
     %1 = tail call i64 @_Z13get_global_idj(i32 0) nounwind
     %sext = shl i64 %1, 32
     %2 = ashr exact i64 %sext, 32
     %3 = getelementptr inbounds <2 x float> addrspace(1)* %in, i64 %2
     %4 = load <2 x float> addrspace(1)* %3, align 8, !tbaa !4
     %5 = getelementptr inbounds <2 x float> addrspace(1)* %out2, i64 %2
     %6 = bitcast <2 x float> %4 to double
     %7 = tail call <4 x float> @_Z6sincosDv2_fPU3AS1S_(double %6, <2 x float> addrspace(1)* %5) nounwind
     %8 = shufflevector <4 x float> %7, <4 x float> undef, <2 x i32> <i32 0, i32 1>
     %9 = getelementptr inbounds <2 x float> addrspace(1)* %out, i64 %2
     store <2 x float> %8, <2 x float> addrspace(1)* %9, align 8, !tbaa !4
     ret void
 }
  
  !opencl.kernels = !{!0}
  !opencl.kernel_info = !{!6}
  
  !0 = !{void (<2 x float> addrspace(1)*, <2 x float> addrspace(1)*, <2 x float> addrspace(1)*)* @math_kernel2, !1}
  !1 = !{!"apple.cl.arg_metadata", !2, !2, !2}
  !2 = !{!"stream", !"write", !"global"}
  !3 = !{!"no_barrier_path", i1 true}
  !4 = !{!"omnipotent char", !5}
  !5 = !{!"Simple C/C++ TBAA"}

  !6 = !{void (<2 x float> addrspace(1)*, <2 x float> addrspace(1)*, <2 x float> addrspace(1)*)* @math_kernel2, !7}
  !7 = !{!3}