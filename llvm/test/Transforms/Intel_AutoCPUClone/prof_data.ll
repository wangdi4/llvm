; RUN: opt -passes=auto-cpu-clone < %s -S | FileCheck %s

; This test checks that functions, that contain hot code per available profile data,
; are multi-versioned.


; CHECK: @foo = dso_local ifunc i32 (), ptr @foo.resolver
; CHECK: @foo.A()
; CHECK: @foo.V()
; CHECK: @foo.resolver()


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: inlinehint mustprogress nofree norecurse nosync nounwind willreturn memory(none) uwtable
define dso_local i32 @foo() local_unnamed_addr #0 !prof !32 !llvm.auto.cpu.dispatch !33 {
entry:
  ret i32 0
}

attributes #0 = { inlinehint mustprogress nofree norecurse nosync nounwind willreturn memory(none) uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }

!llvm.module.flags = !{!0, !1, !2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 2}
!2 = !{i32 1, !"ProfileSummary", !3}
!3 = !{!4, !5, !6, !7, !8, !9, !10, !11, !12, !13}
!4 = !{!"ProfileFormat", !"InstrProf"}
!5 = !{!"TotalCount", i64 10000}
!6 = !{!"MaxCount", i64 10000}
!7 = !{!"MaxInternalCount", i64 0}
!8 = !{!"MaxFunctionCount", i64 10000}
!9 = !{!"NumCounts", i64 1}
!10 = !{!"NumFunctions", i64 1}
!11 = !{!"IsPartialProfile", i64 0}
!12 = !{!"PartialProfileRatio", double 0.000000e+00}
!13 = !{!"DetailedSummary", !14}
!14 = !{!15, !16, !17, !18, !19, !20, !21, !22, !23, !24, !25, !26, !27, !28, !29, !30}
!15 = !{i32 10000, i64 10000, i32 1}
!16 = !{i32 100000, i64 10000, i32 1}
!17 = !{i32 200000, i64 10000, i32 1}
!18 = !{i32 300000, i64 10000, i32 1}
!19 = !{i32 400000, i64 10000, i32 1}
!20 = !{i32 500000, i64 10000, i32 1}
!21 = !{i32 600000, i64 10000, i32 1}
!22 = !{i32 700000, i64 10000, i32 1}
!23 = !{i32 800000, i64 10000, i32 1}
!24 = !{i32 900000, i64 10000, i32 1}
!25 = !{i32 950000, i64 10000, i32 1}
!26 = !{i32 990000, i64 10000, i32 1}
!27 = !{i32 999000, i64 10000, i32 1}
!28 = !{i32 999900, i64 10000, i32 1}
!29 = !{i32 999990, i64 10000, i32 1}
!30 = !{i32 999999, i64 10000, i32 1}
!32 = !{!"function_entry_count", i64 10000}
!33 = !{!"core-avx2"}

