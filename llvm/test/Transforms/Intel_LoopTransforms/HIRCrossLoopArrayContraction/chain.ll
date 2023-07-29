; RUN: opt -intel-libirc-allowed -hir-create-function-level-region -passes="hir-ssa-deconstruction,require<hir-loop-statistics>,hir-cross-loop-array-contraction,print<hir>,hir-cg" -force-hir-cg -aa-pipeline="basic-aa" -disable-output < %s 2>&1 | FileCheck %s

; BEGIN REGION { }
;       + DO i1 = 0, 99, 1   <DO_LOOP>
;       |   + DO i2 = 0, 99, 1   <DO_LOOP>
;       |   |   + DO i3 = 0, 99, 1   <DO_LOOP>
;       |   |   |   + DO i4 = 0, 99, 1   <DO_LOOP>
;       |   |   |   |   + DO i5 = 0, 99, 1   <DO_LOOP>
;       |   |   |   |   |   (%A)[0][i1][i2][i3][i4][i5] = i4 + i5;
;       |   |   |   |   + END LOOP
;       |   |   |   + END LOOP
;       |   |   + END LOOP
;       |   + END LOOP
;       + END LOOP
;
;
;       + DO i1 = 0, 99, 1   <DO_LOOP>
;       |   + DO i2 = 0, 99, 1   <DO_LOOP>
;       |   |   + DO i3 = 0, 99, 1   <DO_LOOP>
;       |   |   |   + DO i4 = 0, 99, 1   <DO_LOOP>
;       |   |   |   |   + DO i5 = 0, 99, 1   <DO_LOOP>
;       |   |   |   |   |   %5 = (%A)[0][i1][i2][i3][i5][i4];
;       |   |   |   |   |   (%B)[0][i1][i2][i3][i4][i5] = i5 + %5;
;       |   |   |   |   + END LOOP
;       |   |   |   + END LOOP
;       |   |   + END LOOP
;       |   + END LOOP
;       + END LOOP
;
;
;       + DO i1 = 0, 99, 1   <DO_LOOP>
;       |   + DO i2 = 0, 99, 1   <DO_LOOP>
;       |   |   + DO i3 = 0, 99, 1   <DO_LOOP>
;       |   |   |   + DO i4 = 0, 99, 1   <DO_LOOP>
;       |   |   |   |   + DO i5 = 0, 99, 1   <DO_LOOP>
;       |   |   |   |   |   %8 = (%B)[0][i1][i2][i3][i5][i4];
;       |   |   |   |   |   (%C)[0][i1][i2][i3][i4][i5] = i5 + %8;
;       |   |   |   |   + END LOOP
;       |   |   |   + END LOOP
;       |   |   + END LOOP
;       |   + END LOOP
;       + END LOOP
;
;       %7 = (%C)[0][1][2][3][4][5];
;       @llvm.lifetime.end.p0(40000000000,  &((i8*)(%C)[0]));
;       @llvm.lifetime.end.p0(40000000000,  &((i8*)(%B)[0]));
;       @llvm.lifetime.end.p0(40000000000,  &((i8*)(%A)[0]));
;       ret %7;
; END REGION

;      BEGIN REGION { modified }
;            + DO i1 = 0, 99, 1   <DO_LOOP>
;            |   + DO i2 = 0, 99, 1   <DO_LOOP>
;            |   |   + DO i3 = 0, 99, 1   <DO_LOOP>
;            |   |   |   + DO i4 = 0, 99, 1   <DO_LOOP>
;            |   |   |   |   + DO i5 = 0, 99, 1   <DO_LOOP>
;            |   |   |   |   |   (%A)[0][i1][i2][i3][i4][i5] = i4 + i5;
;            |   |   |   |   + END LOOP
;            |   |   |   + END LOOP
;            |   |   |
;            |   |   |
;            |   |   |   + DO i4 = 0, 99, 1   <DO_LOOP>
;            |   |   |   |   + DO i5 = 0, 99, 1   <DO_LOOP>
;            |   |   |   |   |   %5 = (%A)[0][i1][i2][i3][i5][i4];
;            |   |   |   |   |   (%B)[0][i1][i2][i3][i4][i5] = i5 + %5;
;            |   |   |   |   + END LOOP
;            |   |   |   + END LOOP
;            |   |   |
;            |   |   |
;            |   |   |   + DO i4 = 0, 99, 1   <DO_LOOP>
;            |   |   |   |   + DO i5 = 0, 99, 1   <DO_LOOP>
;            |   |   |   |   |   %8 = (%B)[0][i1][i2][i3][i5][i4];
;            |   |   |   |   |   (%C)[0][i1][i2][i3][i4][i5] = i5 + %8;
;            |   |   |   |   + END LOOP
;            |   |   |   + END LOOP
;            |   |   + END LOOP
;            |   + END LOOP
;            + END LOOP
;
;            %7 = (%C)[0][1][2][3][4][5];
;            @llvm.lifetime.end.p0(40000000000,  &((i8*)(%C)[0]));
;            @llvm.lifetime.end.p0(40000000000,  &((i8*)(%B)[0]));
;            @llvm.lifetime.end.p0(40000000000,  &((i8*)(%A)[0]));
;            ret %7;
;      END REGION

