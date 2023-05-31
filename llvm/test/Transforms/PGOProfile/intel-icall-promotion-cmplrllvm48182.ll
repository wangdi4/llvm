; RUN: opt < %s -passes=pgo-icall-prom -icp-total-percent-threshold=5 -disable-output -debug-only=pgo-icall-prom,pgo-icall-prom-analysis 2>&1 | FileCheck %s

; Check that a final candidate for indirect call promotion that meets the
; remaining count threshold will be converted under the relaxed condition
; for a total count percentage due to it being a final candidate.
; [CMPLRLLVM-48182]

; CHECK: Work on callsite   %call = call i32 %tmp()
; CHECK: Candidate 0 Count=1636212 Total Count = 1678800 Remaining Count = 1678800  RemainPercent: 97.5  TotalPercent: 97.5
; CHECK: Candidate 1 Count=42588 Total Count = 1678800 Remaining Count = 42588  RemainPercent: 100.0  TotalPercent: 2.5
 
; CHECK:  Promoted call with count: 1636212 out of total: 1678800
; CHECK:    call i32 @func2()
; CHECK:  Promoted call with count: 42588 out of total: 42588
; CHECK:    call i32 @func1()

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@foo = common global ptr null, align 8

define i32 @func1() {
entry:
  ret i32 0
}

define i32 @func2() {
entry:
  ret i32 1
}

define i32 @bar() {
entry:
  %tmp = load ptr, ptr @foo, align 8
  %call = call i32 %tmp(), !prof !1
  ret i32 %call
}

!1 = !{!"VP", i32 0, i64 1678800, i64 -4377547752858689819, i64 1636212, i64 -2545542355363006406, i64 42588}
