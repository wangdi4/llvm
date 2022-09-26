; RUN: opt -hir-ssa-deconstruction -hir-vec-dir-insert -hir-vplan-vec -disable-output -print-after=hir-vplan-vec  -vplan-force-vf=4 < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec,print<hir>" -disable-output -vplan-force-vf=4 < %s 2>&1 | FileCheck %s
;
; LIT test to check dynamic peeling in VPlan HIR path. The test specifically checks
; that having outer loop IVs as part of an array index works fine.
; Incoming HIR looks like:
; 
;         + DO i1 = 0, 399, 1   <DO_LOOP>
;         |   + DO i2 = 0, 399, 1   <DO_LOOP>
;         |   |   %entry.region = @llvm.directive.region.entry(); [ DIR.VPO.AUTO.VEC() ] 
;         |   |   
;         |   |   + DO i3 = 0, 399, 1   <DO_LOOP>
;         |   |   |   (%lp)[100 * i1 + i3] = i1 + i2 + i3;
;         |   |   + END LOOP
;         |   |   
;         |   |   @llvm.directive.region.exit(%entry.region); [ DIR.VPO.END.AUTO.VEC() ] 
;         |   + END LOOP
;         + END LOOP
;
; CHECK:           BEGIN REGION { modified }
; CHECK-NEXT:            + DO i1 = 0, 399, 1   <DO_LOOP>
; CHECK-NEXT:            |   + DO i2 = 0, 399, 1   <DO_LOOP>
; CHECK-NEXT:            |   |   %.vec = ptrtoint.<4 x i64*>.<4 x i64>(&((<4 x i64*>)(%lp)[100 * i1]));
; CHECK-NEXT:            |   |   %.vec3 = %.vec  /u  8;
; CHECK-NEXT:            |   |   %.vec4 = %.vec3  *  3;
; CHECK-NEXT:            |   |   %.vec5 = %.vec4  %u  4;
; CHECK-NEXT:            |   |   %.vec6 = 0 == %.vec5;
; CHECK-NEXT:            |   |   %phi.temp = 0;
; CHECK-NEXT:            |   |   %extract.0. = extractelement %.vec6,  0;
; CHECK-NEXT:            |   |   if (%extract.0. == 1)
; CHECK-NEXT:            |   |   {
; CHECK-NEXT:            |   |      goto [[MERGE_AFTER_PEEL:.*]];
; CHECK-NEXT:            |   |   }
;
define void @foo(i64* %lp) {
entry:
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %entry, %for.inc12
  %l1.028 = phi i64 [ 0, %entry ], [ %inc13, %for.inc12 ]
  %mul = mul nuw nsw i64 %l1.028, 100
  br label %for.cond4.preheader

for.cond4.preheader:                              ; preds = %for.cond1.preheader, %for.inc9
  %l2.027 = phi i64 [ 0, %for.cond1.preheader ], [ %inc10, %for.inc9 ]
  %add = add nuw nsw i64 %l2.027, %l1.028
  br label %for.body6

for.body6:                                        ; preds = %for.cond4.preheader, %for.body6
  %l3.026 = phi i64 [ 0, %for.cond4.preheader ], [ %inc, %for.body6 ]
  %add7 = add nuw nsw i64 %add, %l3.026
  %add8 = add nuw nsw i64 %l3.026, %mul
  %arrayidx = getelementptr inbounds i64, i64* %lp, i64 %add8
  store i64 %add7, i64* %arrayidx, align 8
  %inc = add nuw nsw i64 %l3.026, 1
  %exitcond.not = icmp eq i64 %inc, 400
  br i1 %exitcond.not, label %for.inc9, label %for.body6, !llvm.loop !0

for.inc9:                                         ; preds = %for.body6
  %inc10 = add nuw nsw i64 %l2.027, 1
  %exitcond29.not = icmp eq i64 %inc10, 400
  br i1 %exitcond29.not, label %for.inc12, label %for.cond4.preheader

for.inc12:                                        ; preds = %for.inc9
  %inc13 = add nuw nsw i64 %l1.028, 1
  %exitcond30.not = icmp eq i64 %inc13, 400
  br i1 %exitcond30.not, label %for.end14, label %for.cond1.preheader

for.end14:                                        ; preds = %for.inc12
  ret void
}

!0 = distinct !{!0, !1}
!1 = !{!"llvm.loop.intel.vector.dynamic_align", !"true"}
