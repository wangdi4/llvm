; Check that VPlan HIR decomposer is able to handle HLIf nodes with multiple predicates (of Integer type).
; The test also checks the correctness of code generation for this scenario.
; Input LLVM-IR generated with command: icx -O2 -mllvm -print-module-before-loopopt

; int  foo(int *arr, int n1)
; {
;     int index;
;
; #pragma vector always
; #pragma ivdep
;     for (index = 0; index < 1024; index++) {
;         if (arr[index] > 0 && index < 512) {
;             arr[index + n1] = index + n1 * n1 + 3;
;         }
;     }
;     return arr[0];
; }

; RUN: opt -hir-ssa-deconstruction -hir-vec-dir-insert -VPlanDriverHIR -vplan-print-after-simplify-cfg < %s 2>&1 | FileCheck %s --check-prefix=HCFG

; Check plain HCFG
; HCFG: i1 [[Cmp1:%.*]] = icmp i64 {{%.*}} i64 512
; HCFG-NEXT: i1 [[Cmp2:%.*]] = icmp i32 {{%.*}} i32 0
; HCFG-NEXT: i1 [[And:%.*]] = and i1 [[Cmp1]] i1 [[Cmp2]]
; HCFG: Condition({{BB.*}}): [DA: Divergent] i1 [[And]] = and i1 [[Cmp1]] i1 [[Cmp2]]

; NOTE: We force VF=4 here to test correctness of code generation
; RUN: opt -hir-ssa-deconstruction -hir-vec-dir-insert -VPlanDriverHIR -vplan-force-vf=4 -print-after=VPlanDriverHIR -S < %s 2>&1 | FileCheck %s

; Check HIR
; CHECK: DO i1 = 0, 1023, 4   <DO_LOOP>
; CHECK: [[WideCmp1:%.*]] = i1 + <i64 0, i64 1, i64 2, i64 3> <u 512;
; CHECK-NEXT: [[WideCmp2:%.*]] = {{%.*}} > 0;
; CHECK-NEXT: [[WideAnd:%.*]] = [[WideCmp1]]  &&  [[WideCmp2]];
; CHECK: Mask = @{[[WideAnd]]}


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable
define dso_local i32 @foo(i32* nocapture %arr, i32 %n1) local_unnamed_addr #0 {
entry:
  %mul = mul nsw i32 %n1, %n1
  %add = add nuw i32 %mul, 3
  %0 = sext i32 %n1 to i64
  br label %for.body

for.body:                                         ; preds = %for.inc, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.inc ]
  %arrayidx = getelementptr inbounds i32, i32* %arr, i64 %indvars.iv
  %1 = load i32, i32* %arrayidx, align 4, !tbaa !2
  %cmp1 = icmp sgt i32 %1, 0
  %cmp2 = icmp ult i64 %indvars.iv, 512
  %or.cond = and i1 %cmp2, %cmp1
  br i1 %or.cond, label %if.then, label %for.inc

if.then:                                          ; preds = %for.body
  %2 = trunc i64 %indvars.iv to i32
  %add3 = add i32 %add, %2
  %3 = add nsw i64 %indvars.iv, %0
  %arrayidx6 = getelementptr inbounds i32, i32* %arr, i64 %3
  store i32 %add3, i32* %arrayidx6, align 4, !tbaa !2
  br label %for.inc

for.inc:                                          ; preds = %for.body, %if.then
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond, label %for.end, label %for.body, !llvm.loop !6

for.end:                                          ; preds = %for.inc
  %4 = load i32, i32* %arr, align 4, !tbaa !2
  ret i32 %4
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }


!2 = !{!3, !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
!6 = distinct !{!6, !7, !8, !9}
!7 = !{!"llvm.loop.vectorize.ivdep_back"}
!8 = !{!"llvm.loop.vectorize.ignore_profitability"}
!9 = !{!"llvm.loop.vectorize.enable", i1 true}

