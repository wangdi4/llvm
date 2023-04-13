; CMPLRLLVM-29768: addrspace(256) is reserved by the x86 backend for user
; defined segments, and memcpy is not supported.

; RUN: opt -passes="loop(loop-idiom)" -S %s | FileCheck %s
; CHECK: @f1
; CHECK-NOT: memcpy

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @f1(i64 %n) #0 {
entry:
  %s.addr = alloca i8 addrspace(256)*, align 8
  %n.addr = alloca i64, align 8
  %i = alloca i64, align 8
  store i64 %n, i64* %n.addr, align 8
  store i64 0, i64* %i, align 8
  br label %for.cond

for.cond:                                         ; preds = %for.body, %entry
  %0 = load i64, i64* %i, align 8
  %1 = load i64, i64* %n.addr, align 8
  %cmp = icmp ult i64 %0, %1
  br i1 %cmp, label %for.body, label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond
  ret void

for.body:                                         ; preds = %for.cond
  %2 = load i64, i64* %i, align 8
  %arrayidx = getelementptr inbounds i8, i8 addrspace(256)* null, i64 %2
  store i8 0, i8 addrspace(256)* %arrayidx, align 1
  %3 = load i64, i64* %i, align 8
  %inc = add i64 %3, 1
  store i64 %inc, i64* %i, align 8
  br label %for.cond, !llvm.loop !1
}

attributes #0 = { "unsafe-fp-math"="true" }

!llvm.ident = !{!0}

!0 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2021.4.0 (2021.x.0.YYYYMMDD)"}
!1 = distinct !{!1, !2}
!2 = !{!"llvm.loop.mustprogress"}
