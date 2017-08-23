; RUN: llvm-as %s -o %t.bc
; RUN: opt -runtimelib %p/../../Full/runtime.bc -scalarize -gather-scatter -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; Unmasked load/store <2 x i32>

; ModuleID = 'manual'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64"
target triple = "x86_64-unknown-linux-gnu"

;CHECK: kernel
;CHECK: [[NAME1:%[0-9]*]] = load i32, i32* [[NAME2:%GEP_s[0-9]*]]
;CHECK: [[NAME3:%[0-9]*]] = load i32, i32* [[NAME4:%GEP_s[0-9]*]]
;CHECK: store i32 [[NAME5:%[0-9]*]], i32* [[NAME6:%GEP_s[0-9]*]]
;CHECK: store i32 [[NAME7:%[0-9]*]], i32* [[NAME8:%GEP_s[0-9]*]]
;CHECK: ret void

define void @kernel(<2 x i32>* nocapture %A) nounwind {
  %1 = tail call i32 (...) @_Z13get_global_idj(i32 0) nounwind
  %2 = mul nsw i32 %1, 7
  %3 = sext i32 %2 to i64
  %4 = getelementptr inbounds <2 x i32>, <2 x i32>* %A, i64 %3
  %5 = load <2 x i32>, <2 x i32>* %4, align 4, !tbaa !0
  %6 = add nsw <2 x i32> %5, <i32 3, i32 3>
  store <2 x i32> %6, <2 x i32>* %4, align 4, !tbaa !0
  ret void
}

declare i32 @_Z13get_global_idj(...)

!0 = !{!"int", !1}
!1 = !{!"omnipotent char", !2}
!2 = !{!"Simple C/C++ TBAA"}
