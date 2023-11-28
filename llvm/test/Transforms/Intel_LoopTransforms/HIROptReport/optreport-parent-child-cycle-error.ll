; RUN: not opt -passes="hir-ssa-deconstruction,hir-post-vec-complete-unroll" -intel-opt-report=low -disable-output <%s 2>&1 | FileCheck %s

; Check that we emit an error when emitting an opt-report would cause a circular
; dependence, and hence a compiler hang, due to both parent and child loops
; having the same loop ID.

; CHECK: LLVM ERROR: Found a parent/child cycle when generating opt-report. Proceeding will cause an infinite loop.

; Test src:

; program main
;   implicit none
;   integer :: i,n
;   integer,allocatable :: array(:), input(:)
;
;   read(*,*) n
;   allocate(array(n), input(n))
;   input=1
;   !$omp parallel do private(i) schedule(dynamic,1)
;   do i=1,n
;      array(i)=input(i)
;   end do
;
; end program

; HIR dump:

;  BEGIN REGION { }
;        + UNKNOWN LOOP i1 <ivdep>
;        |   <i1 = 0>
;        |   bb12:
;        |
;        |   + DO i2 = 0, 2, 1   <DO_LOOP> <ivdep>
;        |   |   %tmp6 = (%arg)[0].0;
;        |   |   (%tmp6)[0] = 0;
;        |   + END LOOP
;        |
;        |   %tmp10 = @snork();
;        |   if (%tmp10 != 0)
;        |   {
;        |      <i1 = i1 + 1>
;        |      goto bb12;
;        |   }
;        + END LOOP
;  END REGION

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.widget = type { ptr, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }

define internal void @baz(ptr nocapture readonly %arg) #0 {
bb:
  %tmp = tail call i32 @snork()
  %tmp1 = icmp eq i32 %tmp, 0
  br i1 %tmp1, label %bb14, label %bb2

bb2:                                              ; preds = %bb
  br label %bb12

bb4:                                              ; preds = %bb12, %bb4
  %tmp5 = phi i32 [ %tmp7, %bb4 ], [ 0, %bb12 ]
  %tmp6 = load ptr, ptr %arg, align 8
  store i32 0, ptr %tmp6, align 4
  %tmp7 = add nuw nsw i32 %tmp5, 1
  %tmp8 = icmp eq i32 %tmp7, 3
  br i1 %tmp8, label %bb9, label %bb4, !llvm.loop !5

bb9:                                              ; preds = %bb4
  %tmp10 = tail call i32 @snork()
  %tmp11 = icmp eq i32 %tmp10, 0
  br i1 %tmp11, label %bb13, label %bb12, !llvm.loop !5

bb12:                                             ; preds = %bb9, %bb2
  br label %bb4

bb13:                                             ; preds = %bb9
  br label %bb14

bb14:                                             ; preds = %bb13, %bb
  ret void
}

declare i32 @snork() local_unnamed_addr

attributes #0 = { "pre_loopopt" }

!llvm.module.flags = !{!0, !1, !2}

!0 = !{i32 7, !"openmp", i32 50}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = !{i32 2, !"Dwarf Version", i32 4}
!3 = distinct !DICompileUnit(language: DW_LANG_Fortran95, file: !4, producer: "Intel(R) Fortran 22.0-xxxx", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, splitDebugInlining: false, nameTableKind: None)
!4 = !DIFile(filename: "test.f90", directory: "/path/to/test")
!5 = distinct !{!5, !6, !7, !9}
!6 = !{!"llvm.loop.vectorize.ivdep_loop", i32 0}
!7 = !{!"llvm.loop.parallel_accesses", !8}
!8 = distinct !{}
!9 = distinct !{!"intel.optreport", !11, !17}
!11 = !{!"intel.optreport.debug_location", !12}
!12 = !DILocation(line: 10, column: 3, scope: !13)
!13 = distinct !DILexicalBlock(scope: !14, file: !4, line: 9, column: 9)
!14 = distinct !DISubprogram(name: "baz", scope: !4, file: !4, line: 9, type: !15, scopeLine: 9, flags: DIFlagArtificial, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !3, retainedNodes: !16)
!15 = !DISubroutineType(types: !16)
!16 = !{}
!17 = !{!"intel.optreport.remarks", !18}
!18 = !{!"intel.optreport.remark", i32 0, !"OpenMP: Outlined parallel loop"}
