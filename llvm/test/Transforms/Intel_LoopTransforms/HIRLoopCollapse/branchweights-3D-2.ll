
; RUN: opt -passes="hir-ssa-deconstruction,hir-loop-collapse,hir-cg" -S %s | FileCheck %s
;
; Make sure HIR loop collapse pass correctly modifies profile-related metadata ("branch weights"). The collapsed loop should use the backedge taken weight from the innermost loop and non-taken weight from the outermost loop.
; This testcase triggers loop collapse on i2-i3 level.


; *** Source Code ***
;int foo(void) {
;  int i, j, k, n, res;
;  int a1[15][15][15], b1[15][15][15], c1[15][15], d1[15][15][15];
;  int a2[15][15][15], b2[15][15][15], c2[15][15], d2[15][15][15];
;  int a3[15][15][15], b3[15][15][15], c3[15][15], d3[15][15][15];
;
;  for (k = 0; k < 15; k++) {
;    for (i = 0; i < 15; i++) {
;      for (j = 0; j < 15; j++) {
;        a1[k][i][j] = 1;
;        a2[k][i][j] = 1;
;        a3[k][i][j] = 1;
;        b1[k][i][j] = 1;
;        b2[k][i][j] = 1;
;        b3[k][i][j] = 1;
;        c1[i][j] = 1;
;        c2[i][j] = 1;
;        c3[i][j] = 1;
;        d1[k][i][j] = 1;
;        d2[k][i][j] = 1;
;        d3[k][i][j] = 1;
;      }
;    }
;  }
;
;  return a1[0][0][0] + b2[1][1][1] + c3[2][2] + d2[3][3][3];
;}

;    *** IR Dump Before HIR Loop Collapse ***
;             BEGIN REGION { }
;                   + DO i1 = 0, 14, 1   <DO_LOOP>
;                   |   + DO i2 = 0, 14, 1   <DO_LOOP>
;                   |   |   + DO i3 = 0, 14, 1   <DO_LOOP>
;                   |   |   |   (%a1)[0][i1][i2][i3] = 1;
;                   |   |   |   (%b2)[0][i1][i2][i3] = 1;
;                   |   |   |   (%c3)[0][i2][i3] = 1;
;                   |   |   |   (%d2)[0][i1][i2][i3] = 1;
;                   |   |   + END LOOP
;                   |   + END LOOP
;                   + END LOOP
;             END REGION

;    *** IR Dump After HIR Loop Collapse ***
;
;             BEGIN REGION { modified }
;                   + DO i1 = 0, 14, 1   <DO_LOOP>
;                   |   + DO i2 = 0, 224, 1   <DO_LOOP>
;                   |   |   (%a1)[0][i1][0][i2] = 1;
;                   |   |   (%b2)[0][i1][0][i2] = 1;
;                   |   |   (%c3)[0][0][i2] = 1;
;                   |   |   (%d2)[0][i1][0][i2] = 1;
;                   |   + END LOOP
;                   + END LOOP
;             END REGION

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
;CHECK-SAME: !prof ![[PROF_META_NEW1:[0-9]+]]
;CHECK: afterloop.{{[0-9]+}}:
;CHECK: br i1
;CHECK-SAME: !prof ![[PROF_META_NEW2:[0-9]+]]

;CHECK: ![[PROF_META_LP3]] = !{!"branch_weights", i32 [[LP3_EXIT:[0-9]+]], i32 [[LP3_BACKEDGE:[0-9]+]]}
;CHECK: ![[PROF_META_LP2]] = !{!"branch_weights", i32 [[LP2_EXIT:[0-9]+]], i32 [[LP2_BACKEDGE:[0-9]+]]}
;CHECK: ![[PROF_META_LP1]] = !{!"branch_weights", i32 [[LP1_EXIT:[0-9]+]], i32 [[LP1_BACKEDGE:[0-9]+]]}
;CHECK: ![[PROF_META_NEW1]] = !{!"branch_weights", i32 [[LP3_BACKEDGE]], i32 [[LP2_EXIT]]}
;CHECK: ![[PROF_META_NEW2]] = !{!"branch_weights", i32 [[LP1_BACKEDGE]], i32 [[LP1_EXIT]]}

; ModuleID = 'lc.c'
source_filename = "lc.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct._IO_FILE = type { i32, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, i32, i32, i64, i16, i8, [1 x i8], ptr, i64, ptr, ptr, ptr, ptr, i64, i32, [20 x i8] }
%struct._IO_marker = type { ptr, ptr, i32 }

