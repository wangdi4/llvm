; XFAIL: *
; Test to verify correctness of PHI node placement and fixing by HIR decomposer. There can be some inaccuracies
; in HIR's DDG edge information. To accommdate this we allow NULL for incoming values of PHI nodes in HCFG.
; However such PHI nodes are invalid and will not pass HCFG verification, hence they need to be replaced and
; removed.

; Incoming HIR -
; <0>          BEGIN REGION { }
; <43>               @llvm.intel.directive(!2);
; <44>               @llvm.intel.directive(!3);
; <42>
; <42>               + DO i1 = 0, 19, 1   <DO_LOOP>
; <2>                |   %0 = (@b)[0][0];
; <3>                |   %1 = (@b)[0][1];
; <5>                |   %2 = (@b)[0][2];
; <7>                |   %3 = (@b)[0][3];
; <9>                |   %4 = (@b)[0][4];
; <11>               |   %5 = (@b)[0][5];
; <13>               |   %6 = (@b)[0][6];
; <15>               |   %7 = (@b)[0][7];
; <17>               |   %8 = (@b)[0][8];
; <19>               |   %9 = (@b)[0][9];
; <21>               |   (@a)[0][i1] = %0 + %1 + %2 + %3 + %4 + %5 + %6 + %7 + %8 + %9;
; <42>               + END LOOP
; <42>
; <46>               @llvm.intel.directive(!4);
; <45>               @llvm.intel.directive(!3);
; <41>               ret ;
; <0>          END REGION

; For this example, DDG incorrectly reports that there are 2 reaching definitions for %0, %1 ... %9 leading to
; insertion of unnecessary PHI nodes in HCFG. We replace and remove such PHI nodes in fixPhiNodes().

; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-last-value-computation -hir-loop-rematerialize -hir-vec-dir-insert -hir-vplan-vec -vplan-print-after-plain-cfg -debug-only=vplan-decomposer < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-vec-dir-insert,hir-vplan-vec" -hir-last-value-computation -hir-loop-rematerialize -vplan-print-after-plain-cfg -debug-only=vplan-decomposer < %s 2>&1 | FileCheck %s

; REQUIRES: asserts

