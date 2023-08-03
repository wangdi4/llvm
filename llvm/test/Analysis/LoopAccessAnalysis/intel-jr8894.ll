; RUN: opt -passes="loop-load-elim" -S < %s | FileCheck %s
;
; CMPLRLLVM-8894: LoopAccessAnalysis assumption that allocation size
; and type size are the same caused LoopLoadElimination pass to crash.

; CHECK: %rlast.0 = phi ptr [ %incdec.ptr, %do.body ], [ %spec.select, %entry ]
; CHECK: %arrayidx5 = getelementptr inbounds i8, ptr %rlast.0, i64 undef
; CHECK: %0 = load i8, ptr %arrayidx5, align 1, !tbaa !1
; CHECK: store i8 %0, ptr %rlast.0, align 1, !tbaa !1


; ModuleID = 'bugpoint-reduced-simplified.bc'
source_filename = "tr67329.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @DCTEGetNCLineBig() local_unnamed_addr #0 {
entry:
  %spec.select = select i1 undef, ptr null, ptr undef
  br label %do.body

do.body:                                          ; preds = %do.body, %entry
  %rlast.0 = phi ptr [ %incdec.ptr, %do.body ], [ %spec.select, %entry ]
  %arrayidx5 = getelementptr inbounds i8, ptr %rlast.0, i64 undef
  %0 = load i8, ptr %arrayidx5, align 1, !tbaa !1
  store i8 %0, ptr %rlast.0, align 1, !tbaa !1
  %incdec.ptr = getelementptr inbounds i8, ptr %rlast.0, i64 1
  %cmp = icmp eq ptr %incdec.ptr, undef
  br i1 %cmp, label %do.end.loopexit, label %do.body, !llvm.loop !4

do.end.loopexit:                                  ; preds = %do.body
  ret void
}

attributes #0 = { "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"icx (ICX) 2019.8.2.0"}
!1 = !{!2, !2, i64 0}
!2 = !{!"omnipotent char", !3, i64 0}
!3 = !{!"Simple C/C++ TBAA"}
!4 = distinct !{!4, !5}
!5 = !{!"llvm.loop.isvectorized", i32 1}
