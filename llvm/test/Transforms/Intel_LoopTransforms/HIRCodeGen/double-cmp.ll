;RUN: opt -hir-cg -force-hir-cg -S %s | FileCheck %s
;
;CHECK: region.0:
;CHECK: fcmp olt double %a, %b
;
;          BEGIN REGION { }
;<14>         + DO i1 = 0, zext.i32.i64((-2 + %N)), 1   <DO_LOOP>
;<6>          |   %2 = (%a < %b) ? i1 + 2 : i1 + 3;
;<8>          |   (%out)[i1 + 1] = %2;
;<14>         + END LOOP
;          END REGION
;
; ModuleID = 'short.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define void @foo(i32 %N, double %a, double %b, i32* nocapture %out) #0 {
entry:
  %cmp.7 = icmp sgt i32 %N, 1
  br i1 %cmp.7, label %for.body.lr.ph, label %for.end

for.body.lr.ph:                                   ; preds = %entry
  %cmp1 = fcmp olt double %a, %b
  br label %for.body

for.body:                                         ; preds = %for.body, %for.body.lr.ph
  %indvars.iv = phi i64 [ 1, %for.body.lr.ph ], [ %indvars.iv.next, %for.body ]
  %i.08 = phi i32 [ 1, %for.body.lr.ph ], [ %add, %for.body ]
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %add = add nuw nsw i32 %i.08, 1
  %0 = add nuw nsw i64 %indvars.iv, 2
  %1 = trunc i64 %0 to i32
  %2 = select i1 %cmp1, i32 %add, i32 %1
  %arrayidx = getelementptr inbounds i32, i32* %out, i64 %indvars.iv
  store i32 %2, i32* %arrayidx, align 4, !tbaa !1
  %lftr.wideiv = trunc i64 %indvars.iv.next to i32
  %exitcond = icmp eq i32 %lftr.wideiv, %N
  br i1 %exitcond, label %for.end.loopexit, label %for.body

for.end.loopexit:                                 ; preds = %for.body
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  ret void
}

attributes #0 = { nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.8.0 (trunk 1607) (llvm/branches/loopopt 1631)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"int", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
