;RUN: opt -passes="hir-ssa-deconstruction,hir-loop-distribute-loopnest,print<hir>" -aa-pipeline="basic-aa"  < %s 2>&1 | FileCheck %s

; Loop DO i2 should not be distributed because of output temp dependency on %jp.2.lcssa.

; CHECK: BEGIN REGION
; CHECK-NOT: modified
; CHECK:       + DO i1 = 0, 36, 1
; CHECK:       |   %3 = (%s)[0][i1];
; CHECK:       |
; CHECK:       |   + DO i2 = 0, i1 + -1, 1
; CHECK:       |   |   %jp.2.lcssa = i2 + 1;
; CHECK-NOT:   |   + END LOOP
;              |
; CHECK-NOT:   |   + DO i2 = 0, i1 + -1, 1
; CHECK:       |   |   + DO i3 = 0, -1 * i2 + 1, 1
; CHECK:       |   |   |   %4 = (%uv9)[0][0][i2 + i3 + 1];
; CHECK:       |   |   |   (%uv9)[0][0][i2 + i3 + 1] = %4 + 1;
; CHECK:       |   |   + END LOOP
; CHECK:       |   |      %jp.2.lcssa = 3;
; CHECK:       |   + END LOOP
; CHECK:       |      %jp.050 = %jp.2.lcssa;
; CHECK:       |
; CHECK:       |   %5 = (%s)[0][i1 + 1];
; CHECK:       |   (%s)[0][i1 + 1] = %5 + %jp.050;
; CHECK:       + END LOOP
; CHECK: END REGION

source_filename = "atg_CMPLRLLVM-25387.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [10 x i8] c"res = %u\0A\00", align 1

; Function Attrs: nofree norecurse nounwind uwtable writeonly
define dso_local void @init(ptr nocapture %a, i32 %n, i32 %seed) local_unnamed_addr #0 {
entry:
  store i32 1, ptr %a, align 4, !tbaa !2
  ret void
}

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: nofree nounwind uwtable
define dso_local i32 @main() local_unnamed_addr #2 {
entry:
  %s = alloca [100 x i32], align 16
  %uv9 = alloca [100 x [100 x i32]], align 16
  %0 = bitcast ptr %s to ptr
  call void @llvm.lifetime.start.p0(i64 400, ptr nonnull %0) #5
  call void @llvm.memset.p0.i64(ptr nonnull align 16 dereferenceable(400) %0, i8 0, i64 400, i1 false)
  %1 = bitcast ptr %uv9 to ptr
  call void @llvm.lifetime.start.p0(i64 40000, ptr nonnull %1) #5
  call void @llvm.memset.p0.i64(ptr nonnull align 16 dereferenceable(40000) %1, i8 0, i64 40000, i1 false)
  br label %for.body

for.body:                                         ; preds = %entry, %for.end13
  %indvars.iv59 = phi i64 [ 1, %entry ], [ %indvars.iv.next60, %for.end13 ]
  %jp.050 = phi i32 [ 0, %entry ], [ %jp.1.lcssa, %for.end13 ]
  %2 = add nsw i64 %indvars.iv59, -1
  %arrayidx = getelementptr inbounds [100 x i32], ptr %s, i64 0, i64 %2, !intel-tbaa !6
  %3 = load i32, ptr %arrayidx, align 4, !tbaa !6
  %cmp246 = icmp ugt i64 %indvars.iv59, 1
  br i1 %cmp246, label %for.cond4.preheader.preheader, label %for.end13

for.cond4.preheader.preheader:                    ; preds = %for.body
  br label %for.cond4.preheader

for.cond4.preheader:                              ; preds = %for.cond4.preheader.preheader, %for.inc11
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.inc11 ], [ 1, %for.cond4.preheader.preheader ]
  %kc.047 = phi i32 [ %inc12, %for.inc11 ], [ 1, %for.cond4.preheader.preheader ]
  %cmp544 = icmp ult i32 %kc.047, 3
  br i1 %cmp544, label %for.body6.preheader, label %for.inc11

for.body6.preheader:                              ; preds = %for.cond4.preheader
  br label %for.body6

