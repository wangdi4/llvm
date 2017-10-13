; RUN: llc -mtriple=csa < %s | FileCheck %s --check-prefix=CSA_CHECK 


; ModuleID = 'loopPickSwitch.cpp'
target datalayout = "e-m:e-i64:64-n32:64"
target triple = "csa"

; Function Attrs: nounwind
define i32 @loopPickSwitch(i32* nocapture %ip, i32 %n) #0 {
; CSA_CHECK-LABEL: loopPickSwitch
; CSA_CHECK: switch32

entry:
  %cmp6 = icmp sgt i32 %n, 0
  br i1 %cmp6, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.body
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body ], [ 2, %for.body.preheader ]
  %i.08 = phi i32 [ %add, %for.body ], [ 0, %for.body.preheader ]
  %idxprom = sext i32 %i.08 to i64
  %arrayidx = getelementptr inbounds i32, i32* %ip, i64 %idxprom
  store i32 %i.08, i32* %arrayidx, align 4, !tbaa !1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %0 = trunc i64 %indvars.iv.next to i32
  %add = add nsw i32 %i.08, %0
  %cmp = icmp slt i32 %add, %n
  br i1 %cmp, label %for.body, label %for.end.loopexit

for.end.loopexit:                                 ; preds = %for.body
  %.lcssa = phi i32 [ %0, %for.body ]
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  %j.0.lcssa = phi i32 [ 2, %entry ], [ %.lcssa, %for.end.loopexit ]
  ret i32 %j.0.lcssa
}

attributes #0 = { nounwind "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.6.0 (tags/RELEASE_360/final)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"int", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
