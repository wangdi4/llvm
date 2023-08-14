; RUN: opt -passes="hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec,hir-optreport-emitter" -intel-opt-report=low -intel-opt-report-file=stdout -disable-output < %s | FileCheck %s

; UNSUPPORTED: system-windows

; Check that automatic absolute path printing works in the HIR opt-report
; emitter too.

; CHECK: LOOP BEGIN at /home/dwoodwor/opt-report-abs-filename/example/build/../src/iota.c (2, 3)

source_filename = "ld-temp.o"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = constant [12 x i8] c"A[100] = %d\00"

define i32 @main() #0 !dbg !8 {
  %1 = alloca [2048 x i32], align 16
  br label %2, !dbg !12

2:                                                ; preds = %2, %0
  %3 = phi i64 [ 0, %0 ], [ %6, %2 ]
  %4 = getelementptr inbounds [2048 x i32], ptr %1, i64 0, i64 %3, !dbg !15
  %5 = trunc i64 %3 to i32, !dbg !16
  store i32 %5, ptr %4, align 4, !dbg !16
  %6 = add nuw nsw i64 %3, 1, !dbg !17
  %7 = icmp eq i64 %6, 2048, !dbg !18
  br i1 %7, label %8, label %2, !dbg !12, !llvm.loop !19

8:                                                ; preds = %2
  %9 = getelementptr inbounds [2048 x i32], ptr %1, i64 0, i64 100, !dbg !22
  %10 = load i32, ptr %9, align 16, !dbg !22
  %11 = tail call i32 (ptr, ...) @printf(ptr noundef nonnull dereferenceable(1) @.str, i32 noundef %10), !dbg !23
  ret i32 0, !dbg !24
}

declare i32 @printf(ptr, ...)

attributes #0 = { "min-legal-vector-width"="0" "target-cpu"="skylake-avx512" }

!llvm.dbg.cu = !{!0, !2}
!llvm.module.flags = !{!4, !5, !6, !7}

!0 = distinct !DICompileUnit(language: DW_LANG_C11, file: !1, producer: "clang based Intel(R) oneAPI DPC++/C++ Compiler 2024.0.0 (2024.x.0.YYYYMMDD)", isOptimized: true, flags: " --intel -flto=full -O3 -x CORE-AVX512 -qopt-report=2 -mllvm -intel-opt-report-emitter=hir ../src/iota.c /home/dwoodwor/opt-report-abs-filename/example/src/main.c -o iota -mllvm -print-module-before-loopopt -fveclib=SVML -fheinous-gnu-extensions -dumpdir iota-", runtimeVersion: 0, emissionKind: LineTablesOnly, splitDebugInlining: false, nameTableKind: None)
!1 = !DIFile(filename: "../src/iota.c", directory: "/home/dwoodwor/opt-report-abs-filename/example/build")
!2 = distinct !DICompileUnit(language: DW_LANG_C11, file: !3, producer: "clang based Intel(R) oneAPI DPC++/C++ Compiler 2024.0.0 (2024.x.0.YYYYMMDD)", isOptimized: true, flags: " --intel -flto=full -O3 -x CORE-AVX512 -qopt-report=2 -mllvm -intel-opt-report-emitter=hir ../src/iota.c /home/dwoodwor/opt-report-abs-filename/example/src/main.c -o iota -mllvm -print-module-before-loopopt -fveclib=SVML -fheinous-gnu-extensions -dumpdir iota-", runtimeVersion: 0, emissionKind: LineTablesOnly, splitDebugInlining: false, nameTableKind: None)
!3 = !DIFile(filename: "/home/dwoodwor/opt-report-abs-filename/example/src/main.c", directory: "/home/dwoodwor/opt-report-abs-filename/example/build")
!4 = !{i32 2, !"Debug Info Version", i32 3}
!5 = !{i32 1, !"ThinLTO", i32 0}
!6 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!7 = !{i32 1, !"LTOPostLink", i32 1}
!8 = distinct !DISubprogram(name: "main", scope: !9, file: !9, line: 5, type: !10, scopeLine: 5, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !2)
!9 = !DIFile(filename: "src/main.c", directory: "/home/dwoodwor/opt-report-abs-filename/example")
!10 = !DISubroutineType(types: !11)
!11 = !{}
!12 = !DILocation(line: 2, column: 3, scope: !13, inlinedAt: !14)
!13 = distinct !DISubprogram(name: "iota", scope: !1, file: !1, line: 1, type: !10, scopeLine: 1, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0)
!14 = distinct !DILocation(line: 7, column: 3, scope: !8)
!15 = !DILocation(line: 3, column: 5, scope: !13, inlinedAt: !14)
!16 = !DILocation(line: 3, column: 10, scope: !13, inlinedAt: !14)
!17 = !DILocation(line: 2, column: 29, scope: !13, inlinedAt: !14)
!18 = !DILocation(line: 2, column: 21, scope: !13, inlinedAt: !14)
!19 = distinct !{!19, !12, !20, !21}
!20 = !DILocation(line: 3, column: 12, scope: !13, inlinedAt: !14)
!21 = !{!"llvm.loop.mustprogress"}
!22 = !DILocation(line: 8, column: 25, scope: !8)
!23 = !DILocation(line: 8, column: 3, scope: !8)
!24 = !DILocation(line: 9, column: 1, scope: !8)
