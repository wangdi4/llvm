; RUN: opt -hir-ssa-deconstruction -hir-vec-dir-insert -hir-vplan-vec -disable-output -print-after=hir-vplan-vec  -vplan-force-vf=4 < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec,print<hir>" -disable-output -vplan-force-vf=4  < %s 2>&1 | FileCheck %s
;
; LIT test to check dynamic peeling in VPlan HIR path. The test specifically checks
; that having pointer diff as part of an array index does not cause any issues.
; Incoming HIR looks like:
;               + DO i1 = %lb.tmp, %n1 + -1, 1   <DO_LOOP>
;               |   (%lp)[i1 + %ptr.diff] = i1;
;               + END LOOP
; 
; CHECK:        BEGIN REGION { modified }
; CHECK-NEXT:         %.vec = ptrtoint.<4 x i64*>.<4 x i64>(&((<4 x i64*>)(%lp)[%ptr.diff]));
; CHECK-NEXT:         %.vec1 = %.vec  /u  8;
; CHECK-NEXT:         %.vec2 = %.vec1  *  3;
; CHECK-NEXT:         %.vec3 = %.vec2  %u  4;
; CHECK-NEXT:         %.vec4 = 0 == %.vec3;
; CHECK-NEXT:         %phi.temp = 0;
; CHECK-NEXT:         %extract.0. = extractelement %.vec4,  0;
; CHECK-NEXT:         if (%extract.0. == 1)
; CHECK-NEXT:         {
; CHECK-NEXT:            goto [[MERGE_AFTER_PEEL:.*]];
; CHECK-NEXT:         }
;
define void @foo(i64* %lp, i64 %n1, i64* %lp1, i64* %lp2) {
entry:
  %cmp5 = icmp sgt i64 %n1, 0
  %sub.ptr.lhs.cast = ptrtoint i64* %lp2 to i64
  %sub.ptr.rhs.cast = ptrtoint i64* %lp1 to i64
  %sub.ptr.sub = sub i64 %sub.ptr.lhs.cast, %sub.ptr.rhs.cast
  %ptr.diff = ashr exact i64 %sub.ptr.sub, 3
  br i1 %cmp5, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.body
  %l1.06 = phi i64 [ %inc, %for.body ], [ 0, %for.body.preheader ]
  %idx = add nuw nsw i64 %l1.06, %ptr.diff
  %arrayidx = getelementptr inbounds i64, i64* %lp, i64 %idx
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