@stderr = external dso_local local_unnamed_addr global ptr, align 8
@.str = private unnamed_addr constant [5 x i8] c" %i \00", align 1

; Function Attrs: nounwind readnone uwtable
define dso_local i32 @foo() local_unnamed_addr #0 !prof !29 {
entry:
  %a1 = alloca [15 x [15 x [15 x i32]]], align 16
  %b2 = alloca [15 x [15 x [15 x i32]]], align 16
  %d2 = alloca [15 x [15 x [15 x i32]]], align 16
  %c3 = alloca [15 x [15 x i32]], align 16
  call void @llvm.lifetime.start.p0(i64 13500, ptr nonnull %a1) #4
  call void @llvm.lifetime.start.p0(i64 13500, ptr nonnull %b2) #4
  call void @llvm.lifetime.start.p0(i64 13500, ptr nonnull %d2) #4
  call void @llvm.lifetime.start.p0(i64 900, ptr nonnull %c3) #4
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.inc74, %entry
  %indvars.iv135 = phi i64 [ 0, %entry ], [ %indvars.iv.next136, %for.inc74 ]
  br label %for.cond4.preheader

for.cond4.preheader:                              ; preds = %for.inc71, %for.cond1.preheader
  %indvars.iv132 = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next133, %for.inc71 ]
  br label %for.body6

for.body6:                                        ; preds = %for.body6, %for.cond4.preheader
  %indvars.iv = phi i64 [ 0, %for.cond4.preheader ], [ %indvars.iv.next, %for.body6 ]
  %arrayidx10 = getelementptr inbounds [15 x [15 x [15 x i32]]], ptr %a1, i64 0, i64 %indvars.iv135, i64 %indvars.iv132, i64 %indvars.iv, !intel-tbaa !30
  store i32 1, ptr %arrayidx10, align 4, !tbaa !30
  %arrayidx34 = getelementptr inbounds [15 x [15 x [15 x i32]]], ptr %b2, i64 0, i64 %indvars.iv135, i64 %indvars.iv132, i64 %indvars.iv, !intel-tbaa !30
  store i32 1, ptr %arrayidx34, align 4, !tbaa !30
  %arrayidx52 = getelementptr inbounds [15 x [15 x i32]], ptr %c3, i64 0, i64 %indvars.iv132, i64 %indvars.iv, !intel-tbaa !37
  store i32 1, ptr %arrayidx52, align 4, !tbaa !37
  %arrayidx64 = getelementptr inbounds [15 x [15 x [15 x i32]]], ptr %d2, i64 0, i64 %indvars.iv135, i64 %indvars.iv132, i64 %indvars.iv, !intel-tbaa !30
  store i32 1, ptr %arrayidx64, align 4, !tbaa !30
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 15
  br i1 %exitcond, label %for.inc71, label %for.body6, !prof !38

for.inc71:                                        ; preds = %for.body6
  %indvars.iv.next133 = add nuw nsw i64 %indvars.iv132, 1
  %exitcond134 = icmp eq i64 %indvars.iv.next133, 15
  br i1 %exitcond134, label %for.inc74, label %for.cond4.preheader, !prof !39

for.inc74:                                        ; preds = %for.inc71
  %indvars.iv.next136 = add nuw nsw i64 %indvars.iv135, 1
  %exitcond137 = icmp eq i64 %indvars.iv.next136, 15
  br i1 %exitcond137, label %for.end76, label %for.cond1.preheader, !prof !40

for.end76:                                        ; preds = %for.inc74
  %0 = load i32, ptr %a1, align 16, !tbaa !30
  %arrayidx82 = getelementptr inbounds [15 x [15 x [15 x i32]]], ptr %b2, i64 0, i64 1, i64 1, i64 1, !intel-tbaa !30
  %1 = load i32, ptr %arrayidx82, align 4, !tbaa !30
  %add = add nsw i32 %1, %0
  %arrayidx84 = getelementptr inbounds [15 x [15 x i32]], ptr %c3, i64 0, i64 2, i64 2, !intel-tbaa !37
  %2 = load i32, ptr %arrayidx84, align 8, !tbaa !37
  %add85 = add nsw i32 %add, %2
  %arrayidx88 = getelementptr inbounds [15 x [15 x [15 x i32]]], ptr %d2, i64 0, i64 3, i64 3, i64 3, !intel-tbaa !30
  %3 = load i32, ptr %arrayidx88, align 4, !tbaa !30
  %add89 = add nsw i32 %add85, %3
  call void @llvm.lifetime.end.p0(i64 900, ptr nonnull %c3) #4
  call void @llvm.lifetime.end.p0(i64 13500, ptr nonnull %d2) #4
  call void @llvm.lifetime.end.p0(i64 13500, ptr nonnull %b2) #4
  call void @llvm.lifetime.end.p0(i64 13500, ptr nonnull %a1) #4
  ret i32 %add89
}

