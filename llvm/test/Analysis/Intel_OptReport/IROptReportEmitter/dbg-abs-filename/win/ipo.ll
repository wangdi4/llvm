; RUN: opt -passes="intel-ir-optreport-emitter" -disable-output -intel-opt-report-file=stdout < %s | FileCheck %s --strict-whitespace

; REQUIRES: system-windows

; In a trickier IPO case with a mix of relative and absolute paths, check that
; the IPO opt-report uses absolute paths

; CHECK: LOOP BEGIN at C:\Users\dwoodwor\opt-report-abs-filename\example\build\..\src\iota.c (2, 3)

source_filename = "ld-temp.o"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = constant [12 x i8] c"A[100] = %d\00"

define i32 @main() !dbg !8 {
  %1 = alloca [2048 x i32], align 16
  br label %2

2:                                                ; preds = %2, %0
  %3 = phi i64 [ 0, %0 ], [ %9, %2 ]
  %4 = getelementptr inbounds [2048 x i32], ptr %1, i64 0, i64 %3, !dbg !12
  %5 = trunc i64 %3 to i32, !dbg !15
  %6 = insertelement <4 x i32> undef, i32 %5, i64 0, !dbg !15
  %7 = shufflevector <4 x i32> %6, <4 x i32> poison, <4 x i32> zeroinitializer, !dbg !15
  %8 = or <4 x i32> %7, <i32 0, i32 1, i32 2, i32 3>, !dbg !15
  store <4 x i32> %8, ptr %4, align 16, !dbg !15
  %9 = add nuw nsw i64 %3, 4, !dbg !16
  %10 = icmp ult i64 %3, 2044, !dbg !16
  br i1 %10, label %2, label %11, !dbg !17, !llvm.loop !18

11:                                               ; preds = %2
  %12 = getelementptr inbounds [2048 x i32], ptr %1, i64 0, i64 100, !dbg !28
  %13 = load i32, ptr %12, align 16, !dbg !28
  %14 = tail call i32 (ptr, ...) @printf(ptr noundef nonnull dereferenceable(1) @.str, i32 noundef %13), !dbg !29
  ret i32 0, !dbg !30
}

declare i32 @printf(ptr, ...)

!llvm.dbg.cu = !{!0, !2}
!llvm.module.flags = !{!4, !5, !6, !7}

!0 = distinct !DICompileUnit(language: DW_LANG_C11, file: !1, producer: "clang based Intel(R) oneAPI DPC++/C++ Compiler 2024.0.0 (2024.0.0.YYYYMMDD)", isOptimized: true, flags: " --intel -flto=full -O3 -x CORE-AVX512 -qopt-report=2 -o iota ..\\src\\iota.c C:\\Users\\dwoodwor\\opt-report-abs-filename\\example\\src\\main.c -mllvm -print-before=intel-ir-optreport-emitter -mllvm -print-module-scope -qopt-report-file=stderr -fveclib=SVML -fheinous-gnu-extensions -dumpdir iota-", runtimeVersion: 0, emissionKind: LineTablesOnly, splitDebugInlining: false, nameTableKind: None)
!1 = !DIFile(filename: "..\\src\\iota.c", directory: "C:\\Users\\dwoodwor\\opt-report-abs-filename\\example\\build")
!2 = distinct !DICompileUnit(language: DW_LANG_C11, file: !3, producer: "clang based Intel(R) oneAPI DPC++/C++ Compiler 2024.0.0 (2024.0.0.YYYYMMDD)", isOptimized: true, flags: " --intel -flto=full -O3 -x CORE-AVX512 -qopt-report=2 -o iota ..\\src\\iota.c C:\\Users\\dwoodwor\\opt-report-abs-filename\\example\\src\\main.c -mllvm -print-before=intel-ir-optreport-emitter -mllvm -print-module-scope -qopt-report-file=stderr -fveclib=SVML -fheinous-gnu-extensions -dumpdir iota-", runtimeVersion: 0, emissionKind: LineTablesOnly, splitDebugInlining: false, nameTableKind: None)
!3 = !DIFile(filename: "C:\\Users\\dwoodwor\\opt-report-abs-filename\\example\\src\\main.c", directory: "C:\\Users\\dwoodwor\\opt-report-abs-filename\\example\\build")
!4 = !{i32 2, !"Debug Info Version", i32 3}
!5 = !{i32 1, !"ThinLTO", i32 0}
!6 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!7 = !{i32 1, !"LTOPostLink", i32 1}
!8 = distinct !DISubprogram(name: "main", scope: !9, file: !9, line: 5, type: !10, scopeLine: 5, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !2)
!9 = !DIFile(filename: "src\\main.c", directory: "C:\\Users\\dwoodwor\\opt-report-abs-filename\\example")
!10 = !DISubroutineType(types: !11)
!11 = !{}
!12 = !DILocation(line: 3, column: 5, scope: !13, inlinedAt: !14)
!13 = distinct !DISubprogram(name: "iota", scope: !1, file: !1, line: 1, type: !10, scopeLine: 1, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0)
!14 = distinct !DILocation(line: 7, column: 3, scope: !8)
!15 = !DILocation(line: 3, column: 10, scope: !13, inlinedAt: !14)
!16 = !DILocation(line: 2, column: 21, scope: !13, inlinedAt: !14)
!17 = !DILocation(line: 2, column: 3, scope: !13, inlinedAt: !14)
!18 = distinct !{!18, !17, !19, !20, !21, !22, !23}
!19 = !DILocation(line: 3, column: 12, scope: !13, inlinedAt: !14)
!20 = !{!"llvm.loop.mustprogress"}
!21 = !{!"llvm.loop.vectorize.width", i32 1}
!22 = !{!"llvm.loop.interleave.count", i32 1}
!23 = distinct !{!"intel.optreport", !25}
!25 = !{!"intel.optreport.remarks", !26, !27}
!26 = !{!"intel.optreport.remark", i32 15300}
!27 = !{!"intel.optreport.remark", i32 15305, !"4"}
!28 = !DILocation(line: 8, column: 25, scope: !8)
!29 = !DILocation(line: 8, column: 3, scope: !8)
!30 = !DILocation(line: 9, column: 1, scope: !8)
