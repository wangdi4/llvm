; RUN:  opt -O1 -vplan-driver -vpo-vplan-build-stress-test -vplan-print-after-loop-massaging %s 2>&1 | FileCheck %s 

; Test case reduced from intergrid_map.cc in the spec2006test447Cpp/447 benchmark
; This code runs into an issue in LoopExitCanonicalization under VPlan stress
; testing, where a verifier check fails due to a phi instruction not having
; values for all predecessors after some of the phis were moved into their
; current block.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo(ptr %p, i1 %cond1, i1 %cond2) {
; CHECK: new.loop.latch{{[0-9]+}}: # preds: [[BB1:.*]], [[IMBB:.*]], [[BB2:.*]]
; CHECK-NEXT: i32 [[VPPHI1:%.*]] = phi  [ i32 0, [[BB1]] ],  [ i32 1, [[IMBB]] ],  [ i32 2, [[BB2]] ]
; CHECK-NEXT: i32 [[MOVEDPHI:%.*]] = phi  [ i32 0, [[BB2]] ],  [ i32 1, [[BB1]] ],  [ i32 undef, [[IMBB]] ]
; CHECK-NEXT: i1 [[VPPHI2:%.*]] = phi  [ i1 {{%.*}}, [[BB1]] ],  [ i1 false, [[IMBB]] ],  [ i1 false, [[BB2]] ]
entry:
  %a = alloca [1 x i32], i32 0, align 8
  br label %for.body

for.body:                                         ; preds = %foo.exit, %entry
  %indvars.iv4 = phi i64 [ 0, %entry ], [ %indvars.iv.next, %foo.exit ]
  call void @bar(ptr %a)
  store volatile i32 0, ptr null, align 8
  br label %while.cond

while.cond:                                     ; preds = %while.body, %for.body
  %0 = phi i32 [ 1, %while.body ], [ 0, %for.body ]
  store i32 0, ptr %p, align 4
  store i32 %0, ptr %a, align 8
  br i1 %cond1, label %while.cond.i.preheader, label %while.cond.true

while.cond.i.preheader:                       ; preds = %while.cond
  br i1 %cond2, label %while.body, label %foo.exit

while.cond.true:                      ; preds = %while.cond
  store i32 0, ptr %p, align 4
  br label %foo.exit

while.body:                                     ; preds = %while.cond.i.preheader
  %1 = load i64, ptr %p, align 8
  %anded = and i64 %1, 1
  %cond3 = icmp eq i64 %anded, 0
  br i1 %cond3, label %while.cond, label %foo.exit

foo.exit: ; preds = %while.body, %while.cond.true, %while.cond.i.preheader
  %indvars.iv.next = add i64 %indvars.iv4, 1
  %exitcond.not = icmp eq i64 %indvars.iv4, 4
  br i1 %exitcond.not, label %for.cond.cleanup, label %for.body

for.cond.cleanup:                                 ; preds = %foo.exit
  ret void
}

declare void @bar(ptr)
