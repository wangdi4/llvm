; RUN: opt -hir-cg -force-hir-cg -S < %s | FileCheck %s
; Verifies basic nesting structure for a doubly nested loop nest
; Verifies nesting order is correct, and that correct ivs are 
; incremented in each loop

; basic cg occurred
; CHECK: region.0:

; verify basic loop structure
; for loop 23 (i1)
;    for loop.22 (i2)
;    afterloop 22
; afterloop 23

; CHECK: [[L1:loop.[0-9]+]]
; Load of B should be in i1 loop
; CHECK: getelementptr{{.*}} @B

; CHECK: [[L2:loop.[0-9]+]]

; Store of A should be in i2 loop
; CHECK: getelementptr{{.*}} @A
; increment of i2 occurs in i2 loop body
; CHECK: store i64 %nextiv[[L2]]

; increment of i1 should occur after i2 loop, afterloop.22
; CHECK: after[[L2]]
; CHECK: store i64 %nextiv[[L1]]

; after i1 loop, we should have a jump to region successor
; CHECK: after[[L1]]:
; CHECK-NEXT: br label %for.end12.loopexit
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@B = common global [10 x i32] zeroinitializer, align 16
@A = common global [10 x i32] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define void @foo(i32 %n, i32 %k) #0 {
entry:
  %cmp22 = icmp sgt i32 %n, 0
  br i1 %cmp22, label %for.cond2.preheader.lr.ph, label %for.end12

for.cond2.preheader.lr.ph:                        ; preds = %entry
  %conv7 = sext i32 %k to i64
  %0 = sext i32 %n to i64
  br label %for.body6.lr.ph

for.body6.lr.ph:                                  ; preds = %for.inc10, %for.cond2.preheader.lr.ph
  %i.023 = phi i64 [ 0, %for.cond2.preheader.lr.ph ], [ %inc11, %for.inc10 ]
  %arrayidx = getelementptr inbounds [10 x i32], [10 x i32]* @B, i64 0, i64 %i.023
  %1 = load i32, i32* %arrayidx, align 4, !tbaa !1
  %mul = shl i64 %i.023, 1
  br label %for.body6

for.body6:                                        ; preds = %for.body6, %for.body6.lr.ph
  %j.021 = phi i64 [ 0, %for.body6.lr.ph ], [ %inc, %for.body6 ]
  %mul8 = mul nsw i64 %j.021, %conv7
  %add = add nsw i64 %mul8, %mul
  %arrayidx9 = getelementptr inbounds [10 x i32], [10 x i32]* @A, i64 0, i64 %add
  store i32 %1, i32* %arrayidx9, align 4, !tbaa !1
  %inc = add nuw nsw i64 %j.021, 1
  %exitcond = icmp eq i64 %inc, %0
  br i1 %exitcond, label %for.inc10, label %for.body6

for.inc10:                                        ; preds = %for.body6
  %inc11 = add nuw nsw i64 %i.023, 1
  %exitcond24 = icmp eq i64 %inc11, %0
  br i1 %exitcond24, label %for.end12.loopexit, label %for.body6.lr.ph

for.end12.loopexit:                               ; preds = %for.inc10
  br label %for.end12

for.end12:                                        ; preds = %for.end12.loopexit, %entry
  ret void
}
; Function Attrs: nounwind
declare void @llvm.lifetime.start(i64, i8* nocapture) #1

; Function Attrs: nounwind
declare void @llvm.lifetime.end(i64, i8* nocapture) #1

attributes #0 = { nounwind uwtable "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.7.0 (trunk 768) (llvm/branches/loopopt 776)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"int", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}

