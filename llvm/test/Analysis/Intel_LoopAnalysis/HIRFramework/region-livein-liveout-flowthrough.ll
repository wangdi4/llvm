; RUN: opt -passes="hir-ssa-deconstruction,print<hir-framework>" -disable-output < %s 2>&1 | FileCheck %s

; HIR-
; + DO i1 = 0, -1 * %.pr.us.us + smax(0, (1 + %.pr.us.us)) + -1, 1   <DO_MULTI_EXIT_LOOP>  <MAX_TC_EST = 2147483649>
; |   %storemerge15.us.us53.out = %storemerge15.us.us53;
; |   (@i)[0] = i1 + %.pr.us.us + 1;
; |   if (i1 + %.pr.us.us >= 0)
; |   {
; |      goto for.end12.loopexit;
; |   }
; |   %storemerge15.us.us53 = 0;
; + END LOOP

; Verify that %in which feeds the flowthrough liveout phi %out is marked as livein to the region.

; CHECK: LiveIns:
; CHECK-SAME: %in

; CHECK: LiveOuts:
; CHECK-SAME: %out

; RUN: opt -passes="hir-ssa-deconstruction,hir-cg" -force-hir-cg -S < %s 2>&1 | FileCheck %s --check-prefix=CG-LIVEIN

; Verify that we generate a store for livein instruciton %in even though it has no use inside the region.

; CG-LIVEIN: region
; CG-LIVEIN: store i32 %in

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@x = common dso_local local_unnamed_addr global i32 0, align 4
@i = common dso_local local_unnamed_addr global i32 0, align 4
@k = common dso_local local_unnamed_addr global i32 0, align 4

define void @foo(i32 %inc.us.us, i32 %.pr.us.us) {
entry:
  %in = load i32, ptr @x, align 4
  br label %for.cond.us.us

for.cond.us.us:                                   ; preds = %entry, %for.cond1.us.us
  %storemerge15.us.us53 = phi i32 [ %inc.us.us, %entry ], [ %storemerge15.us.us, %for.cond1.us.us ]
  %0 = phi i32 [ %.pr.us.us, %entry ], [ %inc11.us.us, %for.cond1.us.us ]
  %inc11.us.us = add nsw i32 %0, 1
  store i32 %inc11.us.us, ptr @i, align 4
  %cmp.us.us = icmp slt i32 %0, 0
  br i1 %cmp.us.us, label %for.cond1.us.us, label %for.end12.loopexit

for.cond1.us.us:                                  ; preds = %for.cond.us.us
  %out = phi i32 [ %in, %for.cond.us.us ]
  %storemerge15.us.us = phi i32 [ 0, %for.cond.us.us ]
  %cmp2.us.us = icmp sgt i32 %storemerge15.us.us, %inc11.us.us
  br i1 %cmp2.us.us, label %for.cond.us.us, label %for.cond1.us.us.for.body3.us.us_crit_edge

for.cond1.us.us.for.body3.us.us_crit_edge:        ; preds = %for.cond1.us.us
  %split = phi i32 [ %out, %for.cond1.us.us ]
  ret void

for.end12.loopexit37:                             ; preds = %for.body3.thread
  store i32 1, ptr @i, align 4
  br label %for.end12

for.end12.loopexit:                               ; preds = %for.cond.us.us
  %storemerge15.us.us.lcssa48 = phi i32 [ %storemerge15.us.us53, %for.cond.us.us ]
  store i32 %storemerge15.us.us.lcssa48, ptr @k, align 4
  br label %for.end12

for.end12:                                        ; preds = %for.end12.loopexit, %for.end12.loopexit37
  ret void
}