for.body6:                                        ; preds = %for.body6.preheader, %for.body6
  %indvars.iv52 = phi i64 [ %indvars.iv.next53, %for.body6 ], [ %indvars.iv, %for.body6.preheader ]
  %arrayidx10 = getelementptr inbounds [100 x [100 x i32]], ptr %uv9, i64 0, i64 0, i64 %indvars.iv52, !intel-tbaa !8
  %4 = load i32, ptr %arrayidx10, align 4, !tbaa !8
  %add = add i32 %4, 1
  store i32 %add, ptr %arrayidx10, align 4, !tbaa !8
  %indvars.iv.next53 = add nuw nsw i64 %indvars.iv52, 1
  %exitcond = icmp eq i64 %indvars.iv.next53, 3
  br i1 %exitcond, label %for.inc11.loopexit, label %for.body6, !llvm.loop !10

for.inc11.loopexit:                               ; preds = %for.body6
  br label %for.inc11

for.inc11:                                        ; preds = %for.inc11.loopexit, %for.cond4.preheader
  %jp.2.lcssa = phi i32 [ %kc.047, %for.cond4.preheader ], [ 3, %for.inc11.loopexit ]
  %inc12 = add nuw nsw i32 %kc.047, 1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond58 = icmp eq i64 %indvars.iv.next, %indvars.iv59
  br i1 %exitcond58, label %for.end13.loopexit, label %for.cond4.preheader, !llvm.loop !12

for.end13.loopexit:                               ; preds = %for.inc11
  %jp.2.lcssa.lcssa = phi i32 [ %jp.2.lcssa, %for.inc11 ]
  br label %for.end13

for.end13:                                        ; preds = %for.end13.loopexit, %for.body
  %jp.1.lcssa = phi i32 [ %jp.050, %for.body ], [ %jp.2.lcssa.lcssa, %for.end13.loopexit ]
  %arrayidx15 = getelementptr inbounds [100 x i32], ptr %s, i64 0, i64 %indvars.iv59, !intel-tbaa !6
  %5 = load i32, ptr %arrayidx15, align 4, !tbaa !6
  %add16 = add i32 %5, %jp.1.lcssa
  store i32 %add16, ptr %arrayidx15, align 4, !tbaa !6
  %indvars.iv.next60 = add nuw nsw i64 %indvars.iv59, 1
  %exitcond62 = icmp eq i64 %indvars.iv.next60, 38
  br i1 %exitcond62, label %for.end19, label %for.body, !llvm.loop !13

for.end19:                                        ; preds = %for.end13
  %.lcssa = phi i32 [ %3, %for.end13 ]
  %call = tail call i32 (ptr, ...) @printf(ptr nonnull dereferenceable(1) @.str, i32 %.lcssa)
  call void @llvm.lifetime.end.p0(i64 40000, ptr nonnull %1) #5
  call void @llvm.lifetime.end.p0(i64 400, ptr nonnull %0) #5
  ret i32 0
}

; Function Attrs: argmemonly nofree nosync nounwind willreturn writeonly
declare void @llvm.memset.p0.i64(ptr nocapture writeonly, i8, i64, i1 immarg) #3

; Function Attrs: nofree nounwind
declare dso_local noundef i32 @printf(ptr nocapture noundef readonly, ...) local_unnamed_addr #4

attributes #0 = { nofree norecurse nounwind uwtable writeonly "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="corei7-avx" "target-features"="+avx,+cx16,+cx8,+fxsr,+mmx,+pclmul,+popcnt,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #1 = { argmemonly nofree nosync nounwind willreturn }
attributes #2 = { nofree nounwind uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="corei7-avx" "target-features"="+avx,+cx16,+cx8,+fxsr,+mmx,+pclmul,+popcnt,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #3 = { argmemonly nofree nosync nounwind willreturn writeonly }
attributes #4 = { nofree nounwind "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="corei7-avx" "target-features"="+avx,+cx16,+cx8,+fxsr,+mmx,+pclmul,+popcnt,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #5 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"Intel(R) oneAPI DPC++ Compiler 2021.2.0 (YYYY.2021.0.MMDD)"}
!2 = !{!3, !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
!6 = !{!7, !3, i64 0}
!7 = !{!"array@_ZTSA100_j", !3, i64 0}
!8 = !{!9, !3, i64 0}
!9 = !{!"array@_ZTSA100_A100_j", !7, i64 0}
!10 = distinct !{!10, !11}
!11 = !{!"llvm.loop.mustprogress"}
!12 = distinct !{!12, !11}
!13 = distinct !{!13, !11}
