; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-loop-rematerialize -hir-dd-analysis -analyze -enable-new-pm=0 -hir-dd-analysis-verify=Region < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-loop-rematerialize,print<hir-dd-analysis>" -aa-pipeline="basic-aa" -hir-dd-analysis-verify=Region < %s 2>&1 | FileCheck %s

; Check that invalidation after loop rematerialization happened. No spurious dd-edge remains.

; Resulting HIR -
; <0>          BEGIN REGION { }
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
; <41>               ret ;
; <0>          END REGION

; 3:21 %1 --> %1 FLOW (=) (0)
; 17:21 %8 --> %8 FLOW (=) (0)
; 19:21 %9 --> %9 FLOW (=) (0)
; 15:21 %7 --> %7 FLOW (=) (0)
; 7:21 %3 --> %3 FLOW (=) (0)
; 2:21 %0 --> %0 FLOW (=) (0)
; 11:21 %5 --> %5 FLOW (=) (0)
; 5:21 %2 --> %2 FLOW (=) (0)
; 13:21 %6 --> %6 FLOW (=) (0)
; 9:21 %4 --> %4 FLOW (=) (0)

; CHECK-COUNT-10: FLOW (=) (0)

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
