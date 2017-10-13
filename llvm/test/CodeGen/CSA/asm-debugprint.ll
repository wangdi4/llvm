; RUN: llc -mtriple=csa < %s | FileCheck %s

; ModuleID = 'stream.cpp'
source_filename = "stream.cpp"
target datalayout = "e-m:e-i64:64-n32:64"
target triple = "csa"

; Function Attrs: nounwind
define void @compute_Triad(i32* noalias nocapture %A, i32* noalias nocapture readonly %B, i32* noalias %C, i32 %n, i32 %scalar) local_unnamed_addr #0 {

; CHECK: debugprint %ci32_{{[0-9]+}}
; CHECK: debugprint %ci64_{{[0-9]+}}

entry:
  %cmp16 = icmp sgt i32 %n, 0
  br i1 %cmp16, label %for.body.preheader, label %for.cond.cleanup

for.body.preheader:                               ; preds = %entry
  %wide.trip.count = zext i32 %n to i64
  br label %for.body

for.cond.cleanup.loopexit:                        ; preds = %for.body
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret void

for.body:                                         ; preds = %for.body.preheader, %for.body
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body ], [ 0, %for.body.preheader ]
  %arrayidx = getelementptr inbounds i32, i32* %C, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4, !tbaa !1
  %mul1 = mul nsw i32 %0, %scalar
  tail call void asm sideeffect "debugprint  $0", "c"(i32 %mul1) #1, !srcloc !5
  tail call void asm sideeffect "debugprint  $0", "d"(i32* %C) #1, !srcloc !6
  %arrayidx3 = getelementptr inbounds i32, i32* %B, i64 %indvars.iv
  %1 = load i32, i32* %arrayidx3, align 4, !tbaa !1
  %add = add nsw i32 %1, %mul1
  %arrayidx5 = getelementptr inbounds i32, i32* %A, i64 %indvars.iv
  store i32 %add, i32* %arrayidx5, align 4, !tbaa !1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.cond.cleanup.loopexit, label %for.body
}

attributes #0 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }

!llvm.ident = !{!0}

!0 = !{!"clang version 4.0.0 "}
!1 = !{!2, !2, i64 0}
!2 = !{!"int", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C++ TBAA"}
!5 = !{i32 647}
!6 = !{i32 695}
