; RUN: opt -aa-pipeline="basic-aa" -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-memory-reduction-sinking,print<hir>" 2>&1 < %s | FileCheck %s

; This test case checks that the reductions inside the multiple Ifs are sinked
; properly.

; HIR before transformation

; BEGIN REGION { }
;       + DO i1 = 0, 99, 1   <DO_LOOP>
;       |   + DO i2 = 0, %n + -1, 1   <DO_LOOP>
;       |   |   %0 = (%t14)[i2];
;       |   |   if (%0 >u 1.000000e+01)
;       |   |   {
;       |   |      %add = %mul52  +  (%t13)[5];
;       |   |      (%t13)[5] = %add;
;       |   |   }
;       |   |   if (%0 >u 1.120000e+02)
;       |   |   {
;       |   |      %1 = %0  +  (%t13)[i1 + 1];
;       |   |      (%t13)[i1 + 1] = %1;
;       |   |   }
;       |   + END LOOP
;       + END LOOP
; END REGION


; HIR after transformation

; CHECK: BEGIN REGION { modified }
; CHECK:       + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK:       |      %tmp = 0.000000e+00;
; CHECK:       |      %tmp3 = 0.000000e+00;
; CHECK:       |   + DO i2 = 0, %n + -1, 1   <DO_LOOP>
; CHECK:       |   |   %0 = (%t14)[i2];
; CHECK:       |   |   if (%0 >u 1.000000e+01)
; CHECK:       |   |   {
; CHECK:       |   |      %tmp3 = %tmp3  +  %mul52;
; CHECK:       |   |   }
; CHECK:       |   |   if (%0 >u 1.120000e+02)
; CHECK:       |   |   {
; CHECK:       |   |      %tmp = %tmp  +  %0;
; CHECK:       |   |   }
; CHECK:       |   + END LOOP
; CHECK:       |
; CHECK:       |   if (%tmp3 !=u 0.000000e+00)
; CHECK:       |   {
; CHECK:       |      %add = %tmp3  +  (%t13)[5];
; CHECK:       |      (%t13)[5] = %add;
; CHECK:       |   }
; CHECK:       |   if (%tmp !=u 0.000000e+00)
; CHECK:       |   {
; CHECK:       |      %1 = %tmp  +  (%t13)[i1 + 1];
; CHECK:       |      (%t13)[i1 + 1] = %1;
; CHECK:       |   }
; CHECK:       + END LOOP
; CHECK: END REGION




define void @foo(ptr noalias %t13, ptr noalias %t14, float %mul52, i64 %n) {
entry:
  br label %outer.header

outer.header:
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.end62.loopexit ]
  br label %for.body56

for.body56:                                       ; preds = %for.body56, %entry
  %indvars.iv158 = phi i64 [ 0, %outer.header ], [ %indvars.iv.next159, %if.next.end ]
  %arrayidx58 = getelementptr inbounds float, ptr %t13, i64 5
  %arrayidx = getelementptr inbounds float, ptr %t14, i64 %indvars.iv158
  %0 = load float, ptr %arrayidx
  %cmp1 = fcmp ugt float %0, 10.0
  br i1 %cmp1, label %if.then, label %if.end

if.then:
  %t15 = load float, ptr %arrayidx58, align 4
  %add = fadd fast float %mul52, %t15
  store float %add, ptr %arrayidx58, align 4
  br label %if.end

if.end:
  %cmp2 = fcmp ugt float %0, 112.0
  br i1 %cmp2, label %if.next.then, label %if.next.end

if.next.then:
  %pos = add i64 %indvars.iv, 1
  %arrayidx3 = getelementptr inbounds float, ptr %t13, i64 %pos
  %t16 = load float, ptr %arrayidx3, align 4
  %1 = fadd fast float %0, %t16
  store float %1, ptr %arrayidx3, align 4
  br label %if.next.end

if.next.end:
  %indvars.iv.next159 = add nuw nsw i64 %indvars.iv158, 1
  %exitcond161 = icmp eq i64 %indvars.iv.next159, %n
  br i1 %exitcond161, label %for.end62.loopexit, label %for.body56

for.end62.loopexit:
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond, label %loopexit, label %outer.header

loopexit:
  ret void
}