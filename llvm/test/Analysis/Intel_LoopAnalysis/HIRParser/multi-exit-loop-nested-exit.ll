; RUN: opt %s -passes="hir-ssa-deconstruction,print<hir-framework>" -hir-framework-debug=parser -disable-output  2>&1 | FileCheck %s


; Check that we able to handle early exit embedded inside nested ifs.

; CHECK: LiveOuts:
; CHECK-DAG: %t.1
; CHECK-DAG: %t.0.be

; CHECK: + DO i1 = 0, sext.i32.i64(%n) + -1, 1   <DO_MULTI_EXIT_LOOP>  <MAX_TC_EST = 2147483647>
; CHECK: |   %1 = (%A)[i1];
; CHECK: |   if (%1 > 0)
; CHECK: |   {
; CHECK: |      %t.1 = (%A)[i1 + 1];
; CHECK: |      %t.0.be = %t.1;
; CHECK: |      if (%1 == 4)
; CHECK: |      {
; CHECK: |         goto for.end.loopexit;
; CHECK: |      }
; CHECK: |   }
; CHECK: |   else
; CHECK: |   {
; CHECK: |      %t.123 = (%A)[i1 + 2];
; CHECK: |      %t.0.be = %t.123;
; CHECK: |   }
; CHECK: + END LOOP


; Check that we are able to generate code for the region successfully.
; RUN: opt %s -passes="hir-ssa-deconstruction,hir-cg" -force-hir-cg -S 2>&1 | FileCheck %s -check-prefix=CHECK-CG

; CHECK-CG: region.0

define i32 @voo(ptr nocapture readonly %A, i32 %n) {
entry:
  %cmp26 = icmp sgt i32 %n, 0
  br i1 %cmp26, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
  %0 = sext i32 %n to i64
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.cond.backedge
  %indvars.iv = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next, %for.cond.backedge ]
  %arrayidx = getelementptr inbounds i32, ptr %A, i64 %indvars.iv
  %1 = load i32, ptr %arrayidx, align 4
  %cmp1 = icmp sgt i32 %1, 0
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  br i1 %cmp1, label %if.then, label %if.else

if.then:                                          ; preds = %for.body
  %t.1.in = getelementptr inbounds i32, ptr %A, i64 %indvars.iv.next
  %t.1 = load i32, ptr %t.1.in, align 4
  %cmp9 = icmp eq i32 %1, 4
  br i1 %cmp9, label %for.end.loopexit, label %for.cond.backedge

if.else:                                          ; preds = %for.body
  %2 = add nuw nsw i64 %indvars.iv, 2
  %t.1.in22 = getelementptr inbounds i32, ptr %A, i64 %2
  %t.123 = load i32, ptr %t.1.in22, align 4
  br label %for.cond.backedge

for.cond.backedge:                                ; preds = %if.else, %if.then
  %t.0.be = phi i32 [ %t.123, %if.else ], [ %t.1, %if.then ]
  %cmp = icmp slt i64 %indvars.iv.next, %0
  br i1 %cmp, label %for.body, label %for.end.loopexit

for.end.loopexit:                                 ; preds = %if.then, %for.cond.backedge
  %t.2.ph = phi i32 [ %t.0.be, %for.cond.backedge ], [ %t.1, %if.then ]
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  %t.2 = phi i32 [ 0, %entry ], [ %t.2.ph, %for.end.loopexit ]
  ret i32 %t.2
}

