; RUN: opt -hir-ssa-deconstruction -hir-vec-dir-insert -hir-vplan-vec -disable-output -print-after=hir-vplan-vec  -vplan-force-vf=4 -vplan-enable-new-cfg-merge-hir -hir-details-no-verbose-indent < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec,print<hir>" -disable-output -vplan-force-vf=4  -vplan-enable-new-cfg-merge-hir -hir-details-no-verbose-indent < %s 2>&1 | FileCheck %s
;
; LIT test to check dynamic peeling in VPlan HIR path for the case a[i] = a[i] + 1.
;
; CHECK:          BEGIN REGION { modified }
; CHECK-NEXT:           %.vec = ptrtoint.<4 x i64*>.<4 x i64>(&((<4 x i64*>)(%lp)[0]));
; CHECK-NEXT:           %.vec1 = %.vec  /u  8;
; CHECK-NEXT:           %.vec2 = %.vec1  *  3;
; CHECK-NEXT:           %.vec3 = %.vec2  %u  4;
; CHECK-NEXT:           %.vec4 = 0 == %.vec3;
; CHECK-NEXT:           %phi.temp = 0;
; CHECK-NEXT:           %extract.0. = extractelement %.vec4,  0;
; CHECK-NEXT:           if (%extract.0. == 1)
; CHECK-NEXT:           {
; CHECK-NEXT:              goto merge.blk14.25;
; CHECK-NEXT:           }
; CHECK-NEXT:           %.vec5 = %.vec3 + 4 >u 400;
; CHECK-NEXT:           %phi.temp6 = 0;
; CHECK-NEXT:           %extract.0.8 = extractelement %.vec5,  0;
; CHECK-NEXT:           if (%extract.0.8 == 1)
; CHECK-NEXT:           {
; CHECK-NEXT:              goto merge.blk12.33;
; CHECK-NEXT:           }
; CHECK-NEXT:           %extract.0.9 = extractelement %.vec3,  0;
; CHECK-NEXT:           %ub.tmp = %extract.0.9;
; CHECK-NEXT:           %peel.ub = %ub.tmp  -  1;

; CHECK:                + DO i1 = 0, %peel.ub, 1   <DO_LOOP>
; CHECK-NEXT:           |   %val = (%lp)[i1];
; CHECK-NEXT:           |   (%lp)[i1] = %val + 1;
; CHECK-NEXT:           + END LOOP

; CHECK:                %phi.temp = %ub.tmp;
; CHECK-NEXT:           merge.blk14.25:
; CHECK-NEXT:           %.vec11 = %.vec3 + 4 >u 400;
; CHECK-NEXT:           %phi.temp6 = %phi.temp;
; CHECK-NEXT:           %extract.0.13 = extractelement %.vec11,  0;
; CHECK-NEXT:           if (%extract.0.13 == 1)
; CHECK-NEXT:           {
; CHECK-NEXT:              goto merge.blk12.33;
; CHECK-NEXT:           }
; CHECK-NEXT:           %extract.0.14 = extractelement %.vec3,  0;
; CHECK-NEXT:           %adj.tc = 400  -  %extract.0.14;
; CHECK-NEXT:           %tgu = %adj.tc  /u  4;
; CHECK-NEXT:           %vec.tc = %tgu  *  4;
; CHECK-NEXT:           %extract.0.15 = extractelement %.vec3,  0;
; CHECK-NEXT:           %adj.tc16 = %vec.tc  +  %extract.0.15;

; CHECK:                + DO i1 = %phi.temp, %adj.tc16 + -1, 4   <DO_LOOP> <auto-vectorized> <nounroll> <novectorize>
; CHECK-NEXT:           |   %.vec17 = (<4 x i64>*)(%lp)[i1];
; CHECK-NEXT:           |   (<4 x i64>*)(%lp)[i1] = %.vec17 + 1;
; CHECK-NEXT:           + END LOOP

; CHECK:                %.vec18 = 400 == %adj.tc16;
; CHECK-NEXT:           %phi.temp6 = %adj.tc16;
; CHECK-NEXT:           %phi.temp20 = %adj.tc16;
; CHECK-NEXT:           %extract.0.22 = extractelement %.vec18,  0;
; CHECK-NEXT:           if (%extract.0.22 == 1)
; CHECK-NEXT:           {
; CHECK-NEXT:              goto final.merge.77;
; CHECK-NEXT:           }
; CHECK-NEXT:           merge.blk12.33:
; CHECK-NEXT:           %lb.tmp = %phi.temp6;

; CHECK:                + DO i1 = %lb.tmp, 399, 1   <DO_LOOP>
; CHECK-NEXT:           |   %val = (%lp)[i1];
; CHECK-NEXT:           |   (%lp)[i1] = %val + 1;
; CHECK-NEXT:           + END LOOP

; CHECK:                %phi.temp20 = 399;
; CHECK-NEXT:           final.merge.77:
; CHECK-NEXT:     END REGION
define void @foo(i64* %lp) {
entry:
  br label %for.body.preheader

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.body
  %l1.06 = phi i64 [ %inc, %for.body ], [ 0, %for.body.preheader ]
  %arrayidx = getelementptr inbounds i64, i64* %lp, i64 %l1.06
  %val = load i64, i64* %arrayidx, align 8
  %add = add i64 %val, 1
  store i64 %add, i64* %arrayidx, align 8
  %inc = add nuw nsw i64 %l1.06, 1
  %exitcond.not = icmp eq i64 %inc, 400
  br i1 %exitcond.not, label %for.end.loopexit, label %for.body, !llvm.loop !0

for.end.loopexit:                                 ; preds = %for.body
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  ret void
}

!0 = distinct !{!0, !1}
!1 = !{!"llvm.loop.intel.vector.dynamic_align", !"true"}
