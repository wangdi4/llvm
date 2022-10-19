; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced
; RUN: opt < %s -inline -dtrans-inline-heuristics -intel-libirc-allowed -S 2>&1 | FileCheck %s
; RUN: opt < %s -passes=inline -dtrans-inline-heuristics -intel-libirc-allowed -S 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Test that intel_profx counts on calls are propagated correctly after
; inlining.

; CHECK: define dso_local i32 @main
; CHECK: call fastcc i32 @bar{{.*}}!intel-profx [[IPX1:![0-9]+]]
; CHECK: call fastcc i32 @bar{{.*}}!intel-profx [[IPX2:![0-9]+]]
; CHECK: call fastcc i32 @bar{{.*}}!intel-profx [[IPX3:![0-9]+]]
; CHECK: call fastcc i32 @bar{{.*}}!intel-profx [[IPX4:![0-9]+]]
; CHECK: call fastcc i32 @foo{{.*}}!intel-profx [[IPX1]]
; CHECK: define internal fastcc i32 @foo
; CHECK: call fastcc i32 @bar{{.*}}!intel-profx [[IPX1]]
; CHECK: call fastcc i32 @bar{{.*}}!intel-profx [[IPX2]]
; CHECK: define internal fastcc i32 @bar
; CHECK: [[IPX1]] = !{!"intel_profx", i64 1}
; CHECK: [[IPX2]] = !{!"intel_profx", i64 5000}
; CHECK: [[IPX3]] = !{!"intel_profx", i64 9999}
; CHECK: [[IPX4]] = !{!"intel_profx", i64 49995000}

define dso_local i32 @main() local_unnamed_addr !prof !29 {
entry:
  %call = call fastcc i32 @foo(), !intel-profx !30
  br label %for.cond

for.cond:                                         ; preds = %for.body, %entry
  %i.0 = phi i32 [ 1, %entry ], [ %inc, %for.body ]
  %s.0 = phi i32 [ %call, %entry ], [ %add, %for.body ]
  %cmp = icmp ult i32 %i.0, 10000
  br i1 %cmp, label %for.body, label %for.end, !prof !31

for.body:                                         ; preds = %for.cond
  %call1 = call fastcc i32 @foo(), !intel-profx !32
  %add = add nsw i32 %s.0, %call1
  %inc = add nuw nsw i32 %i.0, 1
  br label %for.cond

for.end:                                          ; preds = %for.cond
  %call2 = call fastcc i32 @foo() #0, !intel-profx !30
  %add3 = add nsw i32 %s.0, %call2
  ret i32 %add3
}

; Function Attrs: nounwind uwtable
define internal fastcc i32 @foo() unnamed_addr #1 !prof !33 !PGOFuncName !34 {
entry:
  %call = call fastcc i32 @bar(i32 0), !intel-profx !35
  br label %for.cond

for.cond:                                         ; preds = %for.body, %entry
  %j.0 = phi i32 [ 0, %entry ], [ %inc, %for.body ]
  %t.0 = phi i32 [ %call, %entry ], [ %add2, %for.body ]
  %cmp = icmp ult i32 %j.0, 5000
  br i1 %cmp, label %for.body, label %for.end, !prof !36

for.body:                                         ; preds = %for.cond
  %mul = mul nsw i32 %j.0, %j.0
  %sub = sub nsw i32 %mul, %j.0
  %call1 = call fastcc i32 @bar(i32 %j.0), !intel-profx !37
  %add = add nsw i32 %sub, %call1
  %add2 = add nsw i32 %t.0, %add
  %inc = add nuw nsw i32 %j.0, 1
  br label %for.cond

for.end:                                          ; preds = %for.cond
  ret i32 %t.0
}

define internal fastcc i32 @bar(i32 %x) unnamed_addr #0 !prof !38 !PGOFuncName !39 {
entry:
  ret i32 %x
}

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!28}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 1, !"ProfileSummary", !2}
!2 = !{!3, !4, !5, !6, !7, !8, !9, !10}
!3 = !{!"ProfileFormat", !"InstrProf"}
!4 = !{!"TotalCount", i64 100040002}
!5 = !{!"MaxCount", i64 50015001}
!6 = !{!"MaxInternalCount", i64 10001}
!7 = !{!"MaxFunctionCount", i64 50015001}
!8 = !{!"NumCounts", i64 5}
!9 = !{!"NumFunctions", i64 3}
!10 = !{!"DetailedSummary", !11}
!11 = !{!12, !13, !14, !15, !16, !17, !18, !19, !20, !21, !22, !23, !24, !25, !26, !27}
!12 = !{i32 10000, i64 50015001, i32 1}
!13 = !{i32 100000, i64 50015001, i32 1}
!14 = !{i32 200000, i64 50015001, i32 1}
!15 = !{i32 300000, i64 50015001, i32 1}
!16 = !{i32 400000, i64 50015001, i32 1}
!17 = !{i32 500000, i64 50005000, i32 2}
!18 = !{i32 600000, i64 50005000, i32 2}
!19 = !{i32 700000, i64 50005000, i32 2}
!20 = !{i32 800000, i64 50005000, i32 2}
!21 = !{i32 900000, i64 50005000, i32 2}
!22 = !{i32 950000, i64 50005000, i32 2}
!23 = !{i32 990000, i64 50005000, i32 2}
!24 = !{i32 999000, i64 50005000, i32 2}
!25 = !{i32 999900, i64 10001, i32 3}
!26 = !{i32 999990, i64 9999, i32 4}
!27 = !{i32 999999, i64 9999, i32 4}
!28 = !{!"icx (ICX) dev.8.x.0"}
!29 = !{!"function_entry_count", i64 1}
!30 = !{!"intel_profx", i64 1}
!31 = !{!"branch_weights", i32 9999, i32 1}
!32 = !{!"intel_profx", i64 9999}
!33 = !{!"function_entry_count", i64 10001}
!34 = !{!"sm.c:foo"}
!35 = !{!"intel_profx", i64 10001}
!36 = !{!"branch_weights", i32 50005000, i32 10001}
!37 = !{!"intel_profx", i64 50005000}
!38 = !{!"function_entry_count", i64 50015001}
!39 = !{!"sm.c:bar"}

attributes #0 = { noinline }
; end INTEL_FEATURE_SW_ADVANCED