; CHECK:     BEGIN REGION { modified }
; CHECK:           + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK:           |   + DO i2 = 0, 99, 1   <DO_LOOP>
; CHECK:           |   |   + DO i3 = 0, 99, 1   <DO_LOOP>
; CHECK:           |   |   |   + DO i4 = 0, 99, 1   <DO_LOOP>
; CHECK:           |   |   |   |   + DO i5 = 0, 99, 1   <DO_LOOP>
; CHECK:           |   |   |   |   |   (%ContractedArray)[0][i4][i5] = i4 + i5;
; CHECK:           |   |   |   |   + END LOOP
; CHECK:           |   |   |   + END LOOP
; CHECK:           |   |   |
; CHECK:           |   |   |
; CHECK:           |   |   |   + DO i4 = 0, 99, 1   <DO_LOOP>
; CHECK:           |   |   |   |   + DO i5 = 0, 99, 1   <DO_LOOP>
; CHECK:           |   |   |   |   |   %2 = (%ContractedArray)[0][i5][i4];
; CHECK:           |   |   |   |   |   (%ContractedArray16)[0][i4][i5] = i5 + %2;
; CHECK:           |   |   |   |   + END LOOP
; CHECK:           |   |   |   + END LOOP
; CHECK:           |   |   |
; CHECK:           |   |   |
; CHECK:           |   |   |   + DO i4 = 0, 99, 1   <DO_LOOP>
; CHECK:           |   |   |   |   + DO i5 = 0, 99, 1   <DO_LOOP>
; CHECK:           |   |   |   |   |   %5 = (%ContractedArray16)[0][i5][i4];
; CHECK:           |   |   |   |   |   (%C)[0][i1][i2][i3][i4][i5] = i5 + %5;
; CHECK:           |   |   |   |   + END LOOP
; CHECK:           |   |   |   + END LOOP
; CHECK:           |   |   + END LOOP
; CHECK:           |   + END LOOP
; CHECK:           + END LOOP
;
; CHECK:           %4 = (%C)[0][1][2][3][4][5];
; CHECK:           @llvm.lifetime.end.p0(40000000000,  &((%C)[0]));
; CHECK:           @llvm.lifetime.end.p0(40000000000,  &((%B)[0]));
; CHECK:           @llvm.lifetime.end.p0(40000000000,  &((%A)[0]));
; CHECK:           ret %4;
; CHECK:     END REGION


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind readnone uwtable
define dso_local i32 @shell() local_unnamed_addr #0 {
entry:
  %A = alloca [100 x [100 x [100 x [100 x [100 x i32]]]]], align 16
  %B = alloca [100 x [100 x [100 x [100 x [100 x i32]]]]], align 16
  %C = alloca [100 x [100 x [100 x [100 x [100 x i32]]]]], align 16
  call void @llvm.lifetime.start.p0(i64 40000000000, ptr nonnull %A) #3
  call void @llvm.lifetime.start.p0(i64 40000000000, ptr nonnull %B) #3
  call void @llvm.lifetime.start.p0(i64 40000000000, ptr nonnull %C) #3
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %entry, %for.cond.cleanup3
  %indvars.iv278 = phi i64 [ 0, %entry ], [ %indvars.iv.next279, %for.cond.cleanup3 ]
  br label %for.cond5.preheader

for.cond5.preheader:                              ; preds = %for.cond1.preheader, %for.cond.cleanup7
  %indvars.iv275 = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next276, %for.cond.cleanup7 ]
  br label %for.cond9.preheader

for.cond.cleanup3:                                ; preds = %for.cond.cleanup7
  %indvars.iv.next279 = add nuw nsw i64 %indvars.iv278, 1
  %exitcond280.not = icmp eq i64 %indvars.iv.next279, 100
  br i1 %exitcond280.not, label %for.cond43.preheader.preheader, label %for.cond1.preheader, !llvm.loop !2

