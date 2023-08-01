; RUN: opt -passes="hir-ssa-deconstruction,hir-loop-collapse,hir-cg" -S %s | FileCheck %s

; Make sure HIR loop collapse pass correctly modifies profile-related metadata ("branch weights")
; The collapsed loop should use the backedge taken weight from the innermost loop and loopexit/non-taken weight from the outermost loop making it [1000, 1] in this case. The LLVM IR metadata is checked based on the corresponding loops.
; *** IR Dump Before HIR Loop Collapse ***
; Function: m

;          BEGIN REGION { }
;               + DO i1 = 0, 9, 1   <DO_LOOP>
;               |   + DO i2 = 0, 9, 1   <DO_LOOP>
;               |   |   + DO i3 = 0, 9, 1   <DO_LOOP>
;               |   |   |   %1 = (%A)[0][i1][i2][i3];
;               |   |   |   (%A)[0][i1][i2][i3] = %1 + 1;
;               |   |   + END LOOP
;               |   + END LOOP
;               + END LOOP
;          END REGION

; *** IR Dump After HIR Loop Collapse ***
; Function: m

;          BEGIN REGION { modified }
;               + DO i1 = 0, 999, 1   <DO_LOOP>
;               |   %1 = (%A)[0][0][0][i1];
;               |   (%A)[0][0][0][i1] = %1 + 1;
;               + END LOOP
;          END REGION

; PROF_META_LPX refer to old loops while and PROF_META_NEW is the new collapsed loop[s]


;CHECK: for.body{{[0-9]+}}:
;CHECK: br i1
;CHECK-SAME: !prof ![[PROF_META_LP3:[0-9]+]]
;CHECK: br i1
;CHECK-SAME: !prof ![[PROF_META_LP2:[0-9]+]]
;CHECK: br i1
;CHECK-SAME: !prof ![[PROF_META_LP1:[0-9]+]]

;CHECK: region.0:
;CHECK: loop.{{[0-9]+}}:
;CHECK: br i1
;CHECK-SAME: !prof ![[PROF_META_NEW:[0-9]+]]

;CHECK: ![[PROF_META_LP3]] = !{!"branch_weights", i32 [[LP3_EXIT:[0-9]+]], i32 [[LP3_BACKEDGE:[0-9]+]]}
;CHECK: ![[PROF_META_LP2]] = !{!"branch_weights", i32 [[LP2_EXIT:[0-9]+]], i32 [[LP2_BACKEDGE:[0-9]+]]}
;CHECK: ![[PROF_META_LP1]] = !{!"branch_weights", i32 [[LP1_EXIT:[0-9]+]], i32 [[LP1_BACKEDGE:[0-9]+]]}
;CHECK: ![[PROF_META_NEW]] = !{!"branch_weights", i32 [[LP3_BACKEDGE]], i32 [[LP1_EXIT]]}

; ModuleID = 'mm2.c'
source_filename = "mm2.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct._IO_FILE = type { i32, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, i32, i32, i64, i16, i8, [1 x i8], ptr, i64, ptr, ptr, ptr, ptr, i64, i32, [20 x i8] }
%struct._IO_marker = type { ptr, ptr, i32 }

@stderr = external dso_local local_unnamed_addr global ptr, align 8
@.str = private unnamed_addr constant [6 x i8] c" %i  \00", align 1

; Function Attrs: nounwind uwtable
define dso_local i32 @main() local_unnamed_addr #0 !prof !29 {
entry:
  %A = alloca [10 x [10 x [10 x i32]]], align 16
  call void @llvm.lifetime.start.p0(i64 4000, ptr nonnull %A) #3
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.inc20, %entry
  %indvars.iv59 = phi i64 [ 0, %entry ], [ %indvars.iv.next60, %for.inc20 ]
  br label %for.cond4.preheader

for.cond4.preheader:                              ; preds = %for.inc17, %for.cond1.preheader
  %indvars.iv56 = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next57, %for.inc17 ]
  br label %for.body6

