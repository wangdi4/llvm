; Test to check correctness of HIR vector CG for loops containing memrefs with loop variant
; base CEs.

; HIR incoming to vectorizer
; <0>     BEGIN REGION { }
; <14>          %entry.region = @llvm.directive.region.entry(); [ DIR.VPO.AUTO.VEC() ]
; <13>
; <13>          + DO i1 = 0, 1023, 1   <DO_LOOP>
; <3>           |   %0 = (%arr)[i1];
; <5>           |   %1 = (%0)[i1];
; <6>           |   %sum.011 = %1  +  %sum.011; <Safe Reduction>
; <13>          + END LOOP
; <13>
; <15>          @llvm.directive.region.exit(%entry.region); [ DIR.VPO.END.AUTO.VEC() ]
; <0>     END REGION

; RUN: opt -hir-ssa-deconstruction -hir-vec-dir-insert -VPlanDriverHIR -vplan-force-vf=4 -print-after=VPlanDriverHIR < %s 2>&1 | FileCheck %s

; CHECK:          + DO i1 = 0, 1023, 4   <DO_LOOP> <novectorize>
; CHECK-NEXT:     |   %.vec = (<4 x i32*>*)(%arr)[i1];
; CHECK-NEXT:     |   %.vec1 = (<4 x i32>*)(%.vec)[i1 + <i64 0, i64 1, i64 2, i64 3>];
; CHECK-NEXT:     |   %result.vector = %.vec1  +  %result.vector;
; CHECK-NEXT:     + END LOOP

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind readonly uwtable
define dso_local i32 @foo(i32** nocapture readonly %arr) local_unnamed_addr {
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %sum.011 = phi i32 [ 0, %entry ], [ %add, %for.body ]
  %arrayidx = getelementptr inbounds i32*, i32** %arr, i64 %indvars.iv
  %0 = load i32*, i32** %arrayidx, align 8
  %arrayidx2 = getelementptr inbounds i32, i32* %0, i64 %indvars.iv
  %1 = load i32, i32* %arrayidx2, align 4
  %add = add nsw i32 %1, %sum.011
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  %add.lcssa = phi i32 [ %add, %for.body ]
  ret i32 %add.lcssa
}
