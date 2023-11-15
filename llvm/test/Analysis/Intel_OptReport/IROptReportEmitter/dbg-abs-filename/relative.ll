; RUN: opt -passes="intel-ir-optreport-emitter" -disable-output -intel-opt-report-file=stdout < %s | FileCheck %s --strict-whitespace
; RUN: opt -passes="intel-ir-optreport-emitter" -disable-output -intel-opt-report-file=stdout -intel-opt-report-use-absolute-paths < %s | FileCheck %s --strict-whitespace --check-prefix=ABSOLUTE

; UNSUPPORTED: system-windows

; Check that the opt-report is generated using relative paths if the source file
; is specified with a relative path, unless the option is used.

; CHECK: LOOP BEGIN at ../src/iota.c (2, 3)
; ABSOLUTE: LOOP BEGIN at /home/dwoodwor/opt-report-abs-filename/example/build/../src/iota.c (2, 3)

source_filename = "../src/iota.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @iota(ptr nocapture noundef writeonly %A) !dbg !3 {
entry:
  br label %loop.14

loop.14:                                          ; preds = %loop.14, %entry
  %i1.i64.0 = phi i64 [ 0, %entry ], [ %nextivloop.14, %loop.14 ]
  %0 = getelementptr inbounds i32, ptr %A, i64 %i1.i64.0, !dbg !6
  %1 = trunc i64 %i1.i64.0 to i32, !dbg !7
  %2 = insertelement <4 x i32> undef, i32 %1, i64 0, !dbg !7
  %3 = shufflevector <4 x i32> %2, <4 x i32> poison, <4 x i32> zeroinitializer, !dbg !7
  %4 = or <4 x i32> %3, <i32 0, i32 1, i32 2, i32 3>, !dbg !7
  store <4 x i32> %4, ptr %0, align 4, !dbg !7
  %nextivloop.14 = add nuw nsw i64 %i1.i64.0, 4, !dbg !8
  %condloop.14 = icmp ult i64 %i1.i64.0, 2044, !dbg !8
  br i1 %condloop.14, label %loop.14, label %afterloop.14, !dbg !9, !llvm.loop !10

afterloop.14:                                     ; preds = %loop.14
  ret void, !dbg !20
}

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2}

!0 = distinct !DICompileUnit(language: DW_LANG_C11, file: !1, producer: "clang based Intel(R) oneAPI DPC++/C++ Compiler 2024.0.0 (2024.0.0.YYYYMMDD)", isOptimized: true, flags: " --intel -O3 -x CORE-AVX512 -qopt-report=2 -c ../src/iota.c -mllvm -print-before=intel-ir-optreport-emitter -mllvm -print-module-scope -qopt-report-file=stderr -fveclib=SVML -fheinous-gnu-extensions", runtimeVersion: 0, emissionKind: LineTablesOnly, splitDebugInlining: false, nameTableKind: None)
!1 = !DIFile(filename: "../src/iota.c", directory: "/home/dwoodwor/opt-report-abs-filename/example/build")
!2 = !{i32 2, !"Debug Info Version", i32 3}
!3 = distinct !DISubprogram(name: "iota", scope: !1, file: !1, line: 1, type: !4, scopeLine: 1, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0)
!4 = !DISubroutineType(types: !5)
!5 = !{}
!6 = !DILocation(line: 3, column: 5, scope: !3)
!7 = !DILocation(line: 3, column: 10, scope: !3)
!8 = !DILocation(line: 2, column: 21, scope: !3)
!9 = !DILocation(line: 2, column: 3, scope: !3)
!10 = distinct !{!10, !9, !11, !12, !13, !14, !15}
!11 = !DILocation(line: 3, column: 12, scope: !3)
!12 = !{!"llvm.loop.mustprogress"}
!13 = !{!"llvm.loop.vectorize.width", i32 1}
!14 = !{!"llvm.loop.interleave.count", i32 1}
!15 = distinct !{!"intel.optreport", !17}
!17 = !{!"intel.optreport.remarks", !18, !19}
!18 = !{!"intel.optreport.remark", i32 15300}
!19 = !{!"intel.optreport.remark", i32 15305, !"4"}
!20 = !DILocation(line: 4, column: 1, scope: !3)
