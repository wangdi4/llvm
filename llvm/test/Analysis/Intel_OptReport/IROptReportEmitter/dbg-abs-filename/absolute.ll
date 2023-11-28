; RUN: opt -passes="intel-ir-optreport-emitter" -disable-output -intel-opt-report-file=stdout < %s | FileCheck %s --strict-whitespace
; RUN: opt -passes="intel-ir-optreport-emitter" -disable-output -intel-opt-report-file=stdout -intel-opt-report-use-absolute-paths=0 < %s | FileCheck %s --strict-whitespace --check-prefix=RELATIVE

; UNSUPPORTED: system-windows

; Check that the opt-report is generated using absolute paths if the source file
; is specified with an absolute path, unless the option is used.

; CHECK: LOOP BEGIN at /home/dwoodwor/opt-report-abs-filename/example/src/iota.c (2, 3)
; RELATIVE: LOOP BEGIN at src/iota.c (2, 3)

source_filename = "/home/dwoodwor/opt-report-abs-filename/example/src/iota.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @iota(ptr nocapture noundef writeonly %A) !dbg !3 {
entry:
  br label %loop.14

loop.14:                                          ; preds = %loop.14, %entry
  %i1.i64.0 = phi i64 [ 0, %entry ], [ %nextivloop.14, %loop.14 ]
  %0 = getelementptr inbounds i32, ptr %A, i64 %i1.i64.0, !dbg !7
  %1 = trunc i64 %i1.i64.0 to i32, !dbg !8
  %2 = insertelement <4 x i32> undef, i32 %1, i64 0, !dbg !8
  %3 = shufflevector <4 x i32> %2, <4 x i32> poison, <4 x i32> zeroinitializer, !dbg !8
  %4 = or <4 x i32> %3, <i32 0, i32 1, i32 2, i32 3>, !dbg !8
  store <4 x i32> %4, ptr %0, align 4, !dbg !8
  %nextivloop.14 = add nuw nsw i64 %i1.i64.0, 4, !dbg !9
  %condloop.14 = icmp ult i64 %i1.i64.0, 2044, !dbg !9
  br i1 %condloop.14, label %loop.14, label %afterloop.14, !dbg !10, !llvm.loop !11

afterloop.14:                                     ; preds = %loop.14
  ret void, !dbg !21
}

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2}

!0 = distinct !DICompileUnit(language: DW_LANG_C11, file: !1, producer: "clang based Intel(R) oneAPI DPC++/C++ Compiler 2024.0.0 (2024.0.0.YYYYMMDD)", isOptimized: true, flags: " --intel -O3 -x CORE-AVX512 -qopt-report=2 -c /home/dwoodwor/opt-report-abs-filename/example/src/iota.c -mllvm -print-before=intel-ir-optreport-emitter -mllvm -print-module-scope -qopt-report-file=stderr -fveclib=SVML -fheinous-gnu-extensions", runtimeVersion: 0, emissionKind: LineTablesOnly, splitDebugInlining: false, nameTableKind: None)
!1 = !DIFile(filename: "/home/dwoodwor/opt-report-abs-filename/example/src/iota.c", directory: "/home/dwoodwor/opt-report-abs-filename/example/build")
!2 = !{i32 2, !"Debug Info Version", i32 3}
!3 = distinct !DISubprogram(name: "iota", scope: !4, file: !4, line: 1, type: !5, scopeLine: 1, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0)
!4 = !DIFile(filename: "src/iota.c", directory: "/home/dwoodwor/opt-report-abs-filename/example")
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = !DILocation(line: 3, column: 5, scope: !3)
!8 = !DILocation(line: 3, column: 10, scope: !3)
!9 = !DILocation(line: 2, column: 21, scope: !3)
!10 = !DILocation(line: 2, column: 3, scope: !3)
!11 = distinct !{!11, !10, !12, !13, !14, !15, !16}
!12 = !DILocation(line: 3, column: 12, scope: !3)
!13 = !{!"llvm.loop.mustprogress"}
!14 = !{!"llvm.loop.vectorize.width", i32 1}
!15 = !{!"llvm.loop.interleave.count", i32 1}
!16 = distinct !{!"intel.optreport", !18}
!18 = !{!"intel.optreport.remarks", !19, !20}
!19 = !{!"intel.optreport.remark", i32 15300}
!20 = !{!"intel.optreport.remark", i32 15305, !"4"}
!21 = !DILocation(line: 4, column: 1, scope: !3)
