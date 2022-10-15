; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced
; RUN: opt < %s -enable-new-pm=0 -inline -dtrans-inline-heuristics -intel-libirc-allowed -inline-report=0xe807 -inline-prof-instr-hot-percentage=95 -inline-prof-instr-hot-count=2 -S 2>&1 | FileCheck %s
; RUN: opt < %s -passes='require<profile-summary>,inline' -dtrans-inline-heuristics -intel-libirc-allowed -inline-report=0xe807 -inline-prof-instr-hot-percentage=95 -inline-prof-instr-hot-count=2 -S 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Test that the two hottest calls get inlined because of hot profiles since
; inline-prof-instr-hot-count=2 and inline-prof-instr-hot-percentage is
; large enough not to exclude the two hottest callsites.

; CHECK: define dso_local i32 @main
; CHECK-NOT: call i32 @baz
; CHECK-NOT: call i32 @booz
; CHECK: COMPILE FUNC: main
; CHECK-NOT: INLINE: bar{{.*}}Callsite has hot profile
; CHECK: INLINE: baz{{.*}}Callsite has hot profile
; CHECK: INLINE: booz{{.*}}Callsite has hot profile

; Function Attrs: cold nounwind uwtable
define dso_local i32 @bar() local_unnamed_addr !prof !31 {
entry:
  ret i32 5
}

; Function Attrs: cold nounwind uwtable
define dso_local i32 @baz(i32 %x) local_unnamed_addr !prof !31 {
entry:
  %add = add nsw i32 %x, 10
  ret i32 %add
}

; Function Attrs: cold nounwind uwtable
define dso_local i32 @booz(i32 %x, i32 %y) local_unnamed_addr !prof !31 {
entry:
  %sub = sub nsw i32 %x, %y
  %add = add nsw i32 %sub, 2
  ret i32 %add
}

define dso_local i32 @main() local_unnamed_addr #1 !prof !32 {
entry:
  %call = call i32 @bar(), !intel-profx !33
  br label %for.cond

for.cond:                                         ; preds = %for.inc8, %entry
  %i.0 = phi i32 [ 0, %entry ], [ %inc9, %for.inc8 ]
  %s.0 = phi i32 [ 5, %entry ], [ %s.1, %for.inc8 ]
  %cmp = icmp ult i32 %i.0, 10
  br i1 %cmp, label %for.body, label %for.end10, !prof !34

for.body:                                         ; preds = %for.cond
  %call1 = call i32 @baz(i32 %i.0), !intel-profx !35
  %add2 = add nsw i32 %s.0, %call1
  br label %for.cond3

for.cond3:                                        ; preds = %for.body5, %for.body
  %j.0 = phi i32 [ 0, %for.body ], [ %inc, %for.body5 ]
  %s.1 = phi i32 [ %add2, %for.body ], [ %add7, %for.body5 ]
  %cmp4 = icmp ult i32 %j.0, 10
  br i1 %cmp4, label %for.body5, label %for.inc8, !prof !36

for.body5:                                        ; preds = %for.cond3
  %call6 = call i32 @booz(i32 %i.0, i32 %j.0), !intel-profx !37
  %add7 = add nsw i32 %s.1, %call6
  %inc = add nuw nsw i32 %j.0, 1
  br label %for.cond3

for.inc8:                                         ; preds = %for.cond3
  %inc9 = add nuw nsw i32 %i.0, 1
  br label %for.cond

for.end10:                                        ; preds = %for.cond
  ret i32 %s.0
}

!llvm.module.flags = !{!0, !1, !2, !3}
!llvm.ident = !{!30}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 1, !"ThinLTO", i32 0}
!2 = !{i32 1, !"EnableSplitLTOUnit", i32 0}
!3 = !{i32 1, !"ProfileSummary", !4}
!4 = !{!5, !6, !7, !8, !9, !10, !11, !12}
!5 = !{!"ProfileFormat", !"InstrProf"}
!6 = !{!"TotalCount", i64 111}
!7 = !{!"MaxCount", i64 100}
!8 = !{!"MaxInternalCount", i64 10}
!9 = !{!"MaxFunctionCount", i64 100}
!10 = !{!"NumCounts", i64 6}
!11 = !{!"NumFunctions", i64 4}
!12 = !{!"DetailedSummary", !13}
!13 = !{!14, !15, !16, !17, !18, !19, !20, !21, !22, !23, !24, !25, !26, !27, !28, !29}
!14 = !{i32 10000, i64 100, i32 1}
!15 = !{i32 100000, i64 100, i32 1}
!16 = !{i32 200000, i64 100, i32 1}
!17 = !{i32 300000, i64 100, i32 1}
!18 = !{i32 400000, i64 100, i32 1}
!19 = !{i32 500000, i64 100, i32 1}
!20 = !{i32 600000, i64 100, i32 1}
!21 = !{i32 700000, i64 100, i32 1}
!22 = !{i32 800000, i64 100, i32 1}
!23 = !{i32 900000, i64 100, i32 1}
!24 = !{i32 950000, i64 10, i32 2}
!25 = !{i32 990000, i64 10, i32 2}
!26 = !{i32 999000, i64 10, i32 2}
!27 = !{i32 999900, i64 10, i32 2}
!28 = !{i32 999990, i64 10, i32 2}
!29 = !{i32 999999, i64 10, i32 2}
!30 = !{!"icx (ICX) dev.8.x.0"}
!31 = !{!"function_entry_count", i64 0}
!32 = !{!"function_entry_count", i64 1}
!33 = !{!"intel_profx", i64 1}
!34 = !{!"branch_weights", i32 10, i32 1}
!35 = !{!"intel_profx", i64 10}
!36 = !{!"branch_weights", i32 100, i32 10}
!37 = !{!"intel_profx", i64 100}


; end INTEL_FEATURE_SW_ADVANCED
