; Verify that decomposer generates non-empty VPPHINodes i.e either valid
; induction phi or empty phis later fixed by fixPhiNode.

; int  foo(int *arr, int n1, int n2, int sum)
; {
;   int index, t1;
;
;   for (index = 0; index < 1024; index++) {
;     t1 = arr[index + n1 * n2 + 3];
;     sum += t1;
;     arr[index + n1 * n2 + 3] = index + n1 * n2 + 3;
;   }
;   return sum + t1;
; }

; RUN: opt -hir-ssa-deconstruction -hir-vec-dir-insert -VPlanDriverHIR -vplan-print-after-simplify-cfg -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-vec-dir-insert,vplan-driver-hir" -vplan-print-after-simplify-cfg -disable-output < %s 2>&1 | FileCheck %s

; CHECK-DAG: i64 [[IVPhi:%.*]] = phi  [ i64 0, [[LoopPH:BB.*]] ],  [ i64 {{%.*}}, [[Latch:BB.*]] ]
; CHECK-DAG: i32 [[Phi:%.*]] = phi  [ i32 [[LiveIn:%.*]], [[LoopPH]] ],  [ i32 [[Sum:%.*]], [[Latch]] ]
; CHECK: i32 [[ALoad:%.*]] = load i32* {{%.*}}
; CHECK-NEXT: i32 [[Sum]] = add i32 [[ALoad]] i32 [[Phi]]
; CHECK-NOT: {{%.*}} = phi


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline norecurse nounwind uwtable
define dso_local i32 @foo(i32* nocapture %arr, i32 %n1, i32 %n2, i32 %sum) local_unnamed_addr #0 {
entry:
  %mul = mul nsw i32 %n2, %n1
  %add = add i32 %mul, 3
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %sum.addr.025 = phi i32 [ %sum, %entry ], [ %add2, %for.body ]
  %0 = trunc i64 %indvars.iv to i32
  %add1 = add i32 %add, %0
  %idxprom = sext i32 %add1 to i64
  %arrayidx = getelementptr inbounds i32, i32* %arr, i64 %idxprom
  %1 = load i32, i32* %arrayidx, align 4, !tbaa !2
  %add2 = add nsw i32 %1, %sum.addr.025
  store i32 %add1, i32* %arrayidx, align 4, !tbaa !2
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  %add11 = add nsw i32 %1, %add2
  ret i32 %add11
}

attributes #0 = { noinline norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!2 = !{!3, !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
