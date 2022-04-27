; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -enable-new-pm=0 -hir-locality-analysis -hir-temporal-locality | FileCheck %s
; RUN: opt < %s -passes=hir-ssa-deconstruction | opt -passes="print<hir-locality-analysis>" -hir-temporal-locality -disable-output 2>&1 | FileCheck %s

; Verify that the computed temporal reuse locality for the loop is 2, one each for these two groups-
; (%A)[2 * i1 + 1], (%A)[2 * i1 + 3];
; (%A)[2 * i1], (%A)[2 * i1 + 2]
;
; HIR-
; <23>            + DO i1 = 0, 79, 1   <DO_LOOP>
; <5>             |   %2 = (%A)[2 * i1 + 1];
; <8>             |   %4 = (%A)[2 * i1 + 2];
; <12>            |   %6 = (%A)[2 * i1 + 3];
; <15>            |   (%A)[2 * i1] = %2 + %4 + %6;
; <23>            + END LOOP

; CHECK: TempInv: 0
; CHECK: TempReuse: 2


; ModuleID = 't.ll'
source_filename = "temp_locality.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable
define void @foo(i32* nocapture %A) local_unnamed_addr #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %0 = shl nsw i64 %indvars.iv, 1
  %1 = or i64 %0, 1
  %arrayidx = getelementptr inbounds i32, i32* %A, i64 %1
  %2 = load i32, i32* %arrayidx, align 4, !tbaa !2
  %3 = add nuw nsw i64 %0, 2
  %arrayidx4 = getelementptr inbounds i32, i32* %A, i64 %3
  %4 = load i32, i32* %arrayidx4, align 4, !tbaa !2
  %add5 = add nsw i32 %4, %2
  %5 = add nuw nsw i64 %0, 3
  %arrayidx9 = getelementptr inbounds i32, i32* %A, i64 %5
  %6 = load i32, i32* %arrayidx9, align 4, !tbaa !2
  %add10 = add nsw i32 %add5, %6
  %arrayidx13 = getelementptr inbounds i32, i32* %A, i64 %0
  store i32 %add10, i32* %arrayidx13, align 4, !tbaa !2
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 80
  %indvars.iv.in = bitcast i64 %indvars.iv.next to i64
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start(i64, i8* nocapture) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end(i64, i8* nocapture) #1

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }

!llvm.ident = !{!0}

!0 = !{!"clang version 4.0.0 (trunk 20641) (llvm/branches/loopopt 20656)"}
!2 = !{!3, !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
