; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-cond-ldst-motion,print<hir>" -aa-pipeline="basic-aa"  < %s -disable-output 2>&1 | FileCheck %s

; This test checks that HIRConditionalLoadStoreMotion is able to appropriately
; bitcast between matching values that support it and not choke on ones that
; don't when sinking.

; Print before:

; CHECK: BEGIN REGION
; CHECK:       + DO i1 = 0, 62, 1   <DO_LOOP>
; CHECK:       |   if (%which != 0)
; CHECK:       |   {
; CHECK:       |      (i32*)(%B)[i1 + 1] = (i32*)(%A)[i1];
; CHECK:       |      (float*)(%B)[i1 + 1] = %Afloat.then;
; CHECK:       |   }
; CHECK:       |   else
; CHECK:       |   {
; CHECK:       |      (i32*)(%B)[i1 + 1] = (i32*)(%A)[i1];
; CHECK:       |      (float*)(%B)[i1 + 1] = %Afloat.else;
; CHECK:       |   }
; CHECK:       + END LOOP
; CHECK: END REGION

; Print after:

; CHECK: BEGIN REGION
; CHECK:       + DO i1 = 0, 62, 1   <DO_LOOP>
; CHECK:       |   if (%which != 0)
; CHECK:       |   {
; CHECK:       |      %[[SUNK32:[A-za-z0-9_.]+]] = bitcast.i32.float(%{{[A-Za-z0-9_.]+}});
; CHECK:       |      %[[SUNK32]] = %Afloat.then;
; CHECK:       |   }
; CHECK:       |   else
; CHECK:       |   {
; CHECK:       |      %[[SUNK32]] = bitcast.i32.float(%{{[A-Za-z0-9_.]+}});
; CHECK:       |      %[[SUNK32]] = %Afloat.else;
; CHECK:       |   }
; CHECK:       |   (float*)(%B)[i1 + 1] = %[[SUNK32]];
; CHECK:       + END LOOP
; CHECK: END REGION

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @casts(i64* %A, i64* %B, i1 %which) {

entry:
  br label %L1

L1:
  %i = phi i64 [ 0, %entry ], [ %i.next, %L1.latch ]
  %Ai = getelementptr inbounds i64, i64* %A, i64 %i
  %Ai32p = bitcast i64* %Ai to i32*
  %Afloatp = bitcast i64* %Ai to float*
  %Bi0 = getelementptr inbounds i64, i64* %B, i64 %i
  %i1 = add nuw nsw i64 %i, 1
  %Bi1 = getelementptr inbounds i64, i64* %B, i64 %i1
  %Bi32p = bitcast i64* %Bi1 to i32*
  %Bfloatp = bitcast i64* %Bi1 to float*
  br i1 %which, label %then, label %else

then:
  %Ai32.then = load i32, i32* %Ai32p, align 8
  %Afloat.then = load float, float* %Afloatp, align 8
  store i32 %Ai32.then, i32* %Bi32p, align 8
  store float %Afloat.then, float* %Bfloatp, align 8
  br label %L1.latch

else:
  %Ai32.else = load i32, i32* %Ai32p, align 8
  %Afloat.else = load float, float* %Afloatp, align 8
  store i32 %Ai32.else, i32* %Bi32p, align 8
  store float %Afloat.else, float* %Bfloatp, align 8
  br label %L1.latch

L1.latch:
  %i.next = add nuw nsw i64 %i, 1
  %cond = icmp ne i64 %i.next, 63
  br i1 %cond, label %L1, label %exit

exit:
  ret void
}
