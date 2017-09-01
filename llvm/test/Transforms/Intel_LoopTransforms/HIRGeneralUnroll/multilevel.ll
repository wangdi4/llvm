
; Test case for HIR General Unrolling for multi-level loops
; where unrolling happens only for innermost loop.

; RUN: opt -loop-simplify -hir-ssa-deconstruction -hir-general-unroll -print-after=hir-general-unroll -hir-cg -S < %s 2>&1 | FileCheck %s
; HIR Check
; CHECK: BEGIN REGION { modified }
; Check unrolling of innermost loop.
; CHECK: DO i2 = 0, 67, 1
; CHECK-NEXT: (@A)[0][i1 + 16 * i2 + -1];
; CHECK: END LOOP
; Remainder loop.
; CHECK: DO i2 = 544, 549, 1
; CHECK-NEXT: (@A)[0][i1 + 2 * i2 + -1]
; CHECK: END LOOP

; Codegen Check. 
; CHECK: entry

; terminator of entry bblock should point to new unrolled region.
; CHECK: for.cond1.preheader:
; CHECK: br i1 true, {{.*}}label %region

; check loop is unrolled.
; CHECK: region.0:
; CHECK: loop{{.*}}
; CHECK: mul i64 16
; CHECK: getelementptr
; CHECK: getelementptr
; CHECK: getelementptr
; CHECK: getelementptr
; CHECK: getelementptr
; CHECK: getelementptr
; CHECK: getelementptr
; CHECK: getelementptr
; CHECK: getelementptr
; CHECK: getelementptr
; CHECK: getelementptr
; CHECK: getelementptr
; CHECK: getelementptr
; CHECK: getelementptr
; CHECK: add i64 {{.*}}, 13
; CHECK: getelementptr
; CHECK: getelementptr
; Remainder loop
; CHECK: br i1 %condloop{{.*}}, label %loop{{.*}}, label %afterloop{{.*}}
; No modification in Remainder loop
; CHECK: mul i64 2
; CHECK: add i64 {{.*}}, -1

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = common global [550 x i32] zeroinitializer
@B = common global [550 x i32] zeroinitializer

; Function Attrs: nounwind uwtable
define void @foo(i32 %n, i32 %k) {
entry:
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.inc7, %entry
  %i.018 = phi i64 [ 0, %entry ], [ %inc8, %for.inc7 ]
  br label %for.body3

for.body3:                                        ; preds = %for.body3, %for.cond1.preheader
  %j.017 = phi i64 [ 0, %for.cond1.preheader ], [ %inc, %for.body3 ]
  %mul = shl nsw i64 %j.017, 1
  %add = add nuw nsw i64 %mul, %i.018
  %sub = add nsw i64 %add, -1
  %arrayidx = getelementptr inbounds [550 x i32], [550 x i32]* @A, i64 0, i64 %sub
  %0 = load i32, i32* %arrayidx, align 4 
  %arrayidx6 = getelementptr inbounds [550 x i32], [550 x i32]* @A, i64 0, i64 %add
  store i32 %0, i32* %arrayidx6, align 4
  %inc = add nuw nsw i64 %j.017, 1
  %exitcond = icmp eq i64 %inc, 550
  br i1 %exitcond, label %for.inc7, label %for.body3

for.inc7:                                         ; preds = %for.body3
  %inc8 = add nuw nsw i64 %i.018, 1
  %exitcond19 = icmp eq i64 %inc8, 550
  br i1 %exitcond19, label %for.end9, label %for.cond1.preheader

for.end9:                                         ; preds = %for.inc7
  ret void
}

; Function Attrs: nounwind
declare void @llvm.lifetime.start(i64, i8* nocapture) 

; Function Attrs: nounwind
declare void @llvm.lifetime.end(i64, i8* nocapture) 


