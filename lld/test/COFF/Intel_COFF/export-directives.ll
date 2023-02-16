; REQUIRES: x86
; UNSUPPORTED: !system-windows

; RUN: llvm-as %s -o %t.bc
; RUN: lld-link -dll -out:%t.dll %t.bc -noentry
; RUN: llvm-readobj --coff-exports %t.dll | grep Name: | FileCheck %s

; Tests that exports processed via 'parseDirectives' in the COFF
; linker driver are parsed correctly

; CHECK: Name: bar
; CHECK: Name: doStuff
; CHECK: Name: doStuffAlias
; CHECK: Name: foo

; ModuleID = 'llvm-link'
source_filename = "llvm-link"
target datalayout = "e-m:w-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-windows-msvc19.29.30133"

; Function Attrs: mustprogress nofree norecurse nosync nounwind readnone willreturn uwtable
define dso_local dllexport i32 @foo() local_unnamed_addr #0 {
entry:
  ret i32 32
}

; Function Attrs: mustprogress nofree norecurse nosync nounwind readnone willreturn uwtable
define dso_local dllexport i32 @bar() local_unnamed_addr #0 {
entry:
  ret i32 18
}

; Function Attrs: mustprogress nofree norecurse nosync nounwind readnone willreturn uwtable
define dso_local dllexport zeroext i1 @doStuff() local_unnamed_addr #0 {
entry:
  ret i1 true
}

attributes #0 = { mustprogress nofree norecurse nosync nounwind readnone willreturn uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }

!llvm.linker.options = !{!0, !1, !2, !3, !4, !5, !0, !5, !6, !7}
!llvm.ident = !{!8}
!llvm.module.flags = !{!9, !10, !11, !12, !13}

!0 = !{!"/DEFAULTLIB:libcmt.lib"}
!1 = !{!"/DEFAULTLIB:libircmt.lib"}
!2 = !{!"/DEFAULTLIB:svml_dispmt.lib"}
!3 = !{!"/DEFAULTLIB:libdecimal.lib"}
!4 = !{!"/DEFAULTLIB:libmmt.lib"}
!5 = !{!"/DEFAULTLIB:oldnames.lib"}
!6 = !{!"/FAILIFMISMATCH:\22_CRT_STDIO_ISO_WIDE_SPECIFIERS=0\22"}
!7 = !{!"/EXPORT:doStuffAlias=exporttest.doStuff"}
!8 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2023.1.0 (2023.x.0.YYYYMMDD)"}
!9 = !{i32 1, !"wchar_size", i32 2}
!10 = !{i32 8, !"PIC Level", i32 2}
!11 = !{i32 7, !"uwtable", i32 2}
!12 = !{i32 1, !"ThinLTO", i32 0}
!13 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
