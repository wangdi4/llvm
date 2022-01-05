; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -enable-new-pm=0 -hir-framework -hir-framework-debug=parser | FileCheck %s
; RUN: opt %s -passes="hir-ssa-deconstruction,print<hir-framework>" -hir-framework-debug=parser -disable-output  2>&1 | FileCheck %s


; Check parsing output for the loop verifying that we do not create the SCC (%e.sroa.3.042 -> %e.sroa.3.1 -> %e.sroa.3.1.lcssa) because of loop liveout phi's (%e.sroa.3.1) dependence on the same bblock phi %k.0.

; CHECK: + DO i1 = 0, 47, 1   <DO_LOOP>
; CHECK: |   %e.sroa.3.1 = %e.sroa.3.042;
; CHECK: |
; CHECK: |   + DO i2 = 0, i1, 1   <DO_LOOP>
; CHECK: |   |   %e.sroa.3.1.out = %e.sroa.3.1;
; CHECK: |   |   %e.sroa.3.1 = i1 + -1 * i2 + 1;
; CHECK: |   + END LOOP
; CHECK: |   %e.sroa.3.042 = %e.sroa.3.1.out;
; CHECK: + END LOOP


;Module Before HIR; ModuleID = 'cq213070.cpp'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define i32 @main() {
entry:
  br label %for.cond.1.preheader

for.cond.1.preheader:                             ; preds = %for.end, %entry
  %i.043 = phi i32 [ 1, %entry ], [ %inc, %for.end ]
  %e.sroa.3.042 = phi i32 [ 0, %entry ], [ %e.sroa.3.1.lcssa, %for.end ]
  br label %for.cond.1

for.cond.1:                                       ; preds = %for.cond.1, %for.cond.1.preheader
  %e.sroa.3.1 = phi i32 [ %k.0, %for.cond.1 ], [ %e.sroa.3.042, %for.cond.1.preheader ]
  %k.0 = phi i32 [ %dec, %for.cond.1 ], [ %i.043, %for.cond.1.preheader ]
  %cmp2 = icmp ugt i32 %k.0, 1
  %dec = add nsw i32 %k.0, -1
  br i1 %cmp2, label %for.cond.1, label %for.end

for.end:                                          ; preds = %for.cond.1
  %e.sroa.3.1.lcssa = phi i32 [ %e.sroa.3.1, %for.cond.1 ]
  %inc = add nuw nsw i32 %i.043, 1
  %exitcond = icmp eq i32 %inc, 49
  br i1 %exitcond, label %for.cond.7.preheader, label %for.cond.1.preheader

for.cond.7.preheader:                             ; preds = %for.end
  %e.sroa.3.1.lcssa.lcssa = phi i32 [ %e.sroa.3.1.lcssa, %for.end ]
  br label %exit

exit:                                           ; preds = %for.cond.7.preheader
  ret i32 %e.sroa.3.1.lcssa.lcssa
}

