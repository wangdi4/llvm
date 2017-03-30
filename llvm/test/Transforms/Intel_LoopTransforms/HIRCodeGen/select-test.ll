;RUN: opt -hir-cg -force-hir-cg -S %s | FileCheck %s

;Verify CG for this HIR STMT
;<7> %.c = (%0 > %1) ? %0 : %c;
;CHECK: loop.{{.*}}:
; Rely on Verifier to filter illegal combinations
;CHECK-DAG: [[A_ADDR:%.*]] = getelementptr inbounds i32, i32* %a, 
;CHECK-DAG: [[A_LOAD:%.*]] = load i32, i32* [[A_ADDR]]
;CHECK-DAG: store i32 [[A_LOAD]], i32* %t[[A_SYM:[0-9]+]]

;CHECK-DAG: [[B_ADDR:%.*]] = getelementptr inbounds i32, i32* %b, 
;CHECK-DAG: [[B_LOAD:%.*]] = load i32, i32* [[B_ADDR]]
;CHECK-DAG: store i32 [[B_LOAD]], i32* %t[[B_SYM:[0-9]+]]

;predicate is a[i] > b[i]
;CHECK: [[HIR_CMP:%.*]] = icmp sgt i32 %t[[A_SYM]].{{[0-9]*}}, %t[[B_SYM]].{{[0-9]*}}
;true value is a[i] false is %c
;CHECK: select i1 [[HIR_CMP]], i32 %t[[A_SYM]].{{[0-9]*}}, i32 %c

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind readonly uwtable
define i32 @foo(i32* nocapture readonly %a, i32* nocapture readonly %b, i32 %c) #0 {
entry:
  %cmp.17 = icmp sgt i32 %c, 0
  br i1 %cmp.17, label %for.body.preheader, label %for.cond.cleanup

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.cond.cleanup.loopexit:                        ; preds = %for.body
  %output.1.lcssa = phi i32 [ %output.1, %for.body ]
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  %output.0.lcssa = phi i32 [ 0, %entry ], [ %output.1.lcssa, %for.cond.cleanup.loopexit ]
  ret i32 %output.0.lcssa

for.body:                                         ; preds = %for.body.preheader, %for.body
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body ], [ 0, %for.body.preheader ]
  %output.018 = phi i32 [ %output.1, %for.body ], [ 0, %for.body.preheader ]
  %arrayidx = getelementptr inbounds i32, i32* %a, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4
  %arrayidx2 = getelementptr inbounds i32, i32* %b, i64 %indvars.iv
  %1 = load i32, i32* %arrayidx2, align 4
  %cmp3 = icmp sgt i32 %0, %1
  %.c = select i1 %cmp3, i32 %0, i32 %c
  %output.1 = add nsw i32 %.c, %output.018
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next to i32
  %exitcond = icmp eq i32 %lftr.wideiv, %c
  br i1 %exitcond, label %for.cond.cleanup.loopexit, label %for.body
}

attributes #0 = { nounwind readonly uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.7.0 (trunk 1049) (llvm/branches/loopopt 1096)"}
