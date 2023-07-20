; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced
;
; RUN: opt -disable-output -passes='hir-ssa-deconstruction,hir-vplan-vec' -vplan-cost-model-print-analysis-for-vf=4 -mattr=avx2 -mtriple=x86_64-unknown-unknown -enable-intel-advanced-opts < %s 2>&1 | FileCheck %s
;
; LIT test to demonstrate issue with cost modeling of masked accesses in VLS heuristic.
; We are not accounting for the access being masked when estimating the gain from VLS
; grouping which can incorrectly cause us to vectorize a loop as we over estimate the
; gain from vectorization.
;
; FIXME: The stores below are masked. The checks below show that the combined cost of
; the two strided stores to lp2[2*i1] and lp2[2*i1+1] is 6. The cost of one unit-stride
; store to lp1[i1] is 8 which is more than the cost of the two non-unit strided stores.
;
define void @foo(ptr noalias %lp1, ptr noalias %lp2) {
; CHECK-LABEL:  Cost Model for VPlan foo:HIR.#{{[0-9]+}} with VF = 4:
; CHECK:         Cost 0 for i1 {{%.*}} = block-predicate i1 {{%.*}}
; CHECK-NEXT:    Cost 0 for ptr [[VP_SUBSCRIPT:%.*]] = subscript inbounds ptr %lp1 i64 [[VP0:%.*]]
; CHECK-NEXT:    Cost 8 for store i64 [[VP0]] ptr [[VP_SUBSCRIPT]]
; CHECK-NEXT:    Cost 6 for i64 [[VP7:%.*]] = mul i64 2 i64 [[VP0]]
; CHECK-NEXT:    Cost 0 for ptr [[VP_SUBSCRIPT_1:%.*]] = subscript inbounds ptr %lp2 i64 [[VP7]]
; CHECK-NEXT:    Cost 19 for store i64 [[VP0]] ptr [[VP_SUBSCRIPT_1]] *OVLS*(-19) AdjCost: 0
; CHECK-NEXT:    Cost 1 for i64 [[VP8:%.*]] = add i64 [[VP7]] i64 1
; CHECK-NEXT:    Cost 0 for ptr [[VP_SUBSCRIPT_2:%.*]] = subscript inbounds ptr %lp2 i64 [[VP8]]
; CHECK-NEXT:    Cost 19 for store i64 [[VP7]] ptr [[VP_SUBSCRIPT_2]] *OVLS*(-13) AdjCost: 6
; CHECK-NEXT:  Cost 0 for br {{BB.*}}
; CHECK-NEXT: {{BB.*}}: base cost: 21
;
entry:
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %for.body

for.body:
  %l1.014 = phi i64 [ 0, %entry ], [ %inc, %for.inc ]
  %rem = and i64 %l1.014, 1
  %tobool.not = icmp eq i64 %rem, 0
  br i1 %tobool.not, label %for.inc, label %if.then

if.then:
  %arrayidx = getelementptr inbounds i64, ptr %lp1, i64 %l1.014
  store i64 %l1.014, ptr %arrayidx, align 8
  %mul = shl nuw nsw i64 %l1.014, 1
  %arrayidx1 = getelementptr inbounds i64, ptr %lp2, i64 %mul
  store i64 %l1.014, ptr %arrayidx1, align 8
  %add = add nuw nsw i64 %mul, 1
  %arrayidx4 = getelementptr inbounds i64, ptr %lp2, i64 %add
  store i64 %mul, ptr %arrayidx4, align 8
  br label %for.inc

for.inc:
  %inc = add nuw nsw i64 %l1.014, 1
  %exitcond.not = icmp eq i64 %inc, 1024
  br i1 %exitcond.not, label %for.end, label %for.body

for.end:
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
; end INTEL_FEATURE_SW_ADVANCED
