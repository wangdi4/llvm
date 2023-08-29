; RUN: opt < %s -xmain-opt-level=3 -hir-details-dims -passes="hir-ssa-deconstruction,print<hir-framework>,hir-temp-cleanup,print<hir-framework>" 2>&1 | FileCheck %s

; Verify that we can substitute pointer load into self-address-of ref even though its stride is a temp.

; CHECK: Function: foo

; CHECK: + DO i1 = 0, %n + -1, 1   <DO_LOOP>
; CHECK: |   %ptr.load = (%ptr)[0:0:8(ptr:0)];
; CHECK: |   @bar(&((%ptr.load)[0:0:%stride(i64:0)]));
; CHECK: + END LOOP

; CHECK: Function: foo

; CHECK: + DO i1 = 0, %n + -1, 1   <DO_LOOP>
; CHECK: |   @bar((%ptr)[0:0:8(ptr:0)]);
; CHECK: + END LOOP

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define void @foo(ptr %ptr, i64 %stride, i64 %n) {
alloca_0:
  %rel = icmp slt i64 %n, 1
  br i1 %rel, label %bb1, label %loop.preheader

loop.preheader:
  br label %loop

loop:                                             ; preds = %loop, %loop
  %indvars.iv = phi i64 [ 1, %loop.preheader ], [ %indvars.iv.next, %loop ]
  %ptr.load = load ptr, ptr %ptr, align 1
  %subs = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 0, i64 %stride, ptr elementtype(i64) %ptr.load, i64 0)
  call void @bar(ptr %subs)
  %indvars.iv.next = add nsw i64 %indvars.iv, 1
  %rel52 = icmp sgt i64 %indvars.iv.next, %n
  br i1 %rel52, label %bb1.loopexit, label %loop

bb1.loopexit:                                     ; preds = %loop
  br label %bb1

bb1:                                              ; preds = %bb1.loopexit, %alloca_0
  ret void
}

declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #1

declare void @bar(ptr) #1

attributes #1 = { willreturn nounwind readnone }