for.body6:                                        ; preds = %for.body6, %for.cond4.preheader
  %indvars.iv53 = phi i64 [ 0, %for.cond4.preheader ], [ %indvars.iv.next54, %for.body6 ]
  %arrayidx10 = getelementptr inbounds [10 x [10 x [10 x i32]]], ptr %A, i64 0, i64 %indvars.iv59, i64 %indvars.iv56, i64 %indvars.iv53, !intel-tbaa !30
  %0 = load i32, ptr %arrayidx10, align 4, !tbaa !30
  %add = add nsw i32 %0, 1
  store i32 %add, ptr %arrayidx10, align 4, !tbaa !30
  %indvars.iv.next54 = add nuw nsw i64 %indvars.iv53, 1
  %exitcond55 = icmp eq i64 %indvars.iv.next54, 10
  br i1 %exitcond55, label %for.inc17, label %for.body6, !prof !37

for.inc17:                                        ; preds = %for.body6
  %indvars.iv.next57 = add nuw nsw i64 %indvars.iv56, 1
  %exitcond58 = icmp eq i64 %indvars.iv.next57, 10
  br i1 %exitcond58, label %for.inc20, label %for.cond4.preheader, !prof !38

for.inc20:                                        ; preds = %for.inc17
  %indvars.iv.next60 = add nuw nsw i64 %indvars.iv59, 1
  %exitcond61 = icmp eq i64 %indvars.iv.next60, 10
  br i1 %exitcond61, label %for.end32, label %for.cond1.preheader, !prof !39

for.end32:                                        ; preds = %for.body25
  call void @llvm.lifetime.end.p0(i64 4000, ptr nonnull %A) #3
  ret i32 0
}

; Function Attrs: argmemonly nounwind willreturn
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: nofree nounwind
declare dso_local i32 @fprintf(ptr nocapture, ptr nocapture readonly, ...) local_unnamed_addr #2

; Function Attrs: argmemonly nounwind willreturn
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #1

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="corei7" "target-features"="+cx16,+cx8,+fxsr,+mmx,+popcnt,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind willreturn }
attributes #2 = { nofree nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="corei7" "target-features"="+cx16,+cx8,+fxsr,+mmx,+popcnt,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind }
attributes #4 = { cold }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!28}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 1, !"ProfileSummary", !2}
!2 = !{!3, !4, !5, !6, !7, !8, !9, !10}
!3 = !{!"ProfileFormat", !"InstrProf"}
!4 = !{!"TotalCount", i64 1120}
!5 = !{!"MaxCount", i64 1000}
!6 = !{!"MaxInternalCount", i64 100}
!7 = !{!"MaxFunctionCount", i64 1000}
!8 = !{!"NumCounts", i64 5}
!9 = !{!"NumFunctions", i64 1}
!10 = !{!"DetailedSummary", !11}
!11 = !{!12, !13, !14, !15, !16, !17, !18, !19, !20, !21, !22, !23, !24, !25, !26, !27}
!12 = !{i32 10000, i64 1000, i32 1}
!13 = !{i32 100000, i64 1000, i32 1}
!14 = !{i32 200000, i64 1000, i32 1}
!15 = !{i32 300000, i64 1000, i32 1}
!16 = !{i32 400000, i64 1000, i32 1}
!17 = !{i32 500000, i64 1000, i32 1}
!18 = !{i32 600000, i64 1000, i32 1}
!19 = !{i32 700000, i64 1000, i32 1}
!20 = !{i32 800000, i64 1000, i32 1}
!21 = !{i32 900000, i64 100, i32 2}
!22 = !{i32 950000, i64 100, i32 2}
!23 = !{i32 990000, i64 10, i32 3}
!24 = !{i32 999000, i64 9, i32 4}
!25 = !{i32 999900, i64 9, i32 4}
!26 = !{i32 999990, i64 9, i32 4}
!27 = !{i32 999999, i64 9, i32 4}
!28 = !{!"Intel(R) oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)"}
!29 = !{!"function_entry_count", i64 1}
!30 = !{!31, !34, i64 0}
!31 = !{!"array@_ZTSA10_A10_A10_i", !32, i64 0}
!32 = !{!"array@_ZTSA10_A10_i", !33, i64 0}
!33 = !{!"array@_ZTSA10_i", !34, i64 0}
!34 = !{!"int", !35, i64 0}
!35 = !{!"omnipotent char", !36, i64 0}
!36 = !{!"Simple C/C++ TBAA"}
!37 = !{!"branch_weights", i32 100, i32 1000}
!38 = !{!"branch_weights", i32 10, i32 100}
!39 = !{!"branch_weights", i32 1, i32 10}
!40 = !{!41, !41, i32 0}
!41 = !{!"pointer@_ZTSP8_IO_FILE", !35, i64 0}
!42 = !{!"intel_profx", i64 9}
