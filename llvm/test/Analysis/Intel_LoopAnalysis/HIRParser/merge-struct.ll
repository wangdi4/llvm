; RUN: opt < %s -passes="hir-ssa-deconstruction,print<hir>" 2>&1 | FileCheck %s

; Check that %tmp17 and %tmp18 would be merged, ommiting %tmp18 first dimension:
;   (%b)[0].0[undef]

; CHECK: BEGIN REGION { }
; CHECK:       + DO i1 = 0, %n + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>
; CHECK:       |   %tmp16 = (%b)[0].0[undef][1];
; CHECK:       |   (i32*)(%b)[0].0[undef] = undef;
; CHECK:       + END LOOP
; CHECK: END REGION

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.bar = type { [4 x [2 x float]] }

define dso_local void @foo(ptr %b, i32 %n) {
entry:
  %cmp1 = icmp slt i32 0, %n
  br i1 %cmp1, label %for.body.lr.ph, label %for.end

for.body.lr.ph:                                   ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.lr.ph, %for.inc
  %i.02 = phi i32 [ 0, %for.body.lr.ph ], [ %inc, %for.inc ]
  %tmp15 = getelementptr inbounds %struct.bar, ptr %b, i64 0, i32 0, i64 undef, i64 1
  %tmp16 = load float, ptr %tmp15, align 4
  %tmp17 = getelementptr inbounds %struct.bar, ptr %b, i64 0, i32 0
  %tmp18 = getelementptr inbounds [4 x [2 x float]], ptr %tmp17, i64 0, i64 undef
  %tmp19 = bitcast ptr %tmp18 to ptr
  store i32 undef, ptr %tmp19, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %inc = add nsw i32 %i.02, 1
  %cmp = icmp slt i32 %inc, %n
  %i.02.in = bitcast i32 %inc to i32
  br i1 %cmp, label %for.body, label %for.cond.for.end_crit_edge

for.cond.for.end_crit_edge:                       ; preds = %for.inc
  br label %for.end

for.end:                                          ; preds = %for.cond.for.end_crit_edge, %entry
  ret void
}