for.cond43.preheader.preheader:                   ; preds = %for.cond.cleanup3
  br label %for.cond43.preheader

for.cond9.preheader:                              ; preds = %for.cond5.preheader, %for.cond.cleanup11
  %indvars.iv272 = phi i64 [ 0, %for.cond5.preheader ], [ %indvars.iv.next273, %for.cond.cleanup11 ]
  br label %for.cond13.preheader

for.cond.cleanup7:                                ; preds = %for.cond.cleanup11
  %indvars.iv.next276 = add nuw nsw i64 %indvars.iv275, 1
  %exitcond277.not = icmp eq i64 %indvars.iv.next276, 100
  br i1 %exitcond277.not, label %for.cond.cleanup3, label %for.cond5.preheader, !llvm.loop !4

for.cond13.preheader:                             ; preds = %for.cond9.preheader, %for.cond.cleanup15
  %indvars.iv269 = phi i64 [ 0, %for.cond9.preheader ], [ %indvars.iv.next270, %for.cond.cleanup15 ]
  br label %for.body16

for.cond.cleanup11:                               ; preds = %for.cond.cleanup15
  %indvars.iv.next273 = add nuw nsw i64 %indvars.iv272, 1
  %exitcond274.not = icmp eq i64 %indvars.iv.next273, 100
  br i1 %exitcond274.not, label %for.cond.cleanup7, label %for.cond9.preheader, !llvm.loop !5

for.cond.cleanup15:                               ; preds = %for.body16
  %indvars.iv.next270 = add nuw nsw i64 %indvars.iv269, 1
  %exitcond271.not = icmp eq i64 %indvars.iv.next270, 100
  br i1 %exitcond271.not, label %for.cond.cleanup11, label %for.cond13.preheader, !llvm.loop !6

for.body16:                                       ; preds = %for.cond13.preheader, %for.body16
  %indvars.iv265 = phi i64 [ 0, %for.cond13.preheader ], [ %indvars.iv.next266, %for.body16 ]
  %0 = add nuw nsw i64 %indvars.iv265, %indvars.iv269
  %arrayidx24 = getelementptr inbounds [100 x [100 x [100 x [100 x [100 x i32]]]]], ptr %A, i64 0, i64 %indvars.iv278, i64 %indvars.iv275, i64 %indvars.iv272, i64 %indvars.iv269, i64 %indvars.iv265, !intel-tbaa !7
  %1 = trunc i64 %0 to i32
  store i32 %1, ptr %arrayidx24, align 4, !tbaa !7
  %indvars.iv.next266 = add nuw nsw i64 %indvars.iv265, 1
  %exitcond268.not = icmp eq i64 %indvars.iv.next266, 100
  br i1 %exitcond268.not, label %for.cond.cleanup15, label %for.body16, !llvm.loop !16

for.cond43.preheader:                             ; preds = %for.cond43.preheader.preheader, %for.cond.cleanup45
  %indvars.iv262 = phi i64 [ %indvars.iv.next263, %for.cond.cleanup45 ], [ 0, %for.cond43.preheader.preheader ]
  br label %for.cond48.preheader

for.cond48.preheader:                             ; preds = %for.cond43.preheader, %for.cond.cleanup50
  %indvars.iv259 = phi i64 [ 0, %for.cond43.preheader ], [ %indvars.iv.next260, %for.cond.cleanup50 ]
  br label %for.cond53.preheader

for.cond.cleanup45:                               ; preds = %for.cond.cleanup50
  %indvars.iv.next263 = add nuw nsw i64 %indvars.iv262, 1
  %exitcond264.not = icmp eq i64 %indvars.iv.next263, 100
  br i1 %exitcond264.not, label %for.cond104.preheader.preheader, label %for.cond43.preheader, !llvm.loop !17

for.cond104.preheader.preheader:                  ; preds = %for.cond.cleanup45
  br label %for.cond104.preheader

for.cond53.preheader:                             ; preds = %for.cond48.preheader, %for.cond.cleanup55
  %indvars.iv256 = phi i64 [ 0, %for.cond48.preheader ], [ %indvars.iv.next257, %for.cond.cleanup55 ]
  br label %for.cond58.preheader

for.cond.cleanup50:                               ; preds = %for.cond.cleanup55
  %indvars.iv.next260 = add nuw nsw i64 %indvars.iv259, 1
  %exitcond261.not = icmp eq i64 %indvars.iv.next260, 100
  br i1 %exitcond261.not, label %for.cond.cleanup45, label %for.cond48.preheader, !llvm.loop !18

