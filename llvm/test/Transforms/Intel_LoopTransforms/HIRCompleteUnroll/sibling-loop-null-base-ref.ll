; RUN: opt -hir-ssa-deconstruction -hir-post-vec-complete-unroll -print-before=hir-post-vec-complete-unroll -print-after=hir-post-vec-complete-unroll  2>&1 < %s | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,print<hir>,hir-post-vec-complete-unroll,print<hir>"  2>&1 < %s | FileCheck %s

; Complete unroll's profitability analysis was choking on (null)[0] (null base
; pointer) in the second region while analyzing the alloca store in the first
; loop.

; CHECK: Function: foo

; CHECK: BEGIN REGION { }
; CHECK: + DO i1 = 0, 1, 1   <DO_LOOP>
; CHECK: |   (%hasPrefixCallbacks)[0][i1].0.0.0.0 = null;
; CHECK: + END LOOP
; CHECK: END REGION

; CHECK: BEGIN REGION { }
; CHECK: + DO i1 = 0, 2305843009213693951, 1   <DO_LOOP>
; CHECK: |   (null)[0] = undef;
; CHECK: + END LOOP
; CHECK: END REGION


; CHECK: Function: foo

; CHECK: BEGIN REGION { modified }
; CHECK: (%hasPrefixCallbacks)[0][0].0.0.0.0 = null;
; CHECK: (%hasPrefixCallbacks)[0][1].0.0.0.0 = null;
; CHECK: END REGION


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct1 = type { %struct2 }
%struct2 = type { %struct3 }
%struct3 = type { %struct4 }
%struct4 = type { %struct5* }
%struct5 = type { i32 (...)** }

define hidden fastcc void @foo() {
entry:
  %hasPrefixCallbacks = alloca [2 x %struct1], align 16
  %arrayinit.begin7 = getelementptr inbounds [2 x %struct1], [2 x %struct1]* %hasPrefixCallbacks, i64 0, i64 0
  %add.ptr1595 = getelementptr inbounds [2 x %struct1], [2 x %struct1]* %hasPrefixCallbacks, i64 0, i64 2
  br label %for.body1605

for.body1605:                     ; preds = %for.body1605, %entry
  %__first.sroa.0.0111600 = phi %struct1* [ %incdec.ptr1602, %for.body1605 ], [ %arrayinit.begin7, %entry ]
  %_M_head_impl1601 = getelementptr inbounds %struct1, %struct1* %__first.sroa.0.0111600, i64 0, i32 0, i32 0, i32 0, i32 0
  store %struct5* null, %struct5** %_M_head_impl1601, align 8
  %incdec.ptr1602 = getelementptr inbounds %struct1, %struct1* %__first.sroa.0.0111600, i64 1
  %cmp1604 = icmp eq %struct1* %incdec.ptr1602, %add.ptr1595
  br i1 %cmp1604, label %exit, label %for.body1605

exit:                             ; preds = %for.body1605
  br label %for.body18398

for.body18398:                      ; preds = %for.body18398, %exit
  %__first.sroa.0.01118393 = phi %struct1* [ %incdec.ptr18395, %for.body18398 ], [ undef, %exit ]
  store i64 undef, i64* null, align 8
  %incdec.ptr18395 = getelementptr inbounds %struct1, %struct1* %__first.sroa.0.01118393, i64 1
  %cmp18397 = icmp eq %struct1* %incdec.ptr18395, undef
  br i1 %cmp18397, label %for.body.preheader18400, label %for.body18398

for.body.preheader18400:              ; preds = %for.body18398
  unreachable
}

