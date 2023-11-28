; REQUIRED: asserts
; RUN: opt %s -passes="hir-ssa-deconstruction,print<hir-region-identification>" -disable-output -debug-only=hir-region-identification  2>&1 | FileCheck %s

; Test checks that we do not create a region for GEP with
; global base pointer and negative first index.

; CHECK: Bailing out on instruction:
; CHECK:  %gep = getelementptr i32, ptr getelementptr ([624 x i32], ptr @c, i32 -1, i32 623), i32 %i.07
; CHECK: LOOPOPT_OPTREPORT: Loop %for.body: Constant array access goes out of range.


target datalayout = "e-m:e-p:32:32-p270:32:32-p271:32:32-p272:64:64-i128:128-f64:32:64-f80:32-n8:16:32-S128"
target triple = "i386-unknown-linux-gnu"

@c = dso_local local_unnamed_addr global [624 x i32] zeroinitializer, align 4

; Function Attrs: nofree noinline norecurse nosync nounwind memory(readwrite, argmem: none, inaccessiblemem: none) uwtable
define dso_local void @bar() local_unnamed_addr {
entry:
  br label %for.body

for.body:                                         ; preds = %entry, %for.body
  %i.07 = phi i32 [ 1, %entry ], [ %inc, %for.body ]
  %gep = getelementptr i32, ptr getelementptr ([624 x i32], ptr @c, i32 -1, i32 623), i32 %i.07
  %0 = load i32, ptr %gep, align 4
  %mul1 = shl i32 %0, 1
  %add.ptr3 = getelementptr inbounds i32, ptr @c, i32 %i.07
  store i32 %mul1, ptr %add.ptr3, align 4
  %inc = add nuw nsw i32 %i.07, 1
  %exitcond.not = icmp eq i32 %inc, 624
  br i1 %exitcond.not, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  ret void
}

