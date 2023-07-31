; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-cond-ldst-motion,print<hir>" -aa-pipeline="basic-aa"  < %s -disable-output 2>&1 | FileCheck %s

; This test checks that HIRConditionalLoadStoreMotion is able to combine
; the hoist and sink variables into one variable since the bitcasts order
; matches. In other words, one variable will be used for hoist and sinking.

; HIR before transformation

; BEGIN REGION { }
;       + DO i1 = 0, 62, 1   <DO_LOOP>
;       |   if (%which != 0)
;       |   {
;       |      %Ai32.then = (i32*)(%A)[i1];
;       |      (float*)(%A)[i1] = 1.000000e+00;
;       |      (i32*)(%A)[i1] = %Ai32.then;
;       |   }
;       |   else
;       |   {
;       |      %Ai32.else = (i32*)(%A)[i1];
;       |      (float*)(%A)[i1] = 2.000000e+00;
;       |      (i32*)(%A)[i1] = %Ai32.else;
;       |   }
;       + END LOOP
; END REGION


; HIR after transformation

; CHECK: BEGIN REGION { }
; CHECK:       + DO i1 = 0, 62, 1   <DO_LOOP>
; CHECK:       |   %cldst.motioned = (i32*)(%A)[i1];
; CHECK:       |   if (%which != 0)
; CHECK:       |   {
; CHECK:       |      %Ai32.then = %cldst.motioned;
; CHECK:       |      %temp = 1.000000e+00;
; CHECK:       |      %cldst.motioned = bitcast.float.i32(%temp);
; CHECK:       |      %cldst.motioned = %Ai32.then;
; CHECK:       |   }
; CHECK:       |   else
; CHECK:       |   {
; CHECK:       |      %Ai32.else = %cldst.motioned;
; CHECK:       |      %temp4 = 2.000000e+00;
; CHECK:       |      %cldst.motioned = bitcast.float.i32(%temp4);
; CHECK:       |      %cldst.motioned = %Ai32.else;
; CHECK:       |   }
; CHECK:       |   (i32*)(%A)[i1] = %cldst.motioned;
; CHECK:       + END LOOP
; CHECK: END REGION


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @casts(ptr %A, ptr %B, i1 %which) {

entry:
  br label %L1

L1:
  %i = phi i64 [ 0, %entry ], [ %i.next, %L1.latch ]
  %Ai = getelementptr inbounds i64, ptr %A, i64 %i
  br i1 %which, label %then, label %else

then:
  %Ai32.then = load i32, ptr %Ai, align 8
  store float 1.0, ptr %Ai, align 8
  store i32 %Ai32.then, ptr %Ai, align 8
  br label %L1.latch

else:
  %Ai32.else = load i32, ptr %Ai, align 8
  store float 2.0, ptr %Ai, align 8
  store i32 %Ai32.else, ptr %Ai, align 8
  br label %L1.latch

L1.latch:
  %i.next = add nuw nsw i64 %i, 1
  %cond = icmp ne i64 %i.next, 63
  br i1 %cond, label %L1, label %exit

exit:
  ret void
}
