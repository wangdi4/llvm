; RUN: opt -hir-ssa-deconstruction -hir-vec-dir-insert -hir-vplan-vec -disable-output -vplan-force-vf=4 -debug-only=vplan-alignment-analysis < %s 2>&1 | FileCheck %s --check-prefix=DEFAULT
; RUN: opt -passes="hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec" -disable-output -vplan-force-vf=4 -debug-only=vplan-alignment-analysis < %s 2>&1 | FileCheck %s --check-prefix=DEFAULT
; RUN: opt -hir-ssa-deconstruction -hir-vec-dir-insert -hir-vplan-vec -disable-output -vplan-force-vf=4 -vplan-cm-store-cost-adjustment=10.0 -debug-only=vplan-alignment-analysis < %s 2>&1 | FileCheck %s --check-prefix=ADJCOST
; RUN: opt -passes="hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec" -disable-output -vplan-force-vf=4 -vplan-cm-store-cost-adjustment=10.0 -debug-only=vplan-alignment-analysis < %s 2>&1 | FileCheck %s --check-prefix=ADJCOST
;
; LIT test to check cost model load/store adjustment options.
;
; REQUIRES: asserts
;
; DEFAULT: Dynamic peeling selection (1 items)
; DEFAULT: store i64 %vp{{.*}} i64* %vp{{.*}} profit=0.375
;
; ADJCOST: Dynamic peeling selection (1 items)
; ADJCOST: store i64 %vp{{.*}} i64* %vp{{.*}} profit=3.75
;
define void @foo(i64* %lp, i64 %n1) {
entry:
  %cmp5 = icmp sgt i64 %n1, 0
  br i1 %cmp5, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.body
  %l1.06 = phi i64 [ %inc, %for.body ], [ 0, %for.body.preheader ]
  %arrayidx = getelementptr inbounds i64, i64* %lp, i64 %l1.06
  store i64 %l1.06, i64* %arrayidx, align 8
  %inc = add nuw nsw i64 %l1.06, 1
  %exitcond.not = icmp eq i64 %inc, %n1
  br i1 %exitcond.not, label %for.end.loopexit, label %for.body, !llvm.loop !0

for.end.loopexit:                                 ; preds = %for.body
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  ret void
}

!0 = distinct !{!0, !1}
!1 = !{!"llvm.loop.intel.vector.dynamic_align", !"true"}
