; RUN: opt -hir-ssa-deconstruction -hir-vec-dir-insert -hir-vplan-vec -disable-output -print-after=hir-vplan-vec  -vplan-force-vf=4  < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec,print<hir>" -disable-output -vplan-force-vf=4 < %s 2>&1 | FileCheck %s
;
; LIT test to make sure that we correctly vectorize a large trip count
; loop(trip count > unsigned_max). Due to issues in the way we were dealing
; with trip count in the planner and remainder evaluator, we incorrectly
; determined that a scalar remainder is needed and were trying to generate
; loops that look like:
;
;      + DO i1 = 0, 2305843009213693951, 4   <DO_LOOP>
;      |   (<4 x i64>*)(%lp)[i1] = i1 + <i64 0, i64 1, i64 2, i64 3>;
;      + END LOOP
;
;      + DO i1 = 2305843009213693952, 2305843009213693951, 1   <DO_LOOP>
;      |   (%lp)[i1] = i1;
;      + END LOOP
;
; The bounds of the scalar DO loop (lowerbound > upperbound) obviously do not
; make sense and this causes an assertion fail in HLLoop.cpp
;
; The fix uses uint64_t type to store trip count information and uses actual
; trip count while making remainder evaluations.
;
; CHECK:      BEGIN REGION { modified }
; CHECK-NEXT:      + DO i1 = 0, 2305843009213693951, 4   <DO_LOOP> <auto-vectorized> <novectorize>
; CHECK-NEXT:      |   (<4 x i64>*)(%lp)[i1] = i1 + <i64 0, i64 1, i64 2, i64 3>;
; CHECK-NEXT:      + END LOOP
; CHECK-NEXT: END REGION
;
define void @foo(i64* %lp) {
entry:
  br label %for.body

for.body:                                         ; preds = %entry, %for.body
  %l1.05 = phi i64 [ 0, %entry ], [ %inc, %for.body ]
  %arrayidx = getelementptr inbounds i64, i64* %lp, i64 %l1.05
  store i64 %l1.05, i64* %arrayidx, align 8
  %inc = add nuw nsw i64 %l1.05, 1
  %exitcond.not = icmp eq i64 %inc, 2305843009213693952
  br i1 %exitcond.not, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  ret void
}
