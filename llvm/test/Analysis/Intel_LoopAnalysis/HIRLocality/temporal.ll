; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -hir-locality-analysis -hir-temporal-locality | FileCheck %s

; Verify the computed temporal locality for the loop.
; HIR-
; <18>            + DO i1 = 0, zext.i32.i64((-1 + %n)), 1   <DO_LOOP>
; <4>             |   %1 = (%A)[i1 + 4];
; <7>             |   %3 = (%A)[i1 + 8];
; <10>            |   (%A)[i1] = %1 + %3;
; <18>            + END LOOP


; CHECK: TempInv: 0
; CHECK: TempReuse: 2


;Module Before HIR; ModuleID = 'temporal-locality.c'
source_filename = "temporal-locality.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable
define void @foo(i32* nocapture %A, i32 %n) local_unnamed_addr #0 {
entry:
  %cmp14 = icmp sgt i32 %n, 0
  br i1 %cmp14, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.body
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body ], [ 0, %for.body.preheader ]
  %0 = add nuw nsw i64 %indvars.iv, 4
  %arrayidx = getelementptr inbounds i32, i32* %A, i64 %0
  %1 = load i32, i32* %arrayidx, align 4, !tbaa !1
  %2 = add nuw nsw i64 %indvars.iv, 8
  %arrayidx3 = getelementptr inbounds i32, i32* %A, i64 %2
  %3 = load i32, i32* %arrayidx3, align 4, !tbaa !1
  %add4 = add nsw i32 %3, %1
  %arrayidx6 = getelementptr inbounds i32, i32* %A, i64 %indvars.iv
  store i32 %add4, i32* %arrayidx6, align 4, !tbaa !1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next to i32
  %exitcond = icmp eq i32 %lftr.wideiv, %n
  br i1 %exitcond, label %for.end.loopexit, label %for.body

for.end.loopexit:                                 ; preds = %for.body
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start(i64, i8* nocapture) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end(i64, i8* nocapture) #1

attributes #0 = { norecurse nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.9.0 (trunk 17939) (llvm/branches/loopopt 17953)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"int", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
