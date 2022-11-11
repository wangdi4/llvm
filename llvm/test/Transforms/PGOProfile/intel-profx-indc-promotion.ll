; RUN: opt < %s -passes=pgo-icall-prom -S | FileCheck %s

; Test that intel_profx metadata execution counts are propagated from
; indirect calls to direct calls during PGO indirect call promotion.

; CHECK: define dso_local i32 @main()
; CHECK: call i32 @func0{{.*}}!intel-profx [[PROFX1:![0-9]+]]
; CHECK: call i32 @func1{{.*}}!intel-profx [[PROFX2:![0-9]+]]
; CHECK: call i32 @func2{{.*}}!intel-profx [[PROFX2]]
; CHECK: call i32 %fp.1{{.*}}!intel-profx [[PROFX3:![0-9]+]]
; CHECK: [[PROFX1]] = !{!"intel_profx", i64 3334}
; CHECK: [[PROFX2]] = !{!"intel_profx", i64 3333}
; CHECK: [[PROFX3]] = !{!"intel_profx", i64 0}

; ModuleID = 'sm.c'
source_filename = "sm.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define dso_local i32 @main() local_unnamed_addr #0 !prof !29 {
entry:
  br label %sw.bb

for.body:                                         ; preds = %sw.epilog
  %rem = urem i32 %inc, 3
  switch i32 %rem, label %sw.epilog [
    i32 0, label %sw.bb
    i32 1, label %sw.bb1
    i32 2, label %sw.bb2
  ], !prof !30

sw.bb:                                            ; preds = %entry, %for.body
  %s.019 = phi i32 [ 0, %entry ], [ %add, %for.body ]
  %i.015 = phi i32 [ 0, %entry ], [ %inc, %for.body ]
  br label %sw.epilog

sw.bb1:                                           ; preds = %for.body
  br label %sw.epilog

sw.bb2:                                           ; preds = %for.body
  br label %sw.epilog

sw.epilog:                                        ; preds = %for.body, %sw.bb2, %sw.bb1, %sw.bb
  %s.018 = phi i32 [ %add, %for.body ], [ %add, %sw.bb2 ], [ %add, %sw.bb1 ], [ %s.019, %sw.bb ]
  %i.016 = phi i32 [ %inc, %for.body ], [ %inc, %sw.bb2 ], [ %inc, %sw.bb1 ], [ %i.015, %sw.bb ]
  %fp.1 = phi i32 (i32)* [ %fp.1, %for.body ], [ @func2, %sw.bb2 ], [ @func1, %sw.bb1 ], [ @func0, %sw.bb ]
  %call = call i32 %fp.1(i32 %i.016) #2, !prof !31, !intel-profx !32
  %add = add nsw i32 %s.018, %call
  %inc = add nuw nsw i32 %i.016, 1
  %cmp = icmp ult i32 %inc, 10000
  br i1 %cmp, label %for.body, label %for.end, !prof !33

for.end:                                          ; preds = %sw.epilog
  ret i32 %add
}

; Function Attrs: inlinehint noinline nounwind uwtable
define internal i32 @func0(i32 %i) unnamed_addr #1 !prof !34 !PGOFuncName !35 {
entry:
  %mul = mul nsw i32 %i, %i
  ret i32 %mul
}

; Function Attrs: inlinehint noinline nounwind uwtable
define internal i32 @func1(i32 %i) unnamed_addr #1 !prof !36 !PGOFuncName !37 {
entry:
  %mul = mul nsw i32 %i, %i
  %sub = sub nsw i32 %mul, %i
  ret i32 %sub
}

; Function Attrs: inlinehint noinline nounwind uwtable
define internal i32 @func2(i32 %i) unnamed_addr #1 !prof !36 !PGOFuncName !38 {
entry:
  %mul = mul nsw i32 %i, %i
  %add = add nsw i32 %mul, %i
  ret i32 %add
}

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { inlinehint noinline nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!28}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 1, !"ProfileSummary", !2}
!2 = !{!3, !4, !5, !6, !7, !8, !9, !10}
!3 = !{!"ProfileFormat", !"InstrProf"}
!4 = !{!"TotalCount", i64 30000}
!5 = !{!"MaxCount", i64 9999}
!6 = !{!"MaxInternalCount", i64 3334}
!7 = !{!"MaxFunctionCount", i64 9999}
!8 = !{!"NumCounts", i64 8}
!9 = !{!"NumFunctions", i64 4}
!10 = !{!"DetailedSummary", !11}
!11 = !{!12, !13, !14, !15, !16, !17, !18, !19, !20, !21, !22, !23, !24, !25, !26, !27}
!12 = !{i32 10000, i64 9999, i32 1}
!13 = !{i32 100000, i64 9999, i32 1}
!14 = !{i32 200000, i64 9999, i32 1}
!15 = !{i32 300000, i64 9999, i32 1}
!16 = !{i32 400000, i64 3334, i32 3}
!17 = !{i32 500000, i64 3334, i32 3}
!18 = !{i32 600000, i64 3333, i32 7}
!19 = !{i32 700000, i64 3333, i32 7}
!20 = !{i32 800000, i64 3333, i32 7}
!21 = !{i32 900000, i64 3333, i32 7}
!22 = !{i32 950000, i64 3333, i32 7}
!23 = !{i32 990000, i64 3333, i32 7}
!24 = !{i32 999000, i64 3333, i32 7}
!25 = !{i32 999900, i64 3333, i32 7}
!26 = !{i32 999990, i64 3333, i32 7}
!27 = !{i32 999999, i64 3333, i32 7}
!28 = !{!"icx (ICX) dev.8.x.0"}
!29 = !{!"function_entry_count", i64 1}
!30 = !{!"branch_weights", i32 0, i32 3333, i32 3333, i32 3333}
!31 = !{!"VP", i32 0, i64 10000, i64 1378811987157390831, i64 3334, i64 -2489713108340730052, i64 3333, i64 4671414384325397476, i64 3333}
!32 = !{!"intel_profx", i64 10000}
!33 = !{!"branch_weights", i32 9999, i32 1}
!34 = !{!"function_entry_count", i64 3334}
!35 = !{!"sm.c:func0"}
!36 = !{!"function_entry_count", i64 3333}
!37 = !{!"sm.c:func1"}
!38 = !{!"sm.c:func2"}
