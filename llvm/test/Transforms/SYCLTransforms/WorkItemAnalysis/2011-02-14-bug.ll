; RUN: opt -passes='require<sycl-kernel-builtin-info-analysis>,print<sycl-kernel-work-item-analysis>' %s -disable-output 2>&1 | FileCheck %s

target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

declare i32 @_Z13get_global_idj(i32)

; CHECK: WorkItemAnalysis for function func:
; CHECK-NEXT: UNI   %0 = getelementptr inbounds float, ptr %A, i64 5
; CHECK-NEXT: UNI   %1 = load float, ptr %0, align 4
; CHECK-NEXT: UNI   %2 = fcmp ogt float %1, 3.000000e+00
; CHECK-NEXT: UNI   br i1 %2, label %bb, label %return
; CHECK-NEXT: UNI   %3 = getelementptr inbounds float, ptr %A, i64 6
; CHECK-NEXT: UNI   store float 8.000000e+00, ptr %3, align 4
; CHECK-NEXT: UNI   ret i32 undef
; CHECK-NEXT: UNI   ret i32 undef

define i32 @func(ptr nocapture %A) nounwind {
entry:
  %0 = getelementptr inbounds float, ptr %A, i64 5    ; <ptr> [#uses=1]
  %1 = load float, ptr %0, align 4                    ; <float> [#uses=1]
  %2 = fcmp ogt float %1, 3.000000e+00            ; <i1> [#uses=1]
  br i1 %2, label %bb, label %return

bb:                                               ; preds = %entry
  %3 = getelementptr inbounds float, ptr %A, i64 6    ; <ptr> [#uses=1]
  store float 8.000000e+00, ptr %3, align 4
  ret i32 undef

return:                                           ; preds = %entry
  ret i32 undef
}

; CHECK: WorkItemAnalysis for function func2:
; CHECK-NEXT: SEQ   %0 = tail call i32 @_Z13get_global_idj(i32 0)
; CHECK-NEXT: RND   %1 = icmp sgt i32 %0, 3
; CHECK-NEXT: RND   br i1 %1, label %bb, label %return
; CHECK-NEXT: UNI   %2 = getelementptr inbounds float, ptr %A, i64 6
; CHECK-NEXT: UNI   store float 8.000000e+00, ptr %2, align 4
; CHECK-NEXT: UNI   ret i32 undef
; CHECK-NEXT: UNI   ret i32 undef

define i32 @func2(ptr nocapture %A) nounwind {
entry:
  %0 = tail call i32 @_Z13get_global_idj(i32 0)        ; <i32> [#uses=1]
  %1 = icmp sgt i32 %0, 3                         ; <i1> [#uses=1]
  br i1 %1, label %bb, label %return

bb:                                               ; preds = %entry
  %2 = getelementptr inbounds float, ptr %A, i64 6    ; <ptr> [#uses=1]
  store float 8.000000e+00, ptr %2, align 4
  ret i32 undef

return:                                           ; preds = %entry
  ret i32 undef
}

!0 = !{!"float*"}
!1 = !{ptr addrspace(1) null}
