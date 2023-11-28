; RUN: opt -disable-output -enable-intel-advanced-opts  -xmain-opt-level=3 -passes="hir-ssa-deconstruction,hir-runtime-dd,print<hir>" -aa-pipeline="basic-aa" < %s 2>&1 | FileCheck %s

; Verify that with certain option levels, MV is enabled without aliasing among baseptr.
; Notice that ptrs %p and %q have noalias attribute. No checks for overlapping address spaces
; are done. However, MV based on validity conditions for delinearization of MIV memrefs are done.
; This MV can help later optimization passes such as loop blocking.


; BEGIN REGION { }
;       + DO i1 = 0, %UB1, 1   <DO_LOOP>
;       |   + DO i2 = 0, %UB2, 1   <DO_LOOP>
;       |   |   + DO i3 = 0, %UB3, 1   <DO_LOOP>
;       |   |   |   (%p)[(%d2 * %d1) * i1 + %d2 * i2 + i3] = (%q)[i3];
;       |   |   |   (%p)[(%d2 * %d1) * i1 + %d2 * i2 + i3 + %d2 - 2] = (%q)[i3];
;       |   |   + END LOOP
;       |   + END LOOP
;       + END LOOP
; END REGION

; To
; BEGIN REGION { }
;       if (%d2 > 1 & %UB3 + 2 < %d2 & %d1 > 1 & %UB2 + 1 < %d1)  <MVTag: 42>
;       {
;          + DO i1 = 0, %UB1, 1   <DO_LOOP>  <MVTag: 42, Delinearized: %p>
;          |   + DO i2 = 0, %UB2, 1   <DO_LOOP>  <MVTag: 43>
;          |   |   + DO i3 = 0, %UB3, 1   <DO_LOOP>  <MVTag: 44>
;          |   |   |   (%p)[(%d2 * %d1) * i1 + %d2 * i2 + i3] = (%q)[i3];
;          |   |   |   (%p)[(%d2 * %d1) * i1 + %d2 * i2 + i3 + %d2 + -2] = (%q)[i3];
;          |   |   + END LOOP
;          |   + END LOOP
;          + END LOOP
;       }
;       else
;       {
;          + DO i1 = 0, %UB1, 1   <DO_LOOP>  <MVTag: 42> <nounroll> <novectorize>
;          |   + DO i2 = 0, %UB2, 1   <DO_LOOP>  <MVTag: 43> <nounroll> <novectorize>
;          |   |   + DO i3 = 0, %UB3, 1   <DO_LOOP>  <MVTag: 44> <nounroll> <novectorize>
;          |   |   |   (%p)[(%d2 * %d1) * i1 + %d2 * i2 + i3] = (%q)[i3];
;          |   |   |   (%p)[(%d2 * %d1) * i1 + %d2 * i2 + i3 + %d2 + -2] = (%q)[i3];
;          |   |   + END LOOP
;          |   + END LOOP
;          + END LOOP
;       }
; END REGION

; Verify that no tests for aliasing are generated.
;
; CHECK-NOT: %mv.and == 0

; Verify that validity conditions from delinearizations are generated, and loop is MVed.
;
; CHECK: if (%d2 > 1 & %UB3 + 2 < %d2 & %d1 > 1 & %UB2 + 1 < %d1)  <MVTag: [[TAG_NUM:[0-9]+]]>
; CHECK:          DO i1 = 0, %UB1, 1   <DO_LOOP>  <MVTag: [[TAG_NUM]], Delinearized: %p>

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo(ptr nocapture noalias %p, ptr nocapture readonly noalias %q, i64 %d2, i64 %d1, i64 %UB1, i64 %UB2, i64 %UB3) {
entry:
  %mul = mul i64 %d1, %d2
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.cond.cleanup3, %entry
  %k.049 = phi i64 [ 0, %entry ], [ %inc25, %for.cond.cleanup3 ]
  %mul9 = mul i64 %mul, %k.049
  br label %for.cond5.preheader

for.cond.cleanup:                                 ; preds = %for.cond.cleanup3
  ret void

for.cond5.preheader:                              ; preds = %for.cond.cleanup7, %for.cond1.preheader
  %j.048 = phi i64 [ 0, %for.cond1.preheader ], [ %add16, %for.cond.cleanup7 ]
  %mul10 = mul i64 %j.048, %d2
  %add16 = add nuw nsw i64 %j.048, 1
  %mul17_ = mul i64 %add16, %d2
  %mul17 = add i64 %mul17_, -2
  br label %for.body8

for.cond.cleanup3:                                ; preds = %for.cond.cleanup7
  %inc25 = add nuw nsw i64 %k.049, 1
  %exitcond53 = icmp ugt i64 %inc25, %UB1
  br i1 %exitcond53, label %for.cond.cleanup, label %for.cond1.preheader

for.cond.cleanup7:                                ; preds = %for.body8
  %exitcond52 = icmp ugt i64 %add16, %UB2
  br i1 %exitcond52, label %for.cond.cleanup3, label %for.cond5.preheader

for.body8:                                        ; preds = %for.body8, %for.cond5.preheader
  %i.047 = phi i64 [ 0, %for.cond5.preheader ], [ %inc, %for.body8 ]
  %arrayidx = getelementptr inbounds i32, ptr %q, i64 %i.047
  %0 = load i32, ptr %arrayidx, align 4
  %add = add i64 %i.047, %mul9
  %add11 = add i64 %add, %mul10
  %arrayidx12 = getelementptr inbounds i32, ptr %p, i64 %add11
  store i32 %0, ptr %arrayidx12, align 4
  %1 = load i32, ptr %arrayidx, align 4
  %add19 = add i64 %add, %mul17
  %arrayidx20 = getelementptr inbounds i32, ptr %p, i64 %add19
  store i32 %1, ptr %arrayidx20, align 4
  %inc = add nuw nsw i64 %i.047, 1
  %exitcond = icmp ugt i64 %inc, %UB3
  br i1 %exitcond, label %for.cond.cleanup7, label %for.body8
}

