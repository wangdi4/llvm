; RUN: opt -passes="vplan-vec" -disable-output -debug-only=VPlanEvaluator -mtriple=x86_64 -vplan-unmasked-remainder-gain-threshold=0 < %s 2>&1 | FileCheck %s

; CHECK: Remainder evaluator: unmasked VPlan for VF=4 is not normalized
; CHECK: Remainder evaluator: unmasked VPlan for VF=2 is not normalized
;
define void @foo(ptr %lp, i32 %N) {
entry:
  %N2 = udiv i32 %N, 4
  br label %prehead

prehead:
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 8)]
  br label %for.body

for.body:                                         ; preds = %entry, %for.body
  %l1.05 = phi i32 [ 1, %prehead], [ %inc, %for.body ]
  %arrayidx = getelementptr inbounds i32, ptr %lp, i32 %l1.05
  %l = load i32, ptr %arrayidx
  %a = add i32 %l, %l1.05
  store i32 %a, ptr %arrayidx, align 8
  %inc = add nuw nsw i32 %l1.05, 1
  %exitcond.not = icmp ult i32 %inc, %N2
  br i1 %exitcond.not, label %for.body, label %for.end

for.end:                                          ; preds = %for.body
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"()]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry()

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token %0)
