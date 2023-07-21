; RUN: opt -passes='require<anders-aa>,indirectcallconv,inline' -intel-ind-call-force-andersen -disable-output -inline-report=0xf847 < %s -S 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-CL
; RUN: opt -passes='require<anders-aa>,inlinereportsetup,indirectcallconv,inline,inlinereportemitter' -intel-ind-call-force-andersen -disable-output -inline-report=0xf8c6 < %s -S 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-MD

; Check that the indirect call is specialized by global points-to and then
; deleted because no fallback case is needed. Then check that peak is inlined
; because it has a single basic block, but base is not inlined, because it
; has a noinline directive.

; CHECK-MD-LABEL: COMPILE FUNC: base
; CHECK-MD-LABEL: COMPILE FUNC: peak
; CHECK-LABEL: COMPILE FUNC: main
; CHECK-LABEL: INDIRECT: DELETE: {{.*}}Indirect call conversion
; CHECK: (GPT) base {{.*}}Callee has noinline attribute
; CHECK: INLINE: (GPT) peak ({{.*}}<={{.*}} {{.*}}Callee is single basic block
; CHECK-CL-LABEL: COMPILE FUNC: base
; CHECK-CL-LABEL: COMPILE FUNC: peak

source_filename = "ld-temp.o"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@fp = internal unnamed_addr global ptr @base, align 8

; Function Attrs: mustprogress nofree noinline norecurse nosync nounwind willreturn memory(none) uwtable
define internal i32 @base(i32 noundef %0) #0 !prof !35 {
  %2 = sub i32 0, %0
  ret i32 %2
}

; Function Attrs: inlinehint mustprogress nofree norecurse nosync nounwind willreturn memory(none) uwtable
define internal i32 @peak(i32 noundef %0) #1 !prof !36 {
  %2 = mul i32 %0, %0
  %3 = add i32 %2, -7
  ret i32 %3
}

; Function Attrs: norecurse nounwind uwtable
define dso_local i32 @main() local_unnamed_addr #2 !prof !37 {
  br label %2

1:                                                ; preds = %15
  ret i32 %17

2:                                                ; preds = %0, %15
  %3 = phi i32 [ %17, %15 ], [ 0, %0 ]
  %4 = phi i32 [ %18, %15 ], [ 0, %0 ]
  %5 = icmp eq i32 %4, 10
  br i1 %5, label %6, label %7, !prof !38

6:                                                ; preds = %2
  store ptr @peak, ptr @fp, align 8, !tbaa !39
  br label %10

7:                                                ; preds = %2
  %8 = load ptr, ptr @fp, align 8, !tbaa !39
  %9 = icmp eq ptr %8, @peak
  br i1 %9, label %10, label %13, !prof !43

10:                                               ; preds = %7, %6
  %11 = mul i32 %4, %4
  %12 = add i32 %11, -7
  br label %15

13:                                               ; preds = %7
  %14 = tail call i32 %8(i32 noundef %4) #3, !prof !44, !callees !45, !intel-profx !46
  br label %15

15:                                               ; preds = %13, %10
  %16 = phi i32 [ %14, %13 ], [ %12, %10 ]
  %17 = add nsw i32 %16, %3
  %18 = add nuw i32 %4, 1
  %19 = icmp eq i32 %18, 10000
  br i1 %19, label %1, label %2, !prof !47, !llvm.loop !48
}

attributes #0 = { mustprogress nofree noinline norecurse nosync nounwind willreturn memory(none) uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { inlinehint mustprogress nofree norecurse nosync nounwind willreturn memory(none) uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #2 = { norecurse nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #3 = { nounwind }

!llvm.ident = !{!0}
!llvm.module.flags = !{!1, !2, !3, !4, !5, !34}

!0 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2023.2.0 (2023.x.0.YYYYMMDD)"}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 7, !"uwtable", i32 2}
!3 = !{i32 1, !"ThinLTO", i32 0}
!4 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!5 = !{i32 1, !"ProfileSummary", !6}
!6 = !{!7, !8, !9, !10, !11, !12, !13, !14, !15, !16}
!7 = !{!"ProfileFormat", !"InstrProf"}
!8 = !{!"TotalCount", i64 20002}
!9 = !{!"MaxCount", i64 10000}
!10 = !{!"MaxInternalCount", i64 1}
!11 = !{!"MaxFunctionCount", i64 10000}
!12 = !{!"NumCounts", i64 5}
!13 = !{!"NumFunctions", i64 3}
!14 = !{!"IsPartialProfile", i64 0}
!15 = !{!"PartialProfileRatio", double 0.000000e+00}
!16 = !{!"DetailedSummary", !17}
!17 = !{!18, !19, !20, !21, !22, !23, !24, !25, !26, !27, !28, !29, !30, !31, !32, !33}
!18 = !{i32 10000, i64 10000, i32 1}
!19 = !{i32 100000, i64 10000, i32 1}
!20 = !{i32 200000, i64 10000, i32 1}
!21 = !{i32 300000, i64 10000, i32 1}
!22 = !{i32 400000, i64 10000, i32 1}
!23 = !{i32 500000, i64 9990, i32 2}
!24 = !{i32 600000, i64 9990, i32 2}
!25 = !{i32 700000, i64 9990, i32 2}
!26 = !{i32 800000, i64 9990, i32 2}
!27 = !{i32 900000, i64 9990, i32 2}
!28 = !{i32 950000, i64 9990, i32 2}
!29 = !{i32 990000, i64 9990, i32 2}
!30 = !{i32 999000, i64 9990, i32 2}
!31 = !{i32 999900, i64 10, i32 3}
!32 = !{i32 999990, i64 1, i32 5}
!33 = !{i32 999999, i64 1, i32 5}
!34 = !{i32 1, !"LTOPostLink", i32 1}
!35 = !{!"function_entry_count", i64 10}
!36 = !{!"function_entry_count", i64 0}
!37 = !{!"function_entry_count", i64 1}
!38 = !{!"branch_weights", i32 1, i32 9999}
!39 = !{!40, !40, i64 0}
!40 = !{!"pointer@_ZTSPFijE", !41, i64 0}
!41 = !{!"omnipotent char", !42, i64 0}
!42 = !{!"Simple C/C++ TBAA"}
!43 = !{!"branch_weights", i32 2145335923, i32 2147725}
!44 = !{!"VP", i32 0, i64 10, i64 1084297779302774361, i64 10}
!45 = !{ptr @base, ptr @peak}
!46 = !{!"intel_profx", i64 10}
!47 = !{!"branch_weights", i32 1, i32 10000}
!48 = distinct !{!48, !49}
!49 = !{!"llvm.loop.mustprogress"}
