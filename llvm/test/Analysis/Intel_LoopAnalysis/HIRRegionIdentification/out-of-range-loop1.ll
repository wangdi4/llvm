; RUN: opt < %s -analyze -scalar-evolution -hir-region-identification | FileCheck %s

; Verify that we bail out on loops whose backedge taken count is -1. We cannot handle such loops in HIR as the trip count is out of the range of IV.

; CHECK: Loop %for.cond15.preheader: backedge-taken count is -1

; CHECK-NOT: Region 1

; ModuleID = 'bugpoint-reduced-simplified.bc'
source_filename = "bugpoint-output-bad1273.bc"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define void @main() local_unnamed_addr {
entry:
  %0 = and i64 84, 4294967295
  br label %for.cond15.preheader

for.cond15.preheader:                             ; preds = %for.cond15.preheader, %entry
  %indvars.iv333 = phi i64 [ %0, %entry ], [ %indvars.iv.next334, %for.cond15.preheader ]
  %indvars.iv.next334 = add nuw nsw i64 %indvars.iv333, 1
  store i32 undef, i32* undef, align 8
  %exitcond337 = icmp eq i64 %indvars.iv.next334, 84
  br i1 %exitcond337, label %for.cond12.for.end32_crit_edge, label %for.cond15.preheader

for.cond12.for.end32_crit_edge:                   ; preds = %for.cond15.preheader
  ret void
}