for.cond58.preheader:                             ; preds = %for.cond53.preheader, %for.cond.cleanup60
  %indvars.iv253 = phi i64 [ 0, %for.cond53.preheader ], [ %indvars.iv.next254, %for.cond.cleanup60 ]
  br label %for.body61

for.cond.cleanup55:                               ; preds = %for.cond.cleanup60
  %indvars.iv.next257 = add nuw nsw i64 %indvars.iv256, 1
  %exitcond258.not = icmp eq i64 %indvars.iv.next257, 100
  br i1 %exitcond258.not, label %for.cond.cleanup50, label %for.cond53.preheader, !llvm.loop !19

for.cond.cleanup60:                               ; preds = %for.body61
  %indvars.iv.next254 = add nuw nsw i64 %indvars.iv253, 1
  %exitcond255.not = icmp eq i64 %indvars.iv.next254, 100
  br i1 %exitcond255.not, label %for.cond.cleanup55, label %for.cond58.preheader, !llvm.loop !20

for.body61:                                       ; preds = %for.cond58.preheader, %for.body61
  %indvars.iv250 = phi i64 [ 0, %for.cond58.preheader ], [ %indvars.iv.next251, %for.body61 ]
  %arrayidx71 = getelementptr inbounds [100 x [100 x [100 x [100 x [100 x i32]]]]], ptr %A, i64 0, i64 %indvars.iv262, i64 %indvars.iv259, i64 %indvars.iv256, i64 %indvars.iv250, i64 %indvars.iv253, !intel-tbaa !7
  %2 = load i32, ptr %arrayidx71, align 4, !tbaa !7
  %3 = trunc i64 %indvars.iv250 to i32
  %add72 = add nsw i32 %2, %3
  %arrayidx82 = getelementptr inbounds [100 x [100 x [100 x [100 x [100 x i32]]]]], ptr %B, i64 0, i64 %indvars.iv262, i64 %indvars.iv259, i64 %indvars.iv256, i64 %indvars.iv253, i64 %indvars.iv250, !intel-tbaa !7
  store i32 %add72, ptr %arrayidx82, align 4, !tbaa !7
  %indvars.iv.next251 = add nuw nsw i64 %indvars.iv250, 1
  %exitcond252.not = icmp eq i64 %indvars.iv.next251, 100
  br i1 %exitcond252.not, label %for.cond.cleanup60, label %for.body61, !llvm.loop !21

for.cond104.preheader:                            ; preds = %for.cond104.preheader.preheader, %for.cond.cleanup106
  %indvars.iv247 = phi i64 [ %indvars.iv.next248, %for.cond.cleanup106 ], [ 0, %for.cond104.preheader.preheader ]
  br label %for.cond109.preheader

for.cond.cleanup101:                              ; preds = %for.cond.cleanup106
  %arrayidx163 = getelementptr inbounds [100 x [100 x [100 x [100 x [100 x i32]]]]], ptr %C, i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, !intel-tbaa !7
  %4 = load i32, ptr %arrayidx163, align 4, !tbaa !7
  call void @llvm.lifetime.end.p0(i64 40000000000, ptr nonnull %C) #3
  call void @llvm.lifetime.end.p0(i64 40000000000, ptr nonnull %B) #3
  call void @llvm.lifetime.end.p0(i64 40000000000, ptr nonnull %A) #3
  ret i32 %4

for.cond109.preheader:                            ; preds = %for.cond104.preheader, %for.cond.cleanup111
  %indvars.iv244 = phi i64 [ 0, %for.cond104.preheader ], [ %indvars.iv.next245, %for.cond.cleanup111 ]
  br label %for.cond114.preheader

for.cond.cleanup106:                              ; preds = %for.cond.cleanup111
  %indvars.iv.next248 = add nuw nsw i64 %indvars.iv247, 1
  %exitcond249.not = icmp eq i64 %indvars.iv.next248, 100
  br i1 %exitcond249.not, label %for.cond.cleanup101, label %for.cond104.preheader, !llvm.loop !22

for.cond114.preheader:                            ; preds = %for.cond109.preheader, %for.cond.cleanup116
  %indvars.iv241 = phi i64 [ 0, %for.cond109.preheader ], [ %indvars.iv.next242, %for.cond.cleanup116 ]
  br label %for.cond119.preheader

