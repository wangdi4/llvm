
; Test for General unrolling with single level loop
; with statement , a[i] = a[n*i].

; RUN: opt -passes="loop-simplify,hir-ssa-deconstruction,hir-general-unroll,print<hir>,hir-cg" -S < %s 2>&1 | FileCheck %s
;
; HIR Check
; CHECK: BEGIN REGION { modified }
; CHECK: DO i1 = 0, 34, 1
; CHECK: (@a)[0][8 * {{.*}}(%n) * i1 + 7 * {{.*}}(%n)]
; CHECK: END LOOP
; CHECK: DO i1 = 280, 283, 1
; CHECK: (@a)[0][i1]
; CHECK-NEXT: END LOOP

; Codegen check.
; CHECK: entry

; terminator of entry bblock should point to new unrolled region.
; CHECK: for.body:
; CHECK: br i1 true, {{.*}}label %region

; check loop is unrolled.
; CHECK: region.0:
; CHECK: loop{{.*}}

; Unroll body
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
; CHECK: getelementptr
; CHECK: getelementptr

; UB of Unrolled Loop
; CHECK: icmp ne i64 {{.*}}, 34

; Check that unrolled loop has disable pragma
; CHECK: !llvm.loop

; Remainder loop
; CHECK: afterloop{{.*}}
; CHECK: store i64 280
; CHECK: loop{{.*}}
; CHECK: getelementptr
; CHECK: getelementptr
; CHECK-NOT: getelementptr

; Remainder of constant trip count loops should not have disabling metadata.
; CHECK-NOT: !llvm.loop

; CHECK: llvm.loop.unroll.disable

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
  %arrayidx = getelementptr inbounds [284 x i32], ptr @a, i64 0, i64 %1
  %2 = load i32, ptr %arrayidx, align 4
  %arrayidx2 = getelementptr inbounds [284 x i32], ptr @a, i64 0, i64 %indvars.iv
  store i32 %2, ptr %arrayidx2, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 284
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  ret i32 0
}

; Function Attrs: nounwind
declare void @llvm.lifetime.start(i64, ptr nocapture)

; Function Attrs: nounwind
declare void @llvm.lifetime.end(i64, ptr nocapture)


