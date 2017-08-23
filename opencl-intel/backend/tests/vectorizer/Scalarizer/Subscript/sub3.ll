; RUN: llvm-as %s -o %t.bc
; RUN: opt -runtimelib %p/../../Full/runtime.bc -scalarize -gather-scatter -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; Unmasked load/store <2 x float>

; ModuleID = 'manual'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64"
target triple = "x86_64-unknown-linux-gnu"

;CHECK: kernel
;CHECK: [[NAME1:%[0-9]*]] = load float, float* [[NAME2:%GEP_s[0-9]*]]
;CHECK: [[NAME3:%[0-9]*]] = load float, float* [[NAME4:%GEP_s[0-9]*]]
;CHECK: store float [[NAME5:%[0-9]*]], float* [[NAME6:%GEP_s[0-9]*]]
;CHECK: store float [[NAME7:%[0-9]*]], float* [[NAME8:%GEP_s[0-9]*]]
;CHECK: ret void

define void @kernel(<2 x float>* nocapture %A) nounwind {
  %1 = tail call i32 (...) @_Z13get_global_idj(i32 0) nounwind
  %2 = mul nsw i32 %1, 7
  %3 = sext i32 %2 to i64
  %4 = getelementptr inbounds <2 x float>, <2 x float>* %A, i64 %3
  %5 = load <2 x float>, <2 x float>* %4, align 4, !tbaa !0
  %6 = fadd <2 x float> %5, <float 3.0, float 3.0>
  store <2 x float> %6, <2 x float>* %4, align 4, !tbaa !0
  ret void
}

declare i32 @_Z13get_global_idj(...)

!0 = !{!"int", !1}
!1 = !{!"omnipotent char", !2}
!2 = !{!"Simple C/C++ TBAA"}