for.cond.cleanup111:                              ; preds = %for.cond.cleanup116
  %indvars.iv.next245 = add nuw nsw i64 %indvars.iv244, 1
  %exitcond246.not = icmp eq i64 %indvars.iv.next245, 100
  br i1 %exitcond246.not, label %for.cond.cleanup106, label %for.cond109.preheader, !llvm.loop !23

for.cond119.preheader:                            ; preds = %for.cond114.preheader, %for.cond.cleanup121
  %indvars.iv238 = phi i64 [ 0, %for.cond114.preheader ], [ %indvars.iv.next239, %for.cond.cleanup121 ]
  br label %for.body122

for.cond.cleanup116:                              ; preds = %for.cond.cleanup121
  %indvars.iv.next242 = add nuw nsw i64 %indvars.iv241, 1
  %exitcond243.not = icmp eq i64 %indvars.iv.next242, 100
  br i1 %exitcond243.not, label %for.cond.cleanup111, label %for.cond114.preheader, !llvm.loop !24

for.cond.cleanup121:                              ; preds = %for.body122
  %indvars.iv.next239 = add nuw nsw i64 %indvars.iv238, 1
  %exitcond240.not = icmp eq i64 %indvars.iv.next239, 100
  br i1 %exitcond240.not, label %for.cond.cleanup116, label %for.cond119.preheader, !llvm.loop !25

for.body122:                                      ; preds = %for.cond119.preheader, %for.body122
  %indvars.iv = phi i64 [ 0, %for.cond119.preheader ], [ %indvars.iv.next, %for.body122 ]
  %arrayidx132 = getelementptr inbounds [100 x [100 x [100 x [100 x [100 x i32]]]]], ptr %B, i64 0, i64 %indvars.iv247, i64 %indvars.iv244, i64 %indvars.iv241, i64 %indvars.iv, i64 %indvars.iv238, !intel-tbaa !7
  %5 = load i32, ptr %arrayidx132, align 4, !tbaa !7
  %6 = trunc i64 %indvars.iv to i32
  %add133 = add nsw i32 %5, %6
  %arrayidx143 = getelementptr inbounds [100 x [100 x [100 x [100 x [100 x i32]]]]], ptr %C, i64 0, i64 %indvars.iv247, i64 %indvars.iv244, i64 %indvars.iv241, i64 %indvars.iv238, i64 %indvars.iv, !intel-tbaa !7
  store i32 %add133, ptr %arrayidx143, align 4, !tbaa !7
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond.not, label %for.cond.cleanup121, label %for.body122, !llvm.loop !26
}

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: nounwind readnone uwtable
define dso_local i32 @main() local_unnamed_addr #2 {
entry:
  %call = tail call i32 @shell()
  ret i32 %call
}

attributes #0 = { noinline nounwind readnone uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="corei7-avx" "target-features"="+avx,+cx16,+cx8,+fxsr,+mmx,+pclmul,+popcnt,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #1 = { argmemonly nofree nosync nounwind willreturn }
attributes #2 = { nounwind readnone uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="corei7-avx" "target-features"="+avx,+cx16,+cx8,+fxsr,+mmx,+pclmul,+popcnt,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #3 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"Intel(R) oneAPI DPC++ Compiler 2021.2.0 (YYYY.x.0.MMDD)"}
!2 = distinct !{!2, !3}
!3 = !{!"llvm.loop.mustprogress"}
!4 = distinct !{!4, !3}
!5 = distinct !{!5, !3}
!6 = distinct !{!6, !3}
!7 = !{!8, !13, i64 0}
!8 = !{!"array@_ZTSA100_A100_A100_A100_A100_i", !9, i64 0}
!9 = !{!"array@_ZTSA100_A100_A100_A100_i", !10, i64 0}
!10 = !{!"array@_ZTSA100_A100_A100_i", !11, i64 0}
!11 = !{!"array@_ZTSA100_A100_i", !12, i64 0}
!12 = !{!"array@_ZTSA100_i", !13, i64 0}
!13 = !{!"int", !14, i64 0}
!14 = !{!"omnipotent char", !15, i64 0}
!15 = !{!"Simple C/C++ TBAA"}
!16 = distinct !{!16, !3}
!17 = distinct !{!17, !3}
!18 = distinct !{!18, !3}
!19 = distinct !{!19, !3}
!20 = distinct !{!20, !3}
!21 = distinct !{!21, !3}
!22 = distinct !{!22, !3}
!23 = distinct !{!23, !3}
!24 = distinct !{!24, !3}
!25 = distinct !{!25, !3}
!26 = distinct !{!26, !3}
