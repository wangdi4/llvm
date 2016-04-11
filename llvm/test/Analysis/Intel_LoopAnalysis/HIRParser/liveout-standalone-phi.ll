; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -hir-parser | FileCheck %s

; Check parsing output for the loop verifying that we create a liveout copy for standalone phi %k.0 which is live outside the loop.
; CHECK: DO i1 = 0, 44
; CHECK-NEXT: %k.0 = 0
; CHECK-NEXT: DO i2 = 0, -1 * i1 + 46
; CHECK-NEXT: %k.0.out = %k.0
; CHECK-NEXT: %k.0 = -1 * i2 + 46
; CHECK-NEXT: END LOOP
; CHECK-NEXT: END LOOP


;Module Before HIR; ModuleID = 'live-out-stand-alone-phi.cpp'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = global [10 x i32] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define void @_Z3foov() {
entry:
  br label %for.cond.1.preheader

for.cond.1.preheader:                             ; preds = %for.end, %entry
  %i.013 = phi i32 [ 0, %entry ], [ %inc, %for.end ]
  br label %for.cond.1

for.cond.1:                                       ; preds = %for.cond.1, %for.cond.1.preheader
  %k.0 = phi i32 [ %j.0, %for.cond.1 ], [ 0, %for.cond.1.preheader ]
  %j.0 = phi i32 [ %dec, %for.cond.1 ], [ 46, %for.cond.1.preheader ]
  %cmp2 = icmp ugt i32 %j.0, %i.013
  %dec = add nsw i32 %j.0, -1
  br i1 %cmp2, label %for.cond.1, label %for.end

for.end:                                          ; preds = %for.cond.1
  %k.0.lcssa = phi i32 [ %k.0, %for.cond.1 ]
  %inc = add nuw nsw i32 %i.013, 1
  %exitcond = icmp eq i32 %inc, 45
  br i1 %exitcond, label %for.end.5, label %for.cond.1.preheader

for.end.5:                                        ; preds = %for.end
  %k.0.lcssa.lcssa = phi i32 [ %k.0.lcssa, %for.end ]
  store i32 %k.0.lcssa.lcssa, i32* getelementptr inbounds ([10 x i32], [10 x i32]* @A, i64 0, i64 0), align 16
  ret void
}

