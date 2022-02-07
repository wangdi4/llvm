; RUN: opt -hir-ssa-deconstruction -hir-loop-blocking -print-after=hir-loop-blocking -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-loop-blocking" -print-after=hir-loop-blocking -disable-output -aa-pipeline="basic-aa" 2>&1 < %s | FileCheck %s

; Verify that the test compiles successfully. Previously, blocking was calling
; delinearization for this loopnest which failed to handle memref with null base
; ptr.

; CHECK-NOT: modified

; CHECK: + DO i1 = 0, 1, 1   <DO_LOOP>
; CHECK: |   + DO i2 = 0, 1, 1   <DO_LOOP>
; CHECK: |   |   (null)[%int_sext5848 * i1 + i2 + (undef * %int_sext5848)] = 1.000000e+00;
; CHECK: |   + END LOOP
; CHECK: + END LOOP

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo(i32* noalias nocapture readonly dereferenceable(4) %SF_SURFACE_PHYSICS, float* noalias nocapture writeonly dereferenceable_or_null(4) %SR, i64 %int_sext5848) {
alloca_1:
  store i32 1, i32* %SF_SURFACE_PHYSICS, align 1
  %rel.2259.not = icmp eq float* %SR, null
  br i1 %rel.2259.not, label %loop_test1209.preheader.preheader, label %common.ret

loop_test1209.preheader.preheader:                ; preds = %alloca_1
  br label %loop_test1209.preheader

loop_test1209.preheader:                          ; preds = %loop_exit1211, %loop_test1209.preheader.preheader
  %rel.2265 = phi i1 [ false, %loop_exit1211 ], [ true, %loop_test1209.preheader.preheader ]
  %"var$337.03" = phi i64 [ %add.586, %loop_exit1211 ], [ undef, %loop_test1209.preheader.preheader ]
  %t1 = mul nsw i64 %"var$337.03", %int_sext5848
  %t2 = getelementptr inbounds float, float* null, i64 %t1
  br label %loop_body1210

loop_body1210:                                    ; preds = %loop_test1209.preheader, %loop_body1210
  %rel.2264 = phi i1 [ true, %loop_test1209.preheader ], [ false, %loop_body1210 ]
  %"$loop_ctr5846.02" = phi i64 [ 0, %loop_test1209.preheader ], [ 1, %loop_body1210 ]
  %t0 = getelementptr inbounds float, float* %t2, i64 %"$loop_ctr5846.02"
  store float 1.000000e+00, float* %t0, align 4
  br i1 %rel.2264, label %loop_body1210, label %loop_exit1211

loop_exit1211:                                    ; preds = %loop_body1210
  %add.586 = add nsw i64 %"var$337.03", 1
  br i1 %rel.2265, label %loop_test1209.preheader, label %common.ret.loopexit

common.ret.loopexit:                              ; preds = %loop_exit1211
  br label %common.ret

common.ret:                                       ; preds = %common.ret.loopexit, %alloca_1
  ret void
}

