; RUN: opt -aa-pipeline="basic-aa" -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-memory-reduction-sinking,print<hir>"  2>&1 < %s | FileCheck %s

; This test case checks that the reduction with multiplication inside an If
; was transformed correctly. The compare operator for the If outside of the
; loop should be not equal to 1.

; HIR before transformation

; BEGIN REGION { }
;       + DO i1 = 0, %n + -1, 1   <DO_LOOP>
;       |   if ((%t14)[i1] >u 1.000000e+01)
;       |   {
;       |      %mul59 = %mul52  *  (%t13)[%t];
;       |      (%t13)[%t] = %mul59;
;       |   }
;       + END LOOP
; END REGION

; HIR after transformation

; CHECK: BEGIN REGION { modified }
; CHECK:       %tmp = 1.000000e+00;
; CHECK:       + DO i1 = 0, %n + -1, 1   <DO_LOOP>
; CHECK:       |   if ((%t14)[i1] >u 1.000000e+01)
; CHECK:       |   {
; CHECK:       |      %tmp = %tmp  *  %mul52;
; CHECK:       |   }
; CHECK:       + END LOOP
; CHECK:       if (%tmp !=u 1.000000e+00)
; CHECK:       {
; CHECK:          %mul59 = %tmp  *  (%t13)[%t];
; CHECK:          (%t13)[%t] = %mul59;
; CHECK:       }
; CHECK: END REGION


define void @foo(ptr noalias %t13, ptr noalias %t14, float %mul52, i64 %n, i64 %t) {
entry:
  br label %for.body56

for.body56:                                       ; preds = %for.body56, %entry
  %indvars.iv158 = phi i64 [ 0, %entry ], [ %indvars.iv.next159, %if.end ]
  %arrayidx58 = getelementptr inbounds float, ptr %t13, i64 %t
  %arrayidx = getelementptr inbounds float, ptr %t14, i64 %indvars.iv158
  %0 = load float, ptr %arrayidx
  %cmp1 = fcmp ugt float %0, 10.0
  br i1 %cmp1, label %if.then, label %if.end

if.then:
  %t15 = load float, ptr %arrayidx58, align 4
  %mul59 = fmul fast float %mul52, %t15
  store float %mul59, ptr %arrayidx58, align 4
  br label %if.end

if.end:
  %indvars.iv.next159 = add nuw nsw i64 %indvars.iv158, 1
  %exitcond161 = icmp eq i64 %indvars.iv.next159, %n
  br i1 %exitcond161, label %for.end62.loopexit, label %for.body56

for.end62.loopexit:
  ret void
}