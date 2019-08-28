; RUN: opt < %s -inline -dtrans-inline-heuristics -inline-report=7 -S 2>&1 | FileCheck %s

; Check that with the old pass manager, that:
;   (1) the first call to foo() is inlined because it is to a linkonce ODR
;       function, even though it has a profile count of 0.
;   (2) the second call to foo() is inlined, because it is hot
;   (3) the call to goo() is not inlined, because it is NOT a linkonce ODR
;       function and it has a profile count of 0.

; CHECK: DEAD STATIC FUNC: foo
; CHECK: COMPILE FUNC: goo
; CHECK: COMPILE FUNC: main
; CHECK: INLINE: foo{{.*}}<<Callee is single basic block>>
; CHECK: INLINE: foo{{.*}}<<Callsite has hot profile>>
; CHECK: goo{{.*}}Callsite has cold profile
; CHECK: define{{.*}}@main
; CHECK-NOT: call{{.*}}@foo
; CHECK: call{{.*}}@goo

@myglobal = dso_local local_unnamed_addr global i32 0, align 4

; Function Attrs: nounwind uwtable
define dso_local i32 @main() local_unnamed_addr #0 !prof !29 {
entry:
  %0 = load i32, i32* @myglobal, align 4, !tbaa !30
  %cmp = icmp eq i32 %0, 0
  br i1 %cmp, label %for.cond, label %if.then, !prof !34

if.then:                                          ; preds = %entry
  call fastcc void @foo(), !intel-profx !35
  br label %cleanup

for.cond:                                         ; preds = %entry, %for.body
  %i.0 = phi i32 [ %inc, %for.body ], [ 0, %entry ]
  %s.0 = phi i32 [ %add, %for.body ], [ 0, %entry ]
  %cmp1 = icmp ult i32 %i.0, 10000
  br i1 %cmp1, label %for.body, label %cleanup, !prof !36

for.body:                                         ; preds = %for.cond
  call fastcc void @foo(), !intel-profx !37
  %add = add i32 %s.0, 5
  %inc = add i32 %i.0, 1
  br label %for.cond

cleanup:                                          ; preds = %for.cond, %if.then
  %retval.0 = phi i32 [ 5, %if.then ], [ %s.0, %for.cond ]
  call fastcc void @goo(), !intel-profx !35
  ret i32 %retval.0
}

; Function Attrs: inlinehint nounwind uwtable
define linkonce_odr fastcc void @foo() unnamed_addr #1 !prof !38 !PGOFuncName !39 {
entry:
  ret void
}


; Function Attrs: inlinehint nounwind uwtable
define internal fastcc void @goo() unnamed_addr #1 !prof !38 !PGOFuncName !39 {
entry:
  ret void
}

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { inlinehint nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!28}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 1, !"ProfileSummary", !2}
!2 = !{!3, !4, !5, !6, !7, !8, !9, !10}
!3 = !{!"ProfileFormat", !"InstrProf"}
!4 = !{!"TotalCount", i64 20001}
!5 = !{!"MaxCount", i64 10000}
!6 = !{!"MaxInternalCount", i64 1}
!7 = !{!"MaxFunctionCount", i64 10000}
!8 = !{!"NumCounts", i64 4}
!9 = !{!"NumFunctions", i64 2}
!10 = !{!"DetailedSummary", !11}
!11 = !{!12, !13, !14, !15, !16, !17, !18, !19, !20, !21, !22, !23, !24, !25, !26, !27}
!12 = !{i32 10000, i64 10000, i32 2}
!13 = !{i32 100000, i64 10000, i32 2}
!14 = !{i32 200000, i64 10000, i32 2}
!15 = !{i32 300000, i64 10000, i32 2}
!16 = !{i32 400000, i64 10000, i32 2}
!17 = !{i32 500000, i64 10000, i32 2}
!18 = !{i32 600000, i64 10000, i32 2}
!19 = !{i32 700000, i64 10000, i32 2}
!20 = !{i32 800000, i64 10000, i32 2}
!21 = !{i32 900000, i64 10000, i32 2}
!22 = !{i32 950000, i64 10000, i32 2}
!23 = !{i32 990000, i64 10000, i32 2}
!24 = !{i32 999000, i64 10000, i32 2}
!25 = !{i32 999900, i64 10000, i32 2}
!26 = !{i32 999990, i64 10000, i32 2}
!27 = !{i32 999999, i64 10000, i32 2}
!28 = !{!"icx (ICX) dev.8.x.0"}
!29 = !{!"function_entry_count", i64 1}
!30 = !{!31, !31, i64 0}
!31 = !{!"int", !32, i64 0}
!32 = !{!"omnipotent char", !33, i64 0}
!33 = !{!"Simple C/C++ TBAA"}
!34 = !{!"branch_weights", i32 1, i32 0}
!35 = !{!"intel_profx", i64 0}
!36 = !{!"branch_weights", i32 10000, i32 1}
!37 = !{!"intel_profx", i64 10000}
!38 = !{!"function_entry_count", i64 10000}
!39 = !{!"sm.c:foo"}
