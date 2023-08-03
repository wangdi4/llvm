
; RUN: opt -passes=instcombine %s -S 2>&1 | FileCheck %s --check-prefixes=CHECK,NOPRESERVE
; RUN: opt -passes=instcombine -instcombine-preserve-addr-compute=true %s -S 2>&1 | FileCheck %s --check-prefixes=CHECK,PRESERVE

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"

define void @test1(ptr %lower.bnd, ptr %upper.bnd, ptr %addr) {
; CHECK-LABEL: @test1
; PRESERVE:       sext i32
; NOPRESERVE-NOT: sext i32
entry:
  %lb = load i64, ptr %lower.bnd, align 8, !range !0
  %ub = load i64, ptr %upper.bnd, align 8, !range !0
  br label %loop

loop:
  %iv = phi i64 [ %lb, %entry ], [ %iv.next, %inc ]
  %val = trunc i64 %iv to i32
  %mul = mul nuw nsw i32 %val, 20
  %add = add nuw nsw i32 %mul, 19
  %idx = sext i32 %add to i64
  %ptr = getelementptr inbounds double, ptr %addr, i64 %idx
  %int = load i32, ptr %ptr, align 4
  %or = or i32 %int, 2
  store i32 %or, ptr %ptr, align 4
  br label %inc

inc:
  %iv.next = add nsw i64 %iv, 1
  %cmp = icmp ne i64 %iv.next, %ub
  br i1 %cmp, label %loop, label %exit

exit:
  ret void
}

!0 = !{i64 0, i64 265}