; Check that 9 invalid PHI nodes are replaced and removed
; CHECK-COUNT-9: VPDecomp fixPhiNodes : The fixed PHI node will be replaced and removed
; CHECK-LABEL: VPlan after importing plain CFG
; CHECK: phi
; CHECK-NOT: phi


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@a = common dso_local local_unnamed_addr global [20 x i32] zeroinitializer, align 16
@b = common dso_local local_unnamed_addr global [20 x i32] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define dso_local void @kernel() local_unnamed_addr {
entry:
  %0 = load i32, i32* getelementptr inbounds ([20 x i32], [20 x i32]* @b, i64 0, i64 0), align 16, !tbaa !2
  %1 = load i32, i32* getelementptr inbounds ([20 x i32], [20 x i32]* @b, i64 0, i64 1), align 4, !tbaa !2
  %add = add nsw i32 %1, %0
  %2 = load i32, i32* getelementptr inbounds ([20 x i32], [20 x i32]* @b, i64 0, i64 2), align 8, !tbaa !2
  %add3 = add nsw i32 %add, %2
  %3 = load i32, i32* getelementptr inbounds ([20 x i32], [20 x i32]* @b, i64 0, i64 3), align 4, !tbaa !2
  %add5 = add nsw i32 %add3, %3
  %4 = load i32, i32* getelementptr inbounds ([20 x i32], [20 x i32]* @b, i64 0, i64 4), align 16, !tbaa !2
  %add7 = add nsw i32 %add5, %4
  %5 = load i32, i32* getelementptr inbounds ([20 x i32], [20 x i32]* @b, i64 0, i64 5), align 4, !tbaa !2
  %add9 = add nsw i32 %add7, %5
  %6 = load i32, i32* getelementptr inbounds ([20 x i32], [20 x i32]* @b, i64 0, i64 6), align 8, !tbaa !2
  %add11 = add nsw i32 %add9, %6
  %7 = load i32, i32* getelementptr inbounds ([20 x i32], [20 x i32]* @b, i64 0, i64 7), align 4, !tbaa !2
  %add13 = add nsw i32 %add11, %7
  %8 = load i32, i32* getelementptr inbounds ([20 x i32], [20 x i32]* @b, i64 0, i64 8), align 16, !tbaa !2
  %add15 = add nsw i32 %add13, %8
  %9 = load i32, i32* getelementptr inbounds ([20 x i32], [20 x i32]* @b, i64 0, i64 9), align 4, !tbaa !2
  %add17 = add nsw i32 %add15, %9
  store i32 %add17, i32* getelementptr inbounds ([20 x i32], [20 x i32]* @a, i64 0, i64 0), align 16, !tbaa !2
  store i32 %add17, i32* getelementptr inbounds ([20 x i32], [20 x i32]* @a, i64 0, i64 1), align 4, !tbaa !2
  store i32 %add17, i32* getelementptr inbounds ([20 x i32], [20 x i32]* @a, i64 0, i64 2), align 8, !tbaa !2
  store i32 %add17, i32* getelementptr inbounds ([20 x i32], [20 x i32]* @a, i64 0, i64 3), align 4, !tbaa !2
  store i32 %add17, i32* getelementptr inbounds ([20 x i32], [20 x i32]* @a, i64 0, i64 4), align 16, !tbaa !2
  store i32 %add17, i32* getelementptr inbounds ([20 x i32], [20 x i32]* @a, i64 0, i64 5), align 4, !tbaa !2
  store i32 %add17, i32* getelementptr inbounds ([20 x i32], [20 x i32]* @a, i64 0, i64 6), align 8, !tbaa !2
  store i32 %add17, i32* getelementptr inbounds ([20 x i32], [20 x i32]* @a, i64 0, i64 7), align 4, !tbaa !2
  store i32 %add17, i32* getelementptr inbounds ([20 x i32], [20 x i32]* @a, i64 0, i64 8), align 16, !tbaa !2
  store i32 %add17, i32* getelementptr inbounds ([20 x i32], [20 x i32]* @a, i64 0, i64 9), align 4, !tbaa !2
  store i32 %add17, i32* getelementptr inbounds ([20 x i32], [20 x i32]* @a, i64 0, i64 10), align 8, !tbaa !2
  store i32 %add17, i32* getelementptr inbounds ([20 x i32], [20 x i32]* @a, i64 0, i64 11), align 4, !tbaa !2
  store i32 %add17, i32* getelementptr inbounds ([20 x i32], [20 x i32]* @a, i64 0, i64 12), align 16, !tbaa !2
  store i32 %add17, i32* getelementptr inbounds ([20 x i32], [20 x i32]* @a, i64 0, i64 13), align 4, !tbaa !2
  store i32 %add17, i32* getelementptr inbounds ([20 x i32], [20 x i32]* @a, i64 0, i64 14), align 8, !tbaa !2
  store i32 %add17, i32* getelementptr inbounds ([20 x i32], [20 x i32]* @a, i64 0, i64 15), align 4, !tbaa !2
  store i32 %add17, i32* getelementptr inbounds ([20 x i32], [20 x i32]* @a, i64 0, i64 16), align 16, !tbaa !2
  store i32 %add17, i32* getelementptr inbounds ([20 x i32], [20 x i32]* @a, i64 0, i64 17), align 4, !tbaa !2
  store i32 %add17, i32* getelementptr inbounds ([20 x i32], [20 x i32]* @a, i64 0, i64 18), align 8, !tbaa !2
  store i32 %add17, i32* getelementptr inbounds ([20 x i32], [20 x i32]* @a, i64 0, i64 19), align 4, !tbaa !2
  ret void
}


!2 = !{!3, !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
!6 = !{!7, !3, i64 0}
!7 = !{!"array@_ZTSA20_i", !3, i64 0}
