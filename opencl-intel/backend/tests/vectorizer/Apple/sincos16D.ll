; RUN: llvm-as %s -o %t.bc
; RUN: opt -runtimelib %p/../Full/apple_only_dcls32.ll -CLBltnPreVec -scalarize -packetize -packet-size=4 -resolve -verify %s -S -o - \
; RUN: | FileCheck %s

;; This test:
;; 1. Based on Apple clang calling convention
;; 2. Checks that vectorizer will vectorize non-uniform sincos instruction.

; CHECK: __Vectorized_.math_kernel16
; CHECK-NOT: sincos
; CHECK: call void @_Z14sincos_ret2ptrDv4_dPS_S0_
; CHECK: call void @_Z14sincos_ret2ptrDv4_dPS_S0_
; CHECK: call void @_Z14sincos_ret2ptrDv4_dPS_S0_
; CHECK: call void @_Z14sincos_ret2ptrDv4_dPS_S0_
; CHECK: call void @_Z14sincos_ret2ptrDv4_dPS_S0_
; CHECK: call void @_Z14sincos_ret2ptrDv4_dPS_S0_
; CHECK: call void @_Z14sincos_ret2ptrDv4_dPS_S0_
; CHECK: call void @_Z14sincos_ret2ptrDv4_dPS_S0_
; CHECK: call void @_Z14sincos_ret2ptrDv4_dPS_S0_
; CHECK: call void @_Z14sincos_ret2ptrDv4_dPS_S0_
; CHECK: call void @_Z14sincos_ret2ptrDv4_dPS_S0_
; CHECK: call void @_Z14sincos_ret2ptrDv4_dPS_S0_
; CHECK: call void @_Z14sincos_ret2ptrDv4_dPS_S0_
; CHECK: call void @_Z14sincos_ret2ptrDv4_dPS_S0_
; CHECK: call void @_Z14sincos_ret2ptrDv4_dPS_S0_
; CHECK: call void @_Z14sincos_ret2ptrDv4_dPS_S0_
; CHECK-NOT: sincos
; CHECK: ret

  ; ModuleID = '-'
  target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-s0:64:64-f80:128:128-n8:16:32:64"
  target triple = "x86_64-applecl-darwin11"
  
  @sgv = internal constant [4 x i8] c"222\00"
  @fgv = internal constant [0 x i8] zeroinitializer
  @lvgv = internal constant [0 x i8*] zeroinitializer
  @llvm.global.annotations = appending global [1 x { i8*, i8*, i8*, i8*, i32 }] [{ i8*, i8*, i8*, i8*, i32 } { i8* bitcast (void (<16 x double> addrspace(1)*, <16 x double> addrspace(1)*, <16 x double> addrspace(1)*)* @math_kernel16 to i8*), i8* getelementptr inbounds ([4 x i8]* @sgv, i32 0, i32 0), i8* getelementptr inbounds ([0 x i8]* @fgv, i32 0, i32 0), i8* bitcast ([0 x i8*]* @lvgv to i8*), i32 0 }], section "llvm.metadata"
  
  declare void @math_kernel16(<16 x double> addrspace(1)* nocapture %out, <16 x double> addrspace(1)* %out2, <16 x double> addrspace(1)* nocapture %in) nounwind
  
  declare i64 @_Z13get_global_idj(i32)
  
  declare void @_Z6sincosDv16_dPU3AS1S_(<16 x double>* sret, <16 x double>* byval align 128, <16 x double> addrspace(1)*)
  
  declare i64 @_Z14get_local_sizej(i32)
  
  declare i64 @get_base_global_id.(i32)
  
  define void @__Vectorized_.math_kernel16(<16 x double> addrspace(1)* nocapture %out, <16 x double> addrspace(1)* %out2, <16 x double> addrspace(1)* nocapture %in) nounwind {
     %1 = alloca <16 x double>, align 128
     %2 = alloca <16 x double>, align 128
     %3 = call i64 @_Z13get_global_idj(i32 0) nounwind
     %sext = shl i64 %3, 32
     %4 = ashr exact i64 %sext, 32
     %5 = getelementptr inbounds <16 x double> addrspace(1)* %in, i64 %4
     %6 = load <16 x double> addrspace(1)* %5, align 128, !tbaa !4
     %7 = getelementptr inbounds <16 x double> addrspace(1)* %out2, i64 %4
     store <16 x double> %6, <16 x double>* %2, align 128
     call void @_Z6sincosDv16_dPU3AS1S_(<16 x double>* sret %1, <16 x double>* byval align 128 %2, <16 x double> addrspace(1)* %7) nounwind
     %8 = load <16 x double>* %1, align 128
     %9 = getelementptr inbounds <16 x double> addrspace(1)* %out, i64 %4
     store <16 x double> %8, <16 x double> addrspace(1)* %9, align 128, !tbaa !4
     ret void
 }
  
  !opencl.kernels = !{!0}
  !opencl.kernel_info = !{!6}
    
  !0 = !{void (<16 x double> addrspace(1)*, <16 x double> addrspace(1)*, <16 x double> addrspace(1)*)* @math_kernel16, !1}
  !1 = !{!"apple.cl.arg_metadata", !2, !2, !2}
  !2 = !{!"stream", !"write", !"global"}
  !3 = !{!"no_barrier_path", i1 true}
  !4 = !{!"omnipotent char", !5}
  !5 = !{!"Simple C/C++ TBAA"}

  !6 = !{void (<16 x double> addrspace(1)*, <16 x double> addrspace(1)*, <16 x double> addrspace(1)*)* @math_kernel16, !7}
  !7 = !{!3}