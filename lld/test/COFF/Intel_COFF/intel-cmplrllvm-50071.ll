; UNSUPPORTED: !system-windows
; RUN: llvm-as -o %t.obj %s
; RUN: env LLD_IN_TEST=0 lld-link -out:mymain.exe -opt:lldlto=2 -mllvm:-intel-opt-report-emitter=ir -mllvm:-enable-ra-report -mllvm:-intel-opt-report=high -mllvm:-intel-ra-spillreport=high -mllvm:-inline-report=0xf859 -mllvm:-intel-opt-report-file=ipo_out.optrpt -mllvm:-loopopt=1 %t.obj
; RUN: FileCheck %s < ipo_out.optrpt
; RUN: rm %t.obj ipo_out.optrpt mymain.exe

; CMPLRLLVM-50071: Check that when LLD_IN_TEST=0, lld-link will produce a
; non-empty .optrpt file with the expected contents 

; CHECK: Global optimization report for : main
; CHECK: Begin Inlining Report
; CHECK: Option Values:
; CHECK: COMPILE FUNC: main
; CHECK: End Inlining Report
; CHECK: Register allocation report for: main
; CHECK: FUNCTION BEGIN
; CHECK: FUNCTION END

; ModuleID = 'mymain.obj'
source_filename = "mymain.c"
target datalayout = "e-m:w-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-windows-msvc19.29.30146"

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(none) uwtable
define dso_local i32 @main() local_unnamed_addr #0 !dbg !16 {
entry:
  ret i32 6, !dbg !19
}

attributes #0 = { mustprogress nofree norecurse nosync nounwind willreturn memory(none) uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }

!llvm.dbg.cu = !{!0}
!llvm.linker.options = !{!2, !3, !4, !5, !6, !7}
!llvm.module.flags = !{!8, !9, !10, !11, !12, !13, !14}
!llvm.ident = !{!15}

!0 = distinct !DICompileUnit(language: DW_LANG_C11, file: !1, producer: "clang based Intel(R) oneAPI DPC++/C++ Compiler 2024.0.0 (2024.x.0.YYYYMMDD)", isOptimized: true, flags: " --driver-mode=cl --intel --icx -c -flto=full -qopt-report=3 mymain.c -fbuiltin -O2 -ffunction-sections -fveclib=SVML -Wno-c++11-narrowing", runtimeVersion: 0, emissionKind: LineTablesOnly, splitDebugInlining: false, nameTableKind: None)
!1 = !DIFile(filename: "mymain.c", directory: "q:\\cq\\50071dd\\icx-cl_demo\\icx-cl_demo")
!2 = !{!"/DEFAULTLIB:libcmt.lib"}
!3 = !{!"/DEFAULTLIB:libircmt.lib"}
!4 = !{!"/DEFAULTLIB:svml_dispmt.lib"}
!5 = !{!"/DEFAULTLIB:libdecimal.lib"}
!6 = !{!"/DEFAULTLIB:libmmt.lib"}
!7 = !{!"/DEFAULTLIB:oldnames.lib"}
!8 = !{i32 2, !"Debug Info Version", i32 3}
!9 = !{i32 1, !"wchar_size", i32 2}
!10 = !{i32 8, !"PIC Level", i32 2}
!11 = !{i32 7, !"uwtable", i32 2}
!12 = !{i32 1, !"MaxTLSAlign", i32 65536}
!13 = !{i32 1, !"ThinLTO", i32 0}
!14 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!15 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2024.0.0 (2024.x.0.YYYYMMDD)"}
!16 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 1, type: !17, scopeLine: 1, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0)
!17 = !DISubroutineType(types: !18)
!18 = !{}
!19 = !DILocation(line: 2, column: 3, scope: !16)

