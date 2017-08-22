;RUN: opt -hir-cg -force-hir-cg -S %s | FileCheck %s

;;for this a[i], the gep does not need a leading 0 as %a is an argument,
;CHECK: loop.{{[0-9]+}}
;CHECK: [[I1:%.*]] = load i64, i64* %i1.i64
;CHECK: [[A_ADDR:%.*]] = getelementptr inbounds i32, i32* %a, i64 [[I1]]

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
; Function Attrs: nounwind readonly uwtable
define i32 @foo(i32* nocapture readonly %a, i32 %c) #0 {
entry:
  %t6 = alloca i32
  %i1.i64 = alloca i64
  %t3 = alloca i32
  %t4 = alloca i64
  %cmp.6 = icmp sgt i32 %c, 0
  br i1 %cmp.6, label %for.body.preheader, label %for.cond.cleanup

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.cond.cleanup.loopexit:                        ; preds = %for.body.split
  %add.lcssa = phi i32 [ %add, %for.body.split ]
  %0 = load i32, i32* %t3
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  %output.0.lcssa = phi i32 [ 0, %entry ], [ %add.lcssa, %for.cond.cleanup.loopexit ]
  ret i32 %output.0.lcssa

for.body:                                         ; preds = %for.body.preheader, %for.body.split
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body.split ], [ 0, %for.body.preheader ]
  %output.07 = phi i32 [ %add, %for.body.split ], [ 0, %for.body.preheader ]
  br label %for.body.split

for.body.split:                                   ; preds = %for.body
  %arrayidx = getelementptr inbounds i32, i32* %a, i64 %indvars.iv
  %1 = load i32, i32* %arrayidx, align 4
  %add = add nsw i32 %1, %output.07
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next to i32
  %exitcond = icmp eq i32 %lftr.wideiv, %c
  br i1 %exitcond, label %for.cond.cleanup.loopexit, label %for.body
}
attributes #0 = { nounwind readonly uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.7.0 (trunk 1049) (llvm/branches/loopopt 1096)"}
