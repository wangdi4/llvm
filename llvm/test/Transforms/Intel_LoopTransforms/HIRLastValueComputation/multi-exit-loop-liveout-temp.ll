; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-lmm,hir-last-value-computation,print<hir>" -aa-pipeline="basic-aa" 2>&1 < %s -disable-output | FileCheck %s

; Verify that the definition of %limm after the early exit is successfully
; sinked to loop postexit instead of getting removed.
; The current liveout analysis being done for each edge of the loop only works
; for region liveout symbases, not for other temps.

; HIR after limm-
; + DO i1 = 0, %conv.i + -1, 1   <DO_MULTI_EXIT_LOOP>
; |   %name.sroa.23.0438.out = -1 * i1 + %conv.i;
; |   %pgocount.i346439.out = 2 * i1 + %pgocount.i346435454;
; |   %limm = 2 * i1 + %pgocount.i346435454 + 2;
; |   if (%t108 != 95)
; |   {
; |      (@__profc__ZNK4llvm9StringRef5emptyEv)[0][0] = %limm;
; |      goto early.exit;
; |   }
; |   %t116 = 2 * i1 + %pgocount.i346435454  +  3;
; |   %limm = 2 * i1 + %pgocount.i346435454 + 3;
; + END LOOP
; (@__profc__ZNK4llvm9StringRef5emptyEv)[0][0] = %limm;


; TODO: Sink the definition of %limm before early-exit as well.

; CHECK: + DO i1 = 0, %conv.i + -1, 1   <DO_MULTI_EXIT_LOOP>
; CHECK: |   %limm = 2 * i1 + %pgocount.i346435454 + 2;
; CHECK: |   if (%t108 != 95)
; CHECK: |   {
; CHECK: |      (@__profc__ZNK4llvm9StringRef5emptyEv)[0][0] = %limm;
; CHECK: |      %name.sroa.23.0438.out = -1 * i1 + %conv.i;
; CHECK: |      %pgocount.i346439.out = 2 * i1 + %pgocount.i346435454;
; CHECK: |      goto early.exit;
; CHECK: |   }
; CHECK: + END LOOP
; CHECK:   %t116 = 2 * %conv.i + %pgocount.i346435454 + -2  +  3;
; CHECK:   %limm = 2 * %conv.i + %pgocount.i346435454 + 1;
; CHECK:   (@__profc__ZNK4llvm9StringRef5emptyEv)[0][0] = %limm;


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@__profc__ZNK4llvm9StringRef5emptyEv = external hidden global [1 x i64], section "__llvm_prf_cnts", align 8

define void @foo(i64 %pgocount.i346435454, i64 %conv.i, i8 %t108) {
entry:
  br label %loop

loop:                 ; preds = %latch, %entry
  %pgocount.i346439 = phi i64 [ %t106, %latch ], [ %pgocount.i346435454, %entry ]
  %name.sroa.23.0438 = phi i64 [ %sub.i, %latch ], [ %conv.i, %entry ]
  %t106 = add i64 %pgocount.i346439, 2
  store i64 %t106, ptr @__profc__ZNK4llvm9StringRef5emptyEv, align 8
  %cmp = icmp eq i8 %t108, 95
  br i1 %cmp, label %latch, label %early.exit

latch:               ; preds = %loop
  %sub.i = add nsw i64 %name.sroa.23.0438, -1
  %t116 = add i64 %pgocount.i346439, 3
  store i64 %t116, ptr @__profc__ZNK4llvm9StringRef5emptyEv, align 8
  %cmp.i348 = icmp eq i64 %sub.i, 0
  br i1 %cmp.i348, label %if.then59.loopexit, label %loop

if.then59.loopexit:                               ; preds = %latch
  %.lcssa = phi i64 [ %t116, %latch ]
  ret void

early.exit:
  %pgocount.i346439.lcssa = phi i64 [ %pgocount.i346439, %loop ]
  %name.sroa.23.0438.lcssa = phi i64 [ %name.sroa.23.0438, %loop ]
  ret void
}
