; RUN: opt -passes="hir-ssa-deconstruction,print<hir>,hir-cg" -force-hir-cg -S %s 2>&1 | FileCheck %s

; Verify that we can generate code correctly for the array with huge size.
; We were using 'unsigned' type to get its size. The size overflows 32 bits and
; was truncated to zero resulting in an assertion.

; CHECK: + DO i1 = 0, 4294967295, 1   <DO_LOOP>
; CHECK: |   %conv = sitofp.i64.float(i1);
; CHECK: |   (@d)[0][i1].0 = %conv;
; CHECK: |   %conv2 = sitofp.i64.float(2 * i1);
; CHECK: |   (@d)[0][i1].1 = %conv2;
; CHECK: |   %conv7 = sitofp.i64.float(3 * i1);
; CHECK: |   (@d)[0][i1].2 = %conv7;
; CHECK: |   %conv12 = sitofp.i64.float(4 * i1);
; CHECK: |   (@d)[0][i1].3 = %conv12;
; CHECK: + END LOOP

; CHECK: loop.{{.*}}:

; CHECK: = getelementptr inbounds [4294967296 x %struct.float4_str], ptr @d, i64 0, i64 %7, i32 1


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.float4_str = type { float, float, float, float }

@d = external dso_local local_unnamed_addr global [4294967296 x %struct.float4_str], align 16

define dso_local noundef i32 @_Z4initv() local_unnamed_addr #0 {
entry:
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  ret i32 undef

for.body:                                         ; preds = %for.body, %entry
  %i.025 = phi i64 [ 0, %entry ], [ %inc, %for.body ]
  %conv = sitofp i64 %i.025 to float
  %x = getelementptr inbounds [4294967296 x %struct.float4_str], ptr @d, i64 0, i64 %i.025, i32 0
  store float %conv, ptr %x, align 16
  %mul = shl nuw nsw i64 %i.025, 1
  %conv2 = sitofp i64 %mul to float
  %y = getelementptr inbounds [4294967296 x %struct.float4_str], ptr @d, i64 0, i64 %i.025, i32 1
  store float %conv2, ptr %y, align 4
  %mul6 = mul nuw nsw i64 %i.025, 3
  %conv7 = sitofp i64 %mul6 to float
  %z = getelementptr inbounds [4294967296 x %struct.float4_str], ptr @d, i64 0, i64 %i.025, i32 2
  store float %conv7, ptr %z, align 8
  %mul11 = shl nuw nsw i64 %i.025, 2
  %conv12 = sitofp i64 %mul11 to float
  %w = getelementptr inbounds [4294967296 x %struct.float4_str], ptr @d, i64 0, i64 %i.025, i32 3
  store float %conv12, ptr %w, align 4
  %inc = add nuw nsw i64 %i.025, 1
  %exitcond.not = icmp eq i64 %inc, 4294967296
  br i1 %exitcond.not, label %for.cond.cleanup, label %for.body
}

