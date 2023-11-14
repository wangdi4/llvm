; RUN: opt -passes=loop-simplify -S %s | FileCheck %s

; The inner loop (body26) has a branch that latches both the inner and outer
; loops. The 2nd edge exits the inner loop and directly goes to the header
; of the outer loop.
; When the 2nd edge is split, the metadata should not be copied, as it belongs
; with the inner loop only.

; CHECK-LABEL: dispatch.header6.loopexit
; CHECK-NOT: br label %dispatch.header6{{.*}} llvm.loop

; CHECK-LABEL: omp.pdo.body26
; CHECK: br i1 %rel.8.not.not{{.*}}llvm.loop

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"QNCA_a0$i32*$rank1$" = type { i32*, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }

declare i32 @__kmpc_dispatch_next_4()

define internal void @MAIN__.DIR.OMP.PARALLEL.LOOP.2119.split125(%"QNCA_a0$i32*$rank1$"* %"main_$ARRAY") {
DIR.OMP.PARALLEL.LOOP.2:
  br label %dispatch.header6

dispatch.header6:                                 ; preds = %omp.pdo.body26, %DIR.OMP.PARALLEL.LOOP.2
  %0 = call i32 @__kmpc_dispatch_next_4()
  %dispatch.cond6.not = icmp eq i32 %0, 0
  br i1 %dispatch.cond6.not, label %DIR.OMP.END.PARALLEL.LOOP.5121.exitStub, label %omp.pdo.body26

omp.pdo.body26:                                   ; preds = %dispatch.header6, %omp.pdo.body26
  %omp.pdo.norm.iv.local.0124 = phi i32 [ %add.8, %omp.pdo.body26 ], [ 0, %dispatch.header6 ]
  %"main_$ARRAY.addr_a0$85" = getelementptr inbounds %"QNCA_a0$i32*$rank1$", %"QNCA_a0$i32*$rank1$"* %"main_$ARRAY", i64 0, i32 0
  %"main_$ARRAY.addr_a0$_fetch.44" = load i32*, i32** %"main_$ARRAY.addr_a0$85", align 8
  store i32 0, i32* %"main_$ARRAY.addr_a0$_fetch.44", align 4
  %add.8 = add nuw nsw i32 %omp.pdo.norm.iv.local.0124, 1
  %rel.8.not.not = icmp ugt i32 %omp.pdo.norm.iv.local.0124, 1
  br i1 %rel.8.not.not, label %dispatch.header6, label %omp.pdo.body26, !llvm.loop !5

DIR.OMP.END.PARALLEL.LOOP.5121.exitStub:          ; preds = %dispatch.header6
  ret void
}

!llvm.module.flags = !{!0, !1, !2}
!llvm.dbg.cu = !{!3}

!0 = !{i32 2, !"Debug Info Version", i32 3}
!1 = !{i32 2, !"Dwarf Version", i32 4}
!2 = !{i32 7, !"openmp", i32 50}
!3 = distinct !DICompileUnit(language: DW_LANG_Fortran95, file: !4, producer: "Intel(R) Fortran 22.0-1775", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, splitDebugInlining: false, nameTableKind: None)
!4 = !DIFile(filename: "file.f90", directory: "dir")
!5 = distinct !{!5, !6, !7, !9}
!6 = !{!"llvm.loop.vectorize.ivdep_loop", i32 0}
!7 = !{!"llvm.loop.parallel_accesses", !8}
!8 = distinct !{}
!9 = distinct !{!"intel.optreport", !11, !17}
!11 = !{!"intel.optreport.debug_location", !12}
!12 = !DILocation(line: 10, column: 3, scope: !13)
!13 = distinct !DILexicalBlock(scope: !14, file: !4, line: 9, column: 9)
!14 = distinct !DISubprogram(name: "MAIN__.DIR.OMP.PARALLEL.LOOP.2119.split125", scope: !4, file: !4, line: 9, type: !15, scopeLine: 9, flags: DIFlagArtificial, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !3, retainedNodes: !16)
!15 = !DISubroutineType(types: !16)
!16 = !{}
!17 = !{!"intel.optreport.remarks", !18}
!18 = !{!"intel.optreport.remark", i32 0, !"OpenMP: Outlined parallel loop"}
