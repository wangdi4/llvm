; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-pre-vec-complete-unroll,print<hir>" -aa-pipeline="basic-aa" 2>&1 < %s | FileCheck %s

; Verify that complete unroll honors ZTT of the inner loop which is more
; restrictive than what the UB implies when unrolling the loopnest.

; + DO i1 = 0, 33, 1   <DO_LOOP>

; |   This Ztt prevents inner loop from executing when (22 <= i1 <= 27)
; |   + Ztt: if (-1 * i1 + 35 <u 8)
; |   + DO i2 = 0, i1 + -22, 1   <DO_LOOP>  <MAX_TC_EST = 12>
; |   |   %1 = (%m)[0][-1 * i1 + i2 + 35];
; |   |   (%m)[0][-1 * i1 + i2 + 35] = -1 * %1 * i2 + (%1 * %nl.026);
; |   + END LOOP
; |      %nl.026 = %nl.026  +  -1 * i1 + 21;
; + END LOOP

; Verify that the first load from the unrolled loopnest is (%m)[0][7] and not
; (%m)[0][13];

; CHECK: BEGIN REGION { modified }
; CHECK-NEXT: %0 = (%m)[0][7];

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local i32 @main() local_unnamed_addr{
entry:
  %m = alloca [100 x i32], align 16
  call void @llvm.memset.p0.i64(ptr nonnull align 16 dereferenceable(400) %m, i8 0, i64 400, i1 false)
  br label %for.body

for.body:                                         ; preds = %entry, %for.inc5
  %indvars.iv29 = phi i32 [ 21, %entry ], [ %indvars.iv.next30, %for.inc5 ]
  %indvars.iv = phi i64 [ 35, %entry ], [ %indvars.iv.next, %for.inc5 ]
  %nl.026 = phi i32 [ 0, %entry ], [ %nl.2, %for.inc5 ]
  %j0.023 = phi i32 [ 35, %entry ], [ %dec6, %for.inc5 ]
  %cmp1 = icmp ult i32 %j0.023, 8
  br i1 %cmp1, label %if.then, label %for.inc5

if.then:                                          ; preds = %for.body
  br label %for.body4

for.body4:                                        ; preds = %if.then, %for.body4
  %indvars.iv27 = phi i64 [ %indvars.iv, %if.then ], [ %indvars.iv.next28, %for.body4 ]
  %nl.122 = phi i32 [ %nl.026, %if.then ], [ %dec, %for.body4 ]
  %dec = add i32 %nl.122, -1
  %arrayidx = getelementptr inbounds [100 x i32], ptr %m, i64 0, i64 %indvars.iv27
  %0 = load i32, ptr %arrayidx, align 4
  %mul = mul i32 %0, %nl.122
  store i32 %mul, ptr %arrayidx, align 4
  %indvars.iv.next28 = add nuw nsw i64 %indvars.iv27, 1
  %exitcond = icmp eq i64 %indvars.iv.next28, 14
  br i1 %exitcond, label %for.inc5.loopexit, label %for.body4

for.inc5.loopexit:                                ; preds = %for.body4
  %1 = add i32 %nl.026, %indvars.iv29
  br label %for.inc5

for.inc5:                                         ; preds = %for.inc5.loopexit, %for.body
  %nl.2 = phi i32 [ %nl.026, %for.body ], [ %1, %for.inc5.loopexit ]
  %dec6 = add nsw i32 %j0.023, -1
  %cmp = icmp ugt i32 %dec6, 1
  %indvars.iv.next = add nsw i64 %indvars.iv, -1
  %indvars.iv.next30 = add nsw i32 %indvars.iv29, -1
  br i1 %cmp, label %for.body, label %for.end7

for.end7:                                         ; preds = %for.inc5
  %nl.2.lcssa = phi i32 [ %nl.2, %for.inc5 ]
  ret i32 0
}

declare void @llvm.memset.p0.i64(ptr nocapture writeonly, i8, i64, i1 immarg)

