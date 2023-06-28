; RUN: opt -S -passes=gvbased-multiversioning -gvbased-multiversion-min-num-branches=3 < %s | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
@global1 = internal unnamed_addr global i1 false, align 1

declare void @unknown_side_effect() local_unnamed_addr #0

; No multiversiong is done because there is too few eligible branches.
; Nothing changes.
define void @no_multiversion_too_few_use(ptr nocapture noundef %num) #0 {
; CHECK-LABEL: define void @no_multiversion_too_few_use
; CHECK-NEXT:  entry:
; CHECK-NEXT:    %.b67 = load i1, ptr @global1, align 1
; CHECK-NEXT:    br i1 %.b67, label %if.end, label %if.end.thread
entry:
  %.b67 = load i1, ptr @global1, align 1
  br i1 %.b67, label %if.end, label %if.end.thread

if.end.thread:                                    ; preds = %entry
  %0 = load i32, ptr %num, align 4, !tbaa !3
  br label %if.else

if.end:                                           ; preds = %entry
  tail call void @unknown_side_effect()
  %.b8.pr = load i1, ptr @global1, align 1
  %1 = load i32, ptr %num, align 4, !tbaa !3
  %inc = add nsw i32 %1, 1
  store i32 %inc, ptr %num, align 4, !tbaa !3
  br i1 %.b8.pr, label %if.then2, label %if.else

if.then2:                                         ; preds = %if.end
  tail call void @unknown_side_effect()
  br label %if.end4

if.else:                                          ; preds = %if.end.thread, %if.end
  %2 = phi i32 [ %0, %if.end.thread ], [ %1, %if.end ]
  %inc3 = add nsw i32 %2, 2
  store i32 %inc3, ptr %num, align 4, !tbaa !3
  br label %if.end4

if.end4:                                          ; preds = %if.else, %if.then2
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
