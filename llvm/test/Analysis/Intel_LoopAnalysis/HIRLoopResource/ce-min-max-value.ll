; REQUIRES: asserts
; RUN: opt < %s -passes="hir-ssa-deconstruction,print<hir-loop-resource>" -debug-only=hir-hlnode-utils -disable-output 2>&1 | FileCheck %s

; Test checks that getMinOrMaxValueImpl() encounter LegalMaxTC of the loop
; when calculating max canon expr value.

; HIR-
;  + DO i1 = 0, %n + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
;  |   %t.024 = 0;
;  |   + DO i2 = 0, i1 + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483646>  <LEGAL_MAX_TC = 2147483646>
;  |   |   %t.024.out = %t.024;
;  |   |   %0 = (%A)[i2];
;  |   |   %1 = (%B)[i2];
;  |   |   %t.024 = %0 + %t.024.out  +  %1;
;  |   + END LOOP
;  + END LOOP

; CHECK:       getMaxValue() called for i1 + -1
; CHECK:                 returned 2147483645

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo(ptr nocapture readonly %A, ptr nocapture readonly %B, i32 %n) {
entry:
  %cmp25 = icmp sgt i32 %n, 0
  br i1 %cmp25, label %for.outer.preheader, label %for.end10

for.outer.preheader:                    ; preds = %entry
  br label %for.outer

for.outer:                              ; preds = %for.outer.preheader, %for.end
  %i.026 = phi i32 [ %inc9, %for.end ], [ 0, %for.outer.preheader ]
  %ztt = icmp eq i32 %i.026, 0
  br i1 %ztt, label %for.end, label %for.body3.pre

for.body3.pre:
  br label %for.body3

for.body3:                                        ; preds = %for.body3, %for.body3.pre
  %indvars.iv = phi i64 [ 0, %for.body3.pre ], [ %indvars.iv.next, %for.body3 ]
  %t.024 = phi i32 [ 0, %for.body3.pre ], [ %add6, %for.body3 ]
  %arrayidx = getelementptr inbounds i32, ptr %A, i64 %indvars.iv
  %0 = load i32, ptr %arrayidx, align 4
  %arrayidx5 = getelementptr inbounds i32, ptr %B, i64 %indvars.iv
  %1 = load i32, ptr %arrayidx5, align 4
  %add = add i32 %0, %t.024
  %add6 = add i32 %add, %1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next to i32
  %exitcond = icmp eq i32 %lftr.wideiv, %i.026
  br i1 %exitcond, label %inner.exit, label %for.body3

inner.exit:
  %lcssa = phi i32 [ %add6, %for.body3 ]
  br label %for.end

for.end:                                          ; preds = %for.body3, %for.outer
  %lcssa.out = phi i32 [ %lcssa, %inner.exit ], [ 0, %for.outer ]
  %inc9 = add nuw nsw i32 %i.026, 1
  %exitcond29 = icmp eq i32 %inc9, %n
  br i1 %exitcond29, label %for.end10.loopexit, label %for.outer

for.end10.loopexit:                               ; preds = %for.end
  %lcssa.out.out = phi i32 [ %lcssa.out, %for.end ]
  br label %for.end10

for.end10:                                        ; preds = %for.end10.loopexit, %entry
  ret void
}
