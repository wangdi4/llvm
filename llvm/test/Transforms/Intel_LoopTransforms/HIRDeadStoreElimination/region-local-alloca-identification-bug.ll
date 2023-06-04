; RUN: opt -passes="hir-ssa-deconstruction,hir-dead-store-elimination,print<hir>" -disable-output 2>&1 < %s | FileCheck %s
;
; Verify that we recognize %a as a region local alloca for the first region
; only. We shouldn't optimize away stores of %a in the second region based
; on this analysis. Before this fix, we eliminated stores to %a in second
; region. Note that the two regions exist inside an outer loop so the
; stored values can reach the loads.


; CHECK: BEGIN REGION { }
; CHECK:    + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK:    |   %ald = (%a)[0][i1];
; CHECK:    |   (%p)[i1] = %ald;
; CHECK:    + END LOOP
; CHECK: END REGION

; CHECK: BEGIN REGION { }
; CHECK:    + DO i1 = 0, 89, 1   <DO_LOOP>
; CHECK:    |   (%a)[0][i1] = 77;
; CHECK:    + END LOOP
; CHECK: END REGION


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local i32 @main(ptr %p) {
entry:
  %a = alloca [100 x i32], align 16
  call void @llvm.memset.p0.i64(ptr noundef nonnull align 16 dereferenceable(400) %a, i8 0, i64 400, i1 false)
  %arrayidx7 = getelementptr inbounds [100 x i32], ptr %a, i64 0, i64 0
  br label %outer.loop

outer.loop:                                         ; preds = %entry, %outer.latch
  %indvars.iv36 = phi i32 [ 16, %entry ], [ %indvars.iv.next37, %outer.latch ]
  br label %inner.loop1

inner.loop1:                                        ; preds = %outer.loop, %inner.loop1
  %indvars.iv = phi i64 [ %indvars.iv.next, %inner.loop1 ], [ 0, %outer.loop ]
  %arrayidx6 = getelementptr inbounds [100 x i32], ptr %a, i64 0, i64 %indvars.iv
  %ald = load i32, ptr %arrayidx6, align 4
  %arrayidx1 = getelementptr inbounds i32, ptr %p, i64 %indvars.iv
  store i32 %ald, ptr %arrayidx1, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond.not, label %inner.loop2.pre, label %inner.loop1

inner.loop2.pre:
  br label %inner.loop2

inner.loop2:                                       ; preds = %inner.loop2.pre, %inner.loop2
  %indvars.iv34 = phi i64 [ 0, %inner.loop2.pre ], [ %indvars.iv.next35, %inner.loop2 ]
  %arrayidx14 = getelementptr inbounds [100 x i32], ptr %a, i64 0, i64 %indvars.iv34
  store i32 77, ptr %arrayidx14, align 4
  %indvars.iv.next35 = add nuw nsw i64 %indvars.iv34, 1
  %cmp11 = icmp eq i64 %indvars.iv.next35, 90
  br i1 %cmp11, label %outer.latch, label %inner.loop2

outer.latch:                                        ; preds = %inner.loop2
  %indvars.iv.next37 = add nsw i32 %indvars.iv36, -1
  %pld = load i32, ptr %p
  %cmp = icmp ugt i32 %indvars.iv.next37, %pld
  br i1 %cmp, label %outer.loop, label %end

end:                                        ; preds = %outer.latch
  ret i32 0
}

declare void @llvm.memset.p0.i64(ptr nocapture writeonly, i8, i64, i1 immarg)

