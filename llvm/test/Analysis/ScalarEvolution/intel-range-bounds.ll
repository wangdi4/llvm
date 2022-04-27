; REQUIRES: asserts
; RUN: opt -analyze -enable-new-pm=0 -scalar-evolution <%s -o - -S -scalar-evolution-print-loop-range-bounds | FileCheck %s
; RUN: opt < %s -o - -S -scalar-evolution-print-loop-range-bounds "-passes=print<scalar-evolution>" 2>&1 | FileCheck %s
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Check that we can derive the maximum bounds for loops such as
; val = 0;
; for (i = 0; i < N; i++)
;   if (cond())
;     val += 1;
; // val must be in the bounds [0, N) here.

; CHECK-LABEL: @forwards
; CHECK:  %idx.phi = phi i64 [ %idx.next, %for.body3.end ], [ 0, %entry ]
; CHECK-NEXT:    -->  %idx.phi
; CHECK-SAME: Refined range bounds: [0,5000)

define void @forwards() {
entry:
  br label %for.body3

for.body3:                                        ; preds = %entry, %for.body3.end
  %indvars.iv51 = phi i64 [ %indvars.iv.next52, %for.body3.end ], [ 0, %entry ]
  %idx.phi = phi i64 [ %idx.next, %for.body3.end ], [ 0, %entry ]
  %cond = call i1 @unpredictable()
  br i1 %cond, label %do.inc, label %for.body3.end

do.inc:                                           ; preds = %for.body3
  %idx.add = add i64 %idx.phi, 1
  br label %for.body3.end

for.body3.end:                                    ; preds = %for.body3, %do.inc
  %idx.next = phi i64 [ %idx.add, %do.inc ], [ %idx.phi, %for.body3]
  %indvars.iv.next52 = add nuw nsw i64 %indvars.iv51, 1
  %exitcond53 = icmp eq i64 %indvars.iv.next52, 5000
  br i1 %exitcond53, label %for.end28, label %for.body3

for.end28:                                        ; preds = %for.body23
  ret void
}

; CHECK-LABEL: @backwards
; CHECK:  %idx.phi = phi i64 [ %idx.next, %for.body3.end ], [ 4999, %entry ]
; CHECK-NEXT:    -->  %idx.phi
; CHECK-SAME: Refined range bounds: [0,5000)

define void @backwards() {
entry:
  br label %for.body3

for.body3:                                        ; preds = %entry, %for.body3.end
  %indvars.iv51 = phi i64 [ %indvars.iv.next52, %for.body3.end ], [ 0, %entry ]
  %idx.phi = phi i64 [ %idx.next, %for.body3.end ], [ 4999, %entry ]
  %cond = call i1 @unpredictable()
  br i1 %cond, label %do.inc, label %for.body3.end

do.inc:                                           ; preds = %for.body3
  %idx.add = add i64 %idx.phi, -1
  br label %for.body3.end

for.body3.end:                                    ; preds = %for.body3, %do.inc
  %idx.next = phi i64 [ %idx.add, %do.inc ], [ %idx.phi, %for.body3]
  %indvars.iv.next52 = add nuw nsw i64 %indvars.iv51, 1
  %exitcond53 = icmp eq i64 %indvars.iv.next52, 5000
  br i1 %exitcond53, label %for.end28, label %for.body3

for.end28:                                        ; preds = %for.body23
  ret void
}

declare i1 @unpredictable()
