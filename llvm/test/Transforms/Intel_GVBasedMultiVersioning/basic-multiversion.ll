; RUN: opt -S -passes=gvbased-multiversioning -gvbased-multiversion-min-num-branches=3 < %s | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@global1 = internal unnamed_addr global i1 false, align 1

declare void @unknown_side_effect() local_unnamed_addr #0

; Multiversioning should be done on @global1.
define void @basic_multiversion(ptr nocapture noundef %num) #0 {
; CHECK-LABEL: define void @basic_multiversion
; CHECK-NEXT:  mv.global.loads:
; CHECK-NEXT:    [[MV_LOAD_GLOBAL1:%.*]] = load i1, ptr @global1, align 1
; CHECK-NEXT:    [[TMP0:%.*]] = xor i1 [[MV_LOAD_GLOBAL1]], true
; CHECK-NEXT:    br i1 [[TMP0]], label %if.end.thread.clone, label %entry
; CHECK:       entry:
; Conditional branches in the cloned code are simplified.
; CHECK:       if.end.thread.clone:
; CHECK-NEXT:    [[TMP5:%.*]] = load i32, ptr %num, align 4
; CHECK-NEXT:    [[INC3_CLONE:%.*]] = add nsw i32 [[TMP5]], 2
; CHECK-NEXT:    store i32 [[INC3_CLONE]], ptr %num, align 4
; CHECK-NEXT:    [[TMP6:%.*]] = load i32, ptr %num, align 4
; CHECK-NEXT:    [[INC8_CLONE:%.*]] = add nsw i32 [[TMP6]], 1
; CHECK-NEXT:    store i32 [[INC8_CLONE]], ptr %num, align 4
; CHECK-NEXT:    ret void
;
entry:
  %.b1213 = load i1, ptr @global1, align 1
  br i1 %.b1213, label %if.end, label %if.end.thread

if.end.thread:                                    ; preds = %entry
  %0 = load i32, ptr %num, align 4, !tbaa !3
  br label %if.end4.thread

if.end:                                           ; preds = %entry
  tail call void @unknown_side_effect()
  %.b1114.pr = load i1, ptr @global1, align 1
  %1 = load i32, ptr %num, align 4, !tbaa !3
  %inc = add nsw i32 %1, 1
  store i32 %inc, ptr %num, align 4, !tbaa !3
  br i1 %.b1114.pr, label %if.end4, label %if.end4.thread

if.end4.thread:                                   ; preds = %if.end, %if.end.thread
  %2 = phi i32 [ %0, %if.end.thread ], [ %1, %if.end ]
  %inc3 = add nsw i32 %2, 2
  store i32 %inc3, ptr %num, align 4, !tbaa !3
  br label %if.end7

if.end4:                                          ; preds = %if.end
  tail call void @unknown_side_effect()
  %.b15.pre = load i1, ptr @global1, align 1
  br i1 %.b15.pre, label %if.then6, label %if.end7

if.then6:                                         ; preds = %if.end4
  tail call void @unknown_side_effect()
  br label %if.end7

if.end7:                                          ; preds = %if.end4.thread, %if.then6, %if.end4
  %3 = load i32, ptr %num, align 4, !tbaa !3
  %inc8 = add nsw i32 %3, 1
  store i32 %inc8, ptr %num, align 4, !tbaa !3
  ret void
}

attributes #0 = { "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 2}
!2 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2023.2.0 (2023.x.0.YYYYMMDD)"}
!3 = !{!4, !4, i64 0}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
