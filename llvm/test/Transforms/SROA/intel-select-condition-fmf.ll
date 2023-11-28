; RUN: opt -passes=sroa -S %s | FileCheck %s

; This tests checks if fast math flag is correctly passed
; from select condition instruction to select.

; CHECK:       %cmp.i = fcmp fast olt float %0, %acc.addr.0
; CHECK-NEXT:  %.sroa.speculate.load.true = load float, ptr %arrayidx, align 4
; CHECK-NEXT:  %.sroa.speculated = select fast i1 %cmp.i, float %.sroa.speculate.load.true, float %acc.addr.0
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: mustprogress uwtable
define dso_local noundef nofpclass(nan inf) float @_Z11hdr_rgb_minPfjf(ptr noundef %r, i32 noundef %len, float noundef nofpclass(nan inf) %acc) {
entry:
  %acc.addr = alloca float, align 4
  store float %acc, ptr %acc.addr, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.body, %entry
  %i.0 = phi i32 [ 0, %entry ], [ %inc, %for.body ]
  %mul = shl i32 %len, 3
  %cmp = icmp ult i32 %i.0, %mul
  br i1 %cmp, label %for.body, label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond
  %0 = load float, ptr %acc.addr, align 4
  ret float %0

for.body:                                         ; preds = %for.cond
  %idxprom = zext i32 %i.0 to i64
  %arrayidx = getelementptr inbounds float, ptr %r, i64 %idxprom
  %1 = load float, ptr %arrayidx, align 4
  %2 = load float, ptr %acc.addr, align 4
  %cmp.i = fcmp fast olt float %1, %2
  %3 = select i1 %cmp.i, ptr %arrayidx, ptr %acc.addr
  %4 = load float, ptr %3, align 4
  store float %4, ptr %acc.addr, align 4
  %inc = add i32 %i.0, 1
  br label %for.cond
}
