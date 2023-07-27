; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced
;
; RUN: opt -disable-output -passes='hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec,print<hir>' -vplan-cost-model-print-analysis-for-vf=2,4,8,16 -mattr=avx2 -mtriple=x86_64-unknown-unknown -enable-intel-advanced-opts < %s 2>&1 | FileCheck %s
;
; LIT test to demonstrate inconsistency with VLS cost modeling and code generation.
; Even when cost modeling tells us that grouping certain accesses is not profitable,
; code generation uses VLS grouping for all accesses that can be grouped. We likely
; need to re-run VLS cost modeling heuristic before CG and apply VLS grouping for
; only profitable groups. Once the issue is fixed, we should not see any shuffles
; or wide VLS grouped stores in the generated code.
;
; Check that we see no cost adjustments from VLS heuristic
; CHECK-NOT: OVLS
;
; FIXME: Checks below should not have any shufflevector and or vls_mask computation
; CHECK:     DO i1 = 0, 1023, 16   <DO_LOOP> <auto-vectorized> <novectorize>
; CHECK-3:         {{.*}} = shufflevector
; CHECK:           %shuffle5 = shufflevector
; CHECK:           %vls.mask = shufflevector
; CHECK:           <32 x i64>*)(%lp2)[2 * i1] = %shuffle5, Mask = @{%vls.mask};
; CHECK:     END LOOP
;
define i64 @foo(ptr noalias %lp1, ptr noalias %lp2) {
entry:
  br label %for.body

for.body:                                         ; preds = %entry, %for.inc
  %l1.017 = phi i64 [ 0, %entry ], [ %inc, %for.inc ]
  %sum.016 = phi i64 [ 0, %entry ], [ %add, %for.inc ]
  %arrayidx = getelementptr inbounds i64, ptr %lp1, i64 %l1.017
  %0 = load i64, ptr %arrayidx, align 8
  %mul = mul nsw i64 %0, 17
  %add = add nsw i64 %mul, %sum.016
  %rem = and i64 %l1.017, 1
  %tobool.not = icmp eq i64 %rem, 0
  br i1 %tobool.not, label %for.inc, label %if.then

if.then:                                          ; preds = %for.body
  %mul1 = shl nuw nsw i64 %l1.017, 1
  %arrayidx2 = getelementptr inbounds i64, ptr %lp2, i64 %mul1
  store i64 %l1.017, ptr %arrayidx2, align 8
  %add5 = add nuw nsw i64 %mul1, 1
  %arrayidx6 = getelementptr inbounds i64, ptr %lp2, i64 %add5
  store i64 %mul1, ptr %arrayidx6, align 8
  br label %for.inc

for.inc:                                          ; preds = %for.body, %if.then
  %inc = add nuw nsw i64 %l1.017, 1
  %exitcond.not = icmp eq i64 %inc, 1024
  br i1 %exitcond.not, label %for.end, label %for.body

for.end:                                          ; preds = %for.inc
  ret i64 %add
}
; end INTEL_FEATURE_SW_ADVANCED
