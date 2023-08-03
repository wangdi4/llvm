; RUN: opt -passes="hir-ssa-deconstruction,hir-unroll-and-jam,hir-cg" -aa-pipeline="basic-aa" -S  <%s 2>&1 | FileCheck %s

; Verify branch_weights after U & J.

; Before Unroll and Jam
;
;   BEGIN REGION { }
;         + DO i1 = 0, sext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 10> <unroll and jam = 2>
;         |   + DO i2 = 0, sext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 10>
;         |   |   (@a)[0][i1][i2] = i2 + 3;
;         |   + END LOOP
;         + END LOOP
;   END REGION

; After Unroll and Jam
;
;   BEGIN REGION { modified }
;         %tgu = (sext.i32.i64(%N))/u2;
;
;         + DO i1 = 0, %tgu + -1, 1   <DO_LOOP>  <MAX_TC_EST = 5> <nounroll and jam>
;         |   + DO i2 = 0, sext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 10>
;         |   |   (@a)[0][2 * i1][i2] = i2 + 3;
;         |   |   (@a)[0][2 * i1 + 1][i2] = i2 + 3;
;         |   + END LOOP
;         + END LOOP
;
;
;         // Remainder loop of the outer loop
;         + DO i1 = 2 * %tgu, sext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 1> <nounroll> <nounroll and jam> <max_trip_count = 1>
;         |   + DO i2 = 0, sext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 10>
;         |   |   (@a)[0][i1][i2] = i2 + 3;
;         |   + END LOOP
;         + END LOOP
;   END REGION

;CHECK: region.0:
;CHECK: br i1
;CHECK-SAME: !prof ![[PROF_OUTER_UROLLED:[0-9]+]]
;CHECK: br i1
;CHECK-SAME: !prof ![[PROF_INNER_JAMMED:[0-9]+]]
;CHECK: br i1
;CHECK-SAME: !prof ![[PROF_OUTER_UROLLED]]
;CHECK: br i1
;CHECK-SAME: !prof ![[PROF_OUTER_REMAINDER:[0-9]+]]
;CHECK: br i1
;CHECK-SAME: !prof ![[PROF_INNER_JAMMED]]
;CHECK: br i1
;CHECK-SAME: !prof ![[PROF_OUTER_REMAINDER]]

;CHECK-DAG: ![[PROF_OUTER_UROLLED]] = !{!"branch_weights", i32 4, i32 1}
;CHECK-DAG: ![[PROF_INNER_JAMMED]] = !{!"branch_weights", i32 81, i32 9}
;CHECK-DAG: ![[PROF_OUTER_REMAINDER]] = !{!"branch_weights", i32 1, i32 1}

; The branch weights of outer main and remainder loop were obtained,
; respectively, by dividing and taking remainder of the original branch weights
; using the unroll factor.

; Note that the branch weights of inner loop were left the same. The trip
; count of inner loops does not change with unroll & jam so this makes sense.


; ModuleID = 'branchweights-uandj.c'
source_filename = "branchweights-uandj.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@a = common dso_local local_unnamed_addr global [10 x [10 x i32]] zeroinitializer, align 16

; Function Attrs: inlinehint norecurse nounwind uwtable
define dso_local i32 @sub(i32 %N) local_unnamed_addr #0 !prof !29 {
entry:
  %cmp28 = icmp sgt i32 %N, 0
  br i1 %cmp28, label %for.cond1.preheader.lr.ph, label %for.end9, !prof !30

for.cond1.preheader.lr.ph:                        ; preds = %entry
  %wide.trip.count = sext i32 %N to i64
  br label %for.body3.lr.ph

for.body3.lr.ph:                                  ; preds = %for.cond1.preheader.lr.ph, %for.inc7
  %indvars.iv32 = phi i64 [ 0, %for.cond1.preheader.lr.ph ], [ %indvars.iv.next33, %for.inc7 ]
  br label %for.body3

