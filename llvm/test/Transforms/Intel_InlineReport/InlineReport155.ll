; RUN: opt -passes='module(pgo-icall-prom),print<inline-report>' -disable-output -inline-report=0xf847 < %s 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-CL
; RUN: opt -passes='inlinereportsetup,module(pgo-icall-prom),inlinereportemitter' -disable-output -inline-report=0xf8c6 < %s 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-MD

; Check that indirect call is specialized to call @peak directly using PGO

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; CHECK-MD-LABEL: COMPILE FUNC: base
; CHECK-MD-LABEL: COMPILE FUNC: peak
; CHECK-LABEL: COMPILE FUNC: main
; CHECK: INDIRECT: {{.*}}Call site is indirect
; CHECK: (PGO) peak {{.*}}Newly created callsite
; CHECK-CL-LABEL: COMPILE FUNC: peak
; CHECK-CL-LABEL: COMPILE FUNC: base

@n = dso_local local_unnamed_addr global i32 10000, align 4
@fp = dso_local local_unnamed_addr global ptr @base, align 8

; Function Attrs: noinline nounwind uwtable
define dso_local i32 @base(i32 noundef %i) #0 !prof !34 {
entry:
  %sub = sub i32 0, %i
  ret i32 %sub
}

; Function Attrs: inlinehint nounwind uwtable
define dso_local i32 @peak(i32 noundef %i) #1 !prof !35 {
entry:
  %mul = mul i32 %i, %i
  %sub = add i32 %mul, -7
  ret i32 %sub
}

; Function Attrs: nounwind uwtable
define dso_local i32 @main() local_unnamed_addr #2 !prof !36 {
entry:
  %0 = load i32, ptr @n, align 4, !tbaa !37
  br label %for.cond

for.cond:                                         ; preds = %if.end, %entry
  %i.0 = phi i32 [ 0, %entry ], [ %inc, %if.end ]
  %s.0 = phi i32 [ 0, %entry ], [ %add, %if.end ]
  %cmp = icmp ult i32 %i.0, %0
  br i1 %cmp, label %for.body, label %for.cond.cleanup, !prof !41

for.cond.cleanup:                                 ; preds = %for.cond
  ret i32 %s.0

for.body:                                         ; preds = %for.cond
  %cmp1 = icmp eq i32 %i.0, 10
  br i1 %cmp1, label %if.then, label %if.end, !prof !42

if.then:                                          ; preds = %for.body
  store ptr @peak, ptr @fp, align 8, !tbaa !43
  br label %if.end

if.end:                                           ; preds = %if.then, %for.body
  %1 = load ptr, ptr @fp, align 8, !tbaa !43
  %call = call i32 %1(i32 noundef %i.0) #3, !prof !45, !intel-profx !46
  %add = add nsw i32 %s.0, %call
  %inc = add i32 %i.0, 1
  br label %for.cond, !llvm.loop !47
}

attributes #0 = { noinline nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { inlinehint nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #2 = { nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #3 = { nounwind }

!llvm.module.flags = !{!0, !1, !2, !3, !4}
!llvm.ident = !{!33}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 2}
!2 = !{i32 1, !"ThinLTO", i32 0}
!3 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!4 = !{i32 1, !"ProfileSummary", !5}
!5 = !{!6, !7, !8, !9, !10, !11, !12, !13, !14, !15}
!6 = !{!"ProfileFormat", !"InstrProf"}
!7 = !{!"TotalCount", i64 20002}
!8 = !{!"MaxCount", i64 10000}
!9 = !{!"MaxInternalCount", i64 1}
!10 = !{!"MaxFunctionCount", i64 10000}
!11 = !{!"NumCounts", i64 5}
!12 = !{!"NumFunctions", i64 3}
!13 = !{!"IsPartialProfile", i64 0}
!14 = !{!"PartialProfileRatio", double 0.000000e+00}
!15 = !{!"DetailedSummary", !16}
!16 = !{!17, !18, !19, !20, !21, !22, !23, !24, !25, !26, !27, !28, !29, !30, !31, !32}
!17 = !{i32 10000, i64 10000, i32 1}
!18 = !{i32 100000, i64 10000, i32 1}
!19 = !{i32 200000, i64 10000, i32 1}
!20 = !{i32 300000, i64 10000, i32 1}
!21 = !{i32 400000, i64 10000, i32 1}
!22 = !{i32 500000, i64 9990, i32 2}
!23 = !{i32 600000, i64 9990, i32 2}
!24 = !{i32 700000, i64 9990, i32 2}
!25 = !{i32 800000, i64 9990, i32 2}
!26 = !{i32 900000, i64 9990, i32 2}
!27 = !{i32 950000, i64 9990, i32 2}
!28 = !{i32 990000, i64 9990, i32 2}
!29 = !{i32 999000, i64 9990, i32 2}
!30 = !{i32 999900, i64 10, i32 3}
!31 = !{i32 999990, i64 1, i32 5}
!32 = !{i32 999999, i64 1, i32 5}
!33 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2023.2.0 (2023.x.0.YYYYMMDD)"}
!34 = !{!"function_entry_count", i64 10}
!35 = !{!"function_entry_count", i64 9990}
!36 = !{!"function_entry_count", i64 1}
!37 = !{!38, !38, i64 0}
!38 = !{!"int", !39, i64 0}
!39 = !{!"omnipotent char", !40, i64 0}
!40 = !{!"Simple C/C++ TBAA"}
!41 = !{!"branch_weights", i32 10000, i32 1}
!42 = !{!"branch_weights", i32 1, i32 9999}
!43 = !{!44, !44, i64 0}
!44 = !{!"pointer@_ZTSPFijE", !39, i64 0}
!45 = !{!"VP", i32 0, i64 10000, i64 4518749848190263604, i64 9990, i64 1084297779302774361, i64 10}
!46 = !{!"intel_profx", i64 10000}
!47 = distinct !{!47, !48}
!48 = !{!"llvm.loop.mustprogress"}