; Function Attrs: argmemonly nounwind willreturn
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: argmemonly nounwind willreturn
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: cold nofree nounwind uwtable
define dso_local i32 @main() local_unnamed_addr #2 !prof !29 {
entry:
  %0 = load ptr, ptr @stderr, align 8, !tbaa !41
  %call = tail call i32 @foo(), !intel-profx !43
  %call1 = tail call i32 (ptr, ptr, ...) @fprintf(ptr %0, ptr @.str, i32 %call) #5, !intel-profx !43
  ret i32 0
}

; Function Attrs: nofree nounwind
declare dso_local i32 @fprintf(ptr nocapture, ptr nocapture readonly, ...) local_unnamed_addr #3

attributes #0 = { nounwind readnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="corei7" "target-features"="+cx16,+cx8,+fxsr,+mmx,+popcnt,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind willreturn }
attributes #2 = { cold nofree nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="corei7" "target-features"="+cx16,+cx8,+fxsr,+mmx,+popcnt,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nofree nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="corei7" "target-features"="+cx16,+cx8,+fxsr,+mmx,+popcnt,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #4 = { nounwind }
attributes #5 = { cold }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!28}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 1, !"ProfileSummary", !2}
!2 = !{!3, !4, !5, !6, !7, !8, !9, !10}
!3 = !{!"ProfileFormat", !"InstrProf"}
!4 = !{!"TotalCount", i64 3617}
!5 = !{!"MaxCount", i64 3375}
!6 = !{!"MaxInternalCount", i64 225}
!7 = !{!"MaxFunctionCount", i64 3375}
!8 = !{!"NumCounts", i64 5}
!9 = !{!"NumFunctions", i64 2}
!10 = !{!"DetailedSummary", !11}
!11 = !{!12, !13, !14, !15, !16, !17, !18, !19, !20, !21, !22, !23, !24, !25, !26, !27}
!12 = !{i32 10000, i64 3375, i32 1}
!13 = !{i32 100000, i64 3375, i32 1}
!14 = !{i32 200000, i64 3375, i32 1}
!15 = !{i32 300000, i64 3375, i32 1}
!16 = !{i32 400000, i64 3375, i32 1}
!17 = !{i32 500000, i64 3375, i32 1}
!18 = !{i32 600000, i64 3375, i32 1}
!19 = !{i32 700000, i64 3375, i32 1}
!20 = !{i32 800000, i64 3375, i32 1}
!21 = !{i32 900000, i64 3375, i32 1}
!22 = !{i32 950000, i64 225, i32 2}
!23 = !{i32 990000, i64 225, i32 2}
!24 = !{i32 999000, i64 15, i32 3}
!25 = !{i32 999900, i64 1, i32 5}
!26 = !{i32 999990, i64 1, i32 5}
!27 = !{i32 999999, i64 1, i32 5}
!28 = !{!"Intel(R) oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)"}
!29 = !{!"function_entry_count", i64 1}
!30 = !{!31, !34, i64 0}
!31 = !{!"array@_ZTSA15_A15_A15_i", !32, i64 0}
!32 = !{!"array@_ZTSA15_A15_i", !33, i64 0}
!33 = !{!"array@_ZTSA15_i", !34, i64 0}
!34 = !{!"int", !35, i64 0}
!35 = !{!"omnipotent char", !36, i64 0}
!36 = !{!"Simple C/C++ TBAA"}
!37 = !{!32, !34, i64 0}
!38 = !{!"branch_weights", i32 225, i32 3375}
!39 = !{!"branch_weights", i32 15, i32 225}
!40 = !{!"branch_weights", i32 1, i32 15}
!41 = !{!42, !42, i64 0}
!42 = !{!"pointer@_ZTSP8_IO_FILE", !35, i64 0}
!43 = !{!"intel_profx", i64 1}
