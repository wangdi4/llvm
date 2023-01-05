; Check that HIR framework and region identification can handle WRegions where loops
; are optimized away. Recognition algorithm should not crossover to sibling regions
; which would lead to stability issues. For example, based on the IR below, we should
; not map the entry directive (%0) inside first loop as SIMD region entry node for
; second loop.

; RUN: opt < %s -passes="hir-ssa-deconstruction,print<hir>" 2>&1 | FileCheck %s

; CHECK: BEGIN REGION { }
; CHECK:       + DO i1 = 0, 3, 1   <DO_LOOP>
; CHECK:       |   %0 = @llvm.directive.region.entry(); [ DIR.OMP.SIMD(),  QUAL.OMP.SIMDLEN(8),  QUAL.OMP.NORMALIZED.IV(null),  QUAL.OMP.NORMALIZED.UB(null),  QUAL.OMP.LINEAR:IV(&((%i.i.i.i.linear.iv)[0])1) ]
; CHECK:       |   @llvm.directive.region.exit(%0); [ DIR.OMP.END.SIMD() ]
; CHECK:       + END LOOP
; CHECK: END REGION

; CHECK: BEGIN REGION { }
; CHECK:       + DO i1 = 0, 3, 1   <DO_LOOP>
; CHECK:       |   %1 = @llvm.directive.region.entry(); [ DIR.OMP.SIMD(),  QUAL.OMP.SIMDLEN(8),  QUAL.OMP.NORMALIZED.IV(null),  QUAL.OMP.NORMALIZED.UB(null),  QUAL.OMP.LINEAR:IV(&((%i.i.i.i8.linear.iv)[0])1) ]
; CHECK:       |   @llvm.directive.region.exit(%1); [ DIR.OMP.END.SIMD() ]
; CHECK:       + END LOOP
; CHECK: END REGION

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable
define dso_local noundef i32 @main() local_unnamed_addr #0 {
entry:
  %i.i.i.i.linear.iv = alloca i32, align 4
  %i.i.i.i8.linear.iv = alloca i32, align 4
  br label %DIR.OMP.END.SIMD.2

DIR.OMP.END.SIMD.2:                               ; preds = %entry, %DIR.OMP.END.SIMD.3
  %i.0.i144 = phi i32 [ 0, %entry ], [ %inc.i5, %DIR.OMP.END.SIMD.3 ]
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %DIR.OMP.END.SIMD.2
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 8), "QUAL.OMP.NORMALIZED.IV"(i8* null), "QUAL.OMP.NORMALIZED.UB"(i8* null), "QUAL.OMP.LINEAR:IV"(i32* %i.i.i.i.linear.iv, i32 1) ]
  br label %DIR.OMP.END.SIMD.3

DIR.OMP.END.SIMD.3:                               ; preds = %DIR.OMP.SIMD.1
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  %inc.i5 = add nuw nsw i32 %i.0.i144, 1
  %exitcond.not = icmp eq i32 %inc.i5, 4
  br i1 %exitcond.not, label %for.cond.i15.preheader, label %DIR.OMP.END.SIMD.2

for.cond.i15.preheader:                           ; preds = %DIR.OMP.END.SIMD.3
  br label %DIR.OMP.END.SIMD.4

DIR.OMP.END.SIMD.4:                               ; preds = %for.cond.i15.preheader, %DIR.OMP.END.SIMD.7
  %i.0.i1345 = phi i32 [ 0, %for.cond.i15.preheader ], [ %inc.i29, %DIR.OMP.END.SIMD.7 ]
  br label %DIR.OMP.SIMD.5

DIR.OMP.SIMD.5:                                   ; preds = %DIR.OMP.END.SIMD.4
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 8), "QUAL.OMP.NORMALIZED.IV"(i8* null), "QUAL.OMP.NORMALIZED.UB"(i8* null), "QUAL.OMP.LINEAR:IV"(i32* %i.i.i.i8.linear.iv, i32 1) ]
  br label %DIR.OMP.END.SIMD.7

DIR.OMP.END.SIMD.7:                               ; preds = %DIR.OMP.SIMD.5
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.SIMD"() ]
  %inc.i29 = add nuw nsw i32 %i.0.i1345, 1
  %exitcond49.not = icmp eq i32 %inc.i29, 4
  br i1 %exitcond49.not, label %_Z10dump_nodesILi4EEvRK7Nodes1DIXT_EE.exit31, label %DIR.OMP.END.SIMD.4

_Z10dump_nodesILi4EEvRK7Nodes1DIXT_EE.exit31:     ; preds = %DIR.OMP.END.SIMD.7
  ret i32 0
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1
