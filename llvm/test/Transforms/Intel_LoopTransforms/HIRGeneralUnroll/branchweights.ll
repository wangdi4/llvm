;RUN: opt -hir-ssa-deconstruction -hir-general-unroll -hir-cg -hir-general-unroll-disable-replace-by-first-iteration -S %s | FileCheck %s
;RUN: opt -passes="hir-ssa-deconstruction,hir-general-unroll,hir-cg" -hir-general-unroll-disable-replace-by-first-iteration -S %s | FileCheck %s

; Make sure HIR loopopt generall unroll preserves input profile-related metadata (i.e. "branch_weights")
; Original branch_weight 10(true weight), 1(false weight) changes into 5, 1 after unroll by two.
; Remainder loops branch_weight is kept low.

; *** IR Dump Before HIR General Unroll ***
;
;         BEGIN REGION { }
;               + DO i1 = 0, sext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 20> <unroll = 2>
;                |   %conv = sitofp.i32.double(i1 + 2);
;                |   (@a)[0][i1] = %conv;
;                + END LOOP
;          END REGION
;
;  *** IR Dump After HIR General Unroll ***
;
;          BEGIN REGION { modified }
;                %tgu = (sext.i32.i64(%N))/u2;
;
;                + DO i1 = 0, %tgu + -1, 1   <DO_LOOP>  <MAX_TC_EST = 10> <nounroll>
;                |   %conv = sitofp.i32.double(2 * i1 + 2);
;                |   (@a)[0][2 * i1] = %conv;
;                |   %conv = sitofp.i32.double(2 * i1 + 3);
;                |   (@a)[0][2 * i1 + 1] = %conv;
;                + END LOOP
;
;
;                + DO i1 = 2 * %tgu, sext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 1> < nounroll> <max_trip_count = 1>
;                |   %conv = sitofp.i32.double(i1 + 2);
;                |   (@a)[0][i1] = %conv;
;                + END LOOP
;         END REGION

; PROF_META_B and PROF_META_R stand for branch_weights metadata for main and remainder loops, respectively. Ztt and backedges for each loop.

;CHECK: region.0:
;CHECK: br i1
;CHECK-SAME: !prof ![[PROF_META_B:[0-9]+]]
;CHECK: loop.{{[0-9]+}}:
;CHECK: br i1
;CHECK-SAME: !prof ![[PROF_META_B]]
;CHECK: ifmerge.{{[0-9]+}}
;CHECK: br i1
;CHECK-SAME: !prof ![[PROF_META_R:[0-9]+]]
;CHECK: loop.{{[0-9]+}}:
;CHECK: br i1
;CHECK-SAME: !prof ![[PROF_META_R]]

;CHECK-DAG: ![[PROF_META_B]] = !{!"branch_weights", i32 5, i32 1}
;CHECK-DAG: ![[PROF_META_R]] = !{!"branch_weights", i32 1, i32 1}

; ModuleID = 'mm3-pragma.c'
source_filename = "mm3-pragma.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@a = common dso_local local_unnamed_addr global [20 x double] zeroinitializer, align 16
@b = common dso_local local_unnamed_addr global [20 x double] zeroinitializer, align 16

; Function Attrs: inlinehint norecurse nounwind uwtable
define dso_local i32 @sub(i32 %N) local_unnamed_addr #0 !prof !29 {
entry:
  %cmp8 = icmp sgt i32 %N, 0
  br i1 %cmp8, label %for.body.preheader, label %for.end, !prof !30

for.body.preheader:                               ; preds = %entry
  %wide.trip.count = sext i32 %N to i64
  br label %for.body

for.body:                                         ; preds = %for.body, %for.body.preheader
  %indvars.iv = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next, %for.body ]
  %0 = trunc i64 %indvars.iv to i32
  %1 = add i32 %0, 2
  %conv = sitofp i32 %1 to double
  %arrayidx = getelementptr inbounds [20 x double], [20 x double]* @a, i64 0, i64 %indvars.iv, !intel-tbaa !31
  store double %conv, double* %arrayidx, align 8, !tbaa !31
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.end.loopexit, label %for.body, !prof !36, !llvm.loop !37

for.end.loopexit:                                 ; preds = %for.body
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  %2 = load double, double* getelementptr inbounds ([20 x double], [20 x double]* @a, i64 0, i64 3), align 8, !tbaa !31
  %conv1 = fptosi double %2 to i32
  ret i32 %conv1
}

; Function Attrs: inlinehint norecurse nounwind uwtable
define dso_local i32 @main() local_unnamed_addr #0 !prof !29 {
entry:
  %call = tail call i32 @sub(i32 10) #1
  ret i32 0
}

attributes #0 = { inlinehint norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="corei7" "target-features"="+cx16,+cx8,+fxsr,+mmx,+popcnt,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { noinline }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!28}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 1, !"ProfileSummary", !2}
!2 = !{!3, !4, !5, !6, !7, !8, !9, !10}
!3 = !{!"ProfileFormat", !"InstrProf"}
!4 = !{!"TotalCount", i64 12}
!5 = !{!"MaxCount", i64 10}
!6 = !{!"MaxInternalCount", i64 1}
!7 = !{!"MaxFunctionCount", i64 10}
!8 = !{!"NumCounts", i64 3}
!9 = !{!"NumFunctions", i64 2}
!10 = !{!"DetailedSummary", !11}
!11 = !{!12, !13, !14, !15, !16, !17, !18, !19, !20, !21, !22, !23, !24, !25, !26, !27}
!12 = !{i32 10000, i64 0, i32 0}
!13 = !{i32 100000, i64 10, i32 1}
!14 = !{i32 200000, i64 10, i32 1}
!15 = !{i32 300000, i64 10, i32 1}
!16 = !{i32 400000, i64 10, i32 1}
!17 = !{i32 500000, i64 10, i32 1}
!18 = !{i32 600000, i64 10, i32 1}
!19 = !{i32 700000, i64 10, i32 1}
!20 = !{i32 800000, i64 10, i32 1}
!21 = !{i32 900000, i64 10, i32 1}
!22 = !{i32 950000, i64 1, i32 3}
!23 = !{i32 990000, i64 1, i32 3}
!24 = !{i32 999000, i64 1, i32 3}
!25 = !{i32 999900, i64 1, i32 3}
!26 = !{i32 999990, i64 1, i32 3}
!27 = !{i32 999999, i64 1, i32 3}
!28 = !{!"icx (ICX) dev.8.x.0"}
!29 = !{!"function_entry_count", i64 1}
!30 = !{!"branch_weights", i32 10, i32 1}
!31 = !{!32, !33, i64 0}
!32 = !{!"array@_ZTSA20_d", !33, i64 0}
!33 = !{!"double", !34, i64 0}
!34 = !{!"omnipotent char", !35, i64 0}
!35 = !{!"Simple C/C++ TBAA"}
!36 = !{!"branch_weights", i32 1, i32 10}
!37 = distinct !{!37, !38}
!38 = !{!"llvm.loop.unroll.count", i32 2}