for.body3:                                        ; preds = %for.body3, %for.body3.lr.ph
  %indvars.iv = phi i64 [ 0, %for.body3.lr.ph ], [ %indvars.iv.next, %for.body3 ]
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %arrayidx6 = getelementptr inbounds [10 x [10 x i32]], ptr @a, i64 0, i64 %indvars.iv32, i64 %indvars.iv, !intel-tbaa !31
  %0 = trunc i64 %indvars.iv to i32
  %1 = add i32 %0, 3
  store i32 %1, ptr %arrayidx6, align 4, !tbaa !31
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.inc7, label %for.body3, !prof !37

for.inc7:                                         ; preds = %for.body3
  %indvars.iv.next33 = add nuw nsw i64 %indvars.iv32, 1
  %exitcond35 = icmp eq i64 %indvars.iv.next33, %wide.trip.count
  br i1 %exitcond35, label %for.end9.loopexit, label %for.body3.lr.ph, !prof !38, !llvm.loop !39

for.end9.loopexit:                                ; preds = %for.inc7
  br label %for.end9

for.end9:                                         ; preds = %for.end9.loopexit, %entry
  %sub = add nsw i32 %N, -1
  %idxprom10 = sext i32 %sub to i64
  %arrayidx14 = getelementptr inbounds [10 x [10 x i32]], ptr @a, i64 0, i64 %idxprom10, i64 %idxprom10, !intel-tbaa !31
  %2 = load i32, ptr %arrayidx14, align 4, !tbaa !31
  ret i32 %2
}

; Function Attrs: inlinehint norecurse nounwind uwtable
define dso_local i32 @main() local_unnamed_addr #0 !prof !29 {
entry:
  %call = tail call i32 @sub(i32 9) #1, !intel-profx !41
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
!4 = !{!"TotalCount", i64 92}
!5 = !{!"MaxCount", i64 81}
!6 = !{!"MaxInternalCount", i64 9}
!7 = !{!"MaxFunctionCount", i64 81}
!8 = !{!"NumCounts", i64 4}
!9 = !{!"NumFunctions", i64 2}
!10 = !{!"DetailedSummary", !11}
!11 = !{!12, !13, !14, !15, !16, !17, !18, !19, !20, !21, !22, !23, !24, !25, !26, !27}
!12 = !{i32 10000, i64 0, i32 0}
!13 = !{i32 100000, i64 81, i32 1}
!14 = !{i32 200000, i64 81, i32 1}
!15 = !{i32 300000, i64 81, i32 1}
!16 = !{i32 400000, i64 81, i32 1}
!17 = !{i32 500000, i64 81, i32 1}
!18 = !{i32 600000, i64 81, i32 1}
!19 = !{i32 700000, i64 81, i32 1}
!20 = !{i32 800000, i64 81, i32 1}
!21 = !{i32 900000, i64 9, i32 2}
!22 = !{i32 950000, i64 9, i32 2}
!23 = !{i32 990000, i64 1, i32 4}
!24 = !{i32 999000, i64 1, i32 4}
!25 = !{i32 999900, i64 1, i32 4}
!26 = !{i32 999990, i64 1, i32 4}
!27 = !{i32 999999, i64 1, i32 4}
!28 = !{!"icx (ICX) dev.8.x.0"}
!29 = !{!"function_entry_count", i64 1}
!30 = !{!"branch_weights", i32 9, i32 1}
!31 = !{!32, !34, i64 0}
!32 = !{!"array@_ZTSA10_A10_i", !33, i64 0}
!33 = !{!"array@_ZTSA10_i", !34, i64 0}
!34 = !{!"int", !35, i64 0}
!35 = !{!"omnipotent char", !36, i64 0}
!36 = !{!"Simple C/C++ TBAA"}
!37 = !{!"branch_weights", i32 9, i32 81}
!38 = !{!"branch_weights", i32 1, i32 9}
!39 = distinct !{!39, !40}
!40 = !{!"llvm.loop.unroll_and_jam.count", i32 2}
!41 = !{!"intel_profx", i64 1}
