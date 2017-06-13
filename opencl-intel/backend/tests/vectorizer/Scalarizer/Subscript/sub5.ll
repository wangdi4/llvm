; RUN: llvm-as %s -o %t.bc
; RUN: opt -runtimelib %p/../../Full/runtime.bc -scalarize -gather-scatter -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; Masked load/store of type <8 x float>

; ModuleID = 'manual'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64"
target triple = "x86_64-unknown-linux-gnu"

;CHECK: kernel
;CHECK: [[NAME1:%[0-9]*]] = load float, float* [[NAME2:%GEP_s[0-9]*]]
;CHECK: [[NAME3:%[0-9]*]] = load float, float* [[NAME4:%GEP_s[0-9]*]]
;CHECK: [[NAME5:%[0-9]*]] = load float, float* [[NAME6:%GEP_s[0-9]*]]
;CHECK: [[NAME7:%[0-9]*]] = load float, float* [[NAME8:%GEP_s[0-9]*]]
;CHECK: [[NAME9:%[0-9]*]] = load float, float* [[NAME10:%GEP_s[0-9]*]]
;CHECK: [[NAME11:%[0-9]*]] = load float, float* [[NAME12:%GEP_s[0-9]*]]
;CHECK: [[NAME13:%[0-9]*]] = load float, float* [[NAME14:%GEP_s[0-9]*]]
;CHECK: [[NAME15:%[0-9]*]] = load float, float* [[NAME16:%GEP_s[0-9]*]]
;CHECK: store float [[NAME17:%[0-9]*]], float* [[NAME18:%GEP_s[0-9]*]]
;CHECK: store float [[NAME19:%[0-9]*]], float* [[NAME20:%GEP_s[0-9]*]]
;CHECK: store float [[NAME21:%[0-9]*]], float* [[NAME22:%GEP_s[0-9]*]]
;CHECK: store float [[NAME23:%[0-9]*]], float* [[NAME24:%GEP_s[0-9]*]]
;CHECK: store float [[NAME25:%[0-9]*]], float* [[NAME26:%GEP_s[0-9]*]]
;CHECK: store float [[NAME27:%[0-9]*]], float* [[NAME28:%GEP_s[0-9]*]]
;CHECK: store float [[NAME29:%[0-9]*]], float* [[NAME30:%GEP_s[0-9]*]]
;CHECK: store float [[NAME31:%[0-9]*]], float* [[NAME32:%GEP_s[0-9]*]]
;CHECK: ret void

define void @kernel(<8 x float>* nocapture %A) nounwind {
  %1 = tail call i32 (...) @_Z13get_global_idj(i32 0) nounwind
  %2 = icmp sgt i32 %1, 70
  br i1 %2, label %3, label %9

; <label>:3                                       ; preds = %0
  %4 = mul nsw i32 %1, 7
  %5 = sext i32 %4 to i64
  %6 = getelementptr inbounds <8 x float>, <8 x float>* %A, i64 %5
  %7 = load <8 x float>, <8 x float>* %6, align 4, !tbaa !0
  %8 = fadd <8 x float> %7, <float 3.0, float 3.0, float 3.0, float 3.0, float 3.0, float 3.0, float 3.0, float 3.0>
  store <8 x float> %8, <8 x float>* %6, align 4, !tbaa !0
  br label %9

; <label>:9                                       ; preds = %3, %0
  ret void
}

declare i32 @_Z13get_global_idj(...)

!0 = !{!"int", !1}
!1 = !{!"omnipotent char", !2}
!2 = !{!"Simple C/C++ TBAA"}
