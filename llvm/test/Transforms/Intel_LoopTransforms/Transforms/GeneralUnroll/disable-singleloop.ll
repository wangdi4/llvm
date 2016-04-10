; Sanity Tests for Disable General unrolling with single level loop
; with statement , a[i] = a[n*i].
;
; ===-----------------------------------===
; *** Run0: Normal General Unrolling ***
; ===-----------------------------------===
; RUN: opt -hir-ssa-deconstruction -hir-general-unroll -print-after=hir-general-unroll \
; RUN: -S < %s 2>&1 | FileCheck %s -check-prefix=UNROLL
;
;  
; ===-----------------------------------===
; *** Run1: Disabled Normal General Unrolling ***
; ===-----------------------------------===
; RUN: opt -hir-ssa-deconstruction -hir-general-unroll -disable-hir-general-unroll \
; RUN: -print-after=hir-general-unroll -S < %s 2>&1 | FileCheck %s -check-prefix=NOUNROLL
; 
; 
; ===----------------------------------------------===
; --- HIR Tests for Run0: Normal General Unrolling ---
; ===----------------------------------------------===
; 
; UNROLL: BEGIN REGION { modified }
; UNROLL: DO i1 = 0, 34, 1
; UNROLL: {al:4}(@a)[0][8 * {{.*}}(%n) * i1 + 7 * {{.*}}(%n)]
; UNROLL: END LOOP
; UNROLL-NEXT: DO i1 = 280, 283, 1
; UNROLL: {al:4}(@a)[0][i1]
; UNROLL-NEXT: END LOOP
;
;
; ===----------------------------------------------===
; === HIR Tests for Run1: Disabled General Unrolling ===
; ===----------------------------------------------===
;
; NOUNROLL: BEGIN REGION { }
; NOUNROLL: DO i1 = 0, 283, 1   <DO_LOOP>
; NOUNROLL: %2 = {al:4}(@a)[0][sext.i32.i64(%n) * i1];
; NOUNROLL:  {al:4}(@a)[0][i1] = %2;
; NOUNROLL: END LOOP
; NOUNROLL: END REGION
;
;
;Following is the testcase!
;
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@a = common global [284 x i32] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define i32 @subx(i32 %n) {
entry:
  %0 = sext i32 %n to i64
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %1 = mul nsw i64 %indvars.iv, %0
  %arrayidx = getelementptr inbounds [284 x i32], [284 x i32]* @a, i64 0, i64 %1
  %2 = load i32, i32* %arrayidx, align 4
  %arrayidx2 = getelementptr inbounds [284 x i32], [284 x i32]* @a, i64 0, i64 %indvars.iv
  store i32 %2, i32* %arrayidx2, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 284
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  ret i32 0
}

; Function Attrs: nounwind
declare void @llvm.lifetime.start(i64, i8* nocapture)

; Function Attrs: nounwind
declare void @llvm.lifetime.end(i64, i8* nocapture) 


