; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-sinking-for-perfect-loopnest,hir-undo-sinking-for-perfect-loopnest,print<hir>" -hir-details -aa-pipeline="basic-aa" 2>&1 < %s | FileCheck %s
;
; This test checks updating the live-out for the blob symbase
;
;*** IR Dump Before HIR Sinking For Perfect Loopnest ***
;Function: main
;
;<0>          BEGIN REGION { }
;<28>               + LiveOut symbases:
;<28>               + Loop metadata: !llvm.loop !15
;<28>               + DO i64 i1 = 0, zext.i32.i64((-1 + (-1 * %.pr))), 1   <DO_LOOP>  <MAX_TC_EST = 75>
;<28>               | <RVAL-REG> LINEAR i64 zext.i32.i64((-1 + (-1 * %.pr))) {sb:2}
;<28>               |    <BLOB> LINEAR i32 %.pr {sb:4}
;<28>               |
;<29>               |   + LiveOut symbases: 11
;<29>               |   + DO i64 i2 = 0, 51, 1   <DO_LOOP>
;<9>                |   |   %xor = (@d)[0][92 * i1 + i2 + 92 * sext.i32.i64(%.pr) + 12]  ^  4;
;<9>                |   |   <LVAL-REG> NON-LINEAR i32 %xor {sb:11}
;<9>                |   |   <RVAL-REG> {al:4}(LINEAR [6864 x i32]* @d)[i64 0][LINEAR i64 92 * i1 + i2 + 92 * sext.i32.i64(%.pr) + 12] inbounds  !tbaa !8 {sb:20}
;<9>                |   |      <BLOB> LINEAR i32 %.pr {sb:4}
;<9>                |   |      <BLOB> LINEAR [6864 x i32]* @d {sb:9}
;<9>                |   |
;<10>               |   |   (@d)[0][92 * i1 + i2 + 92 * sext.i32.i64(%.pr) + 12] = %xor;
;<10>               |   |   <LVAL-REG> {al:4}(LINEAR [6864 x i32]* @d)[i64 0][LINEAR i64 92 * i1 + i2 + 92 * sext.i32.i64(%.pr) + 12] inbounds  !tbaa !8 {sb:20}
;<10>               |   |      <BLOB> LINEAR i32 %.pr {sb:4}
;<10>               |   |      <BLOB> LINEAR [6864 x i32]* @d {sb:9}
;<10>               |   |   <RVAL-REG> NON-LINEAR i32 %xor {sb:11}
;<10>               |   |
;<29>               |   + END LOOP
;<29>               |
;<20>               |   (@c)[0][i1 + sext.i32.i64(%.pr)] = %xor;
;<20>               |   <LVAL-REG> {al:8}(LINEAR [92 x i64]* @c)[i64 0][LINEAR i64 i1 + sext.i32.i64(%.pr)] inbounds  !tbaa !12 {sb:21}
;<20>               |      <BLOB> LINEAR [92 x i64]* @c {sb:16}
;<20>               |      <BLOB> LINEAR i32 %.pr {sb:4}
;<20>               |   <RVAL-REG> NON-LINEAR sext.i32.i64(%xor) {sb:2}
;<20>               |      <BLOB> NON-LINEAR i32 %xor {sb:11}
;<20>               |
;<28>               + END LOOP
;<0>          END REGION
;
;*** IR Dump After HIR Sinking For Perfect Loopnest ***
;Function: main
;
;<0>          BEGIN REGION { }
;<28>               + LiveOut symbases:
;<28>               + DO i64 i1 = 0, zext.i32.i64((-1 + (-1 * %.pr))), 1   <DO_LOOP>  <MAX_TC_EST = 75>
;<28>               | <RVAL-REG> LINEAR i64 zext.i32.i64((-1 + (-1 * %.pr))) {sb:2}
;<28>               |    <BLOB> LINEAR i32 %.pr {sb:4}
;<28>               |
;<29>               |   + LiveOut symbases:
;<29>               |   + DO i64 i2 = 0, 51, 1   <DO_LOOP>
;<9>                |   |   %xor = (@d)[0][92 * i1 + i2 + 92 * sext.i32.i64(%.pr) + 12]  ^  4;
;<9>                |   |   <LVAL-REG> NON-LINEAR i32 %xor {sb:11}
;<9>                |   |   <RVAL-REG> {al:4}(LINEAR [6864 x i32]* @d)[i64 0][LINEAR i64 92 * i1 + i2 + 92 * sext.i32.i64(%.pr) + 12] inbounds  !tbaa !8 {sb:20}
;<9>                |   |      <BLOB> LINEAR i32 %.pr {sb:4}
;<9>                |   |      <BLOB> LINEAR [6864 x i32]* @d {sb:9}
;<9>                |   |
;<10>               |   |   (@d)[0][92 * i1 + i2 + 92 * sext.i32.i64(%.pr) + 12] = %xor;
;<10>               |   |   <LVAL-REG> {al:4}(LINEAR [6864 x i32]* @d)[i64 0][LINEAR i64 92 * i1 + i2 + 92 * sext.i32.i64(%.pr) + 12] inbounds  !tbaa !8 {sb:20}
;<10>               |   |      <BLOB> LINEAR i32 %.pr {sb:4}
;<10>               |   |      <BLOB> LINEAR [6864 x i32]* @d {sb:9}
;<10>               |   |   <RVAL-REG> NON-LINEAR i32 %xor {sb:11}
;<10>               |   |
;<20>               |   |   (@c)[0][i1 + sext.i32.i64(%.pr)] = %xor;
;<20>               |   |   <LVAL-REG> {al:8}(LINEAR [92 x i64]* @c)[i64 0][LINEAR i64 i1 + sext.i32.i64(%.pr)] inbounds  !tbaa !12 {sb:21}
;<20>               |   |      <BLOB> LINEAR [92 x i64]* @c {sb:16}
;<20>               |   |      <BLOB> LINEAR i32 %.pr {sb:4}
;<20>               |   |   <RVAL-REG> NON-LINEAR sext.i32.i64(%xor) {sb:2}
;<20>               |   |      <BLOB> NON-LINEAR i32 %xor {sb:11}
;<20>               |   |
;<29>               |   + END LOOP
;<28>               + END LOOP
;<0>          END REGION
;
;*** IR Dump After HIR Undo Sinking For Perfect Loopnest ***
;Function: main
;
; CHECK:     BEGIN REGION { }
; CHECK:           + LiveOut symbases:
; CHECK:           + DO i64 i1 = 0, zext.i32.i64((-1 + (-1 * %.pr))), 1   <DO_LOOP>  <MAX_TC_EST = 75>
; CHECK:           | <RVAL-REG> LINEAR i64 zext.i32.i64((-1 + (-1 * %.pr))) {sb:2}
; CHECK:           |
; CHECK:           |   + LiveOut symbases: [[SB:.*]]
; CHECK:           |   + DO i64 i2 = 0, 51, 1   <DO_LOOP>
; CHECK:           |   |   %xor = (@d)[0][92 * i1 + i2 + 92 * sext.i32.i64(%.pr) + 12]  ^  4;
; CHECK:           |   |   <LVAL-REG> NON-LINEAR i32 %xor {sb:[[SB]]}
;                  |   |
; CHECK:           |   |   (@d)[0][92 * i1 + i2 + 92 * sext.i32.i64(%.pr) + 12] = %xor;
; CHECK:           |   |   <RVAL-REG> NON-LINEAR i32 %xor {sb:[[SB]]}
;                  |   |
; CHECK:           |   + END LOOP
; CHECK:           |      (@c)[0][i1 + sext.i32.i64(%.pr)] = %xor;
; CHECK:           |      <RVAL-REG> NON-LINEAR sext.i32.i64(%xor) {sb:[[SB2:.*]]}
; CHECK:           |         <BLOB> NON-LINEAR i32 %xor {sb:[[SB]]}
;                  |
; CHECK:           + END LOOP
; CHECK:     END REGION
;
;Module Before HIR
; ModuleID = 'atg_CMPLRLLVM-26564_cpp.cpp'
source_filename = "atg_CMPLRLLVM-26564_cpp.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@a = dso_local local_unnamed_addr global i32 0, align 4
@b = dso_local local_unnamed_addr global i32 0, align 4
@c = dso_local local_unnamed_addr global [92 x i64] zeroinitializer, align 16
@d = dso_local local_unnamed_addr global [6864 x i32] zeroinitializer, align 16

; Function Attrs: nofree norecurse nounwind uwtable mustprogress
define dso_local i32 @main() local_unnamed_addr #0 {
entry:
  %.pr = load i32, ptr @b, align 4, !tbaa !2
  %tobool.not9 = icmp eq i32 %.pr, 0
  br i1 %tobool.not9, label %for.end7, label %for.cond1.preheader.lr.ph

for.cond1.preheader.lr.ph:                        ; preds = %entry
  %0 = sext i32 %.pr to i64
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.cond1.preheader.lr.ph, %for.inc5
  %indvars.iv12 = phi i64 [ %0, %for.cond1.preheader.lr.ph ], [ %indvars.iv.next13, %for.inc5 ]
  %1 = mul nsw i64 %indvars.iv12, 92
  br label %for.body2

for.body2:                                        ; preds = %for.cond1.preheader, %for.body2
  %indvars.iv = phi i64 [ 12, %for.cond1.preheader ], [ %indvars.iv.next, %for.body2 ]
  %2 = add nsw i64 %1, %indvars.iv
  %arrayidx = getelementptr inbounds [6864 x i32], ptr @d, i64 0, i64 %2, !intel-tbaa !6
  %3 = load i32, ptr %arrayidx, align 4, !tbaa !6
  %xor = xor i32 %3, 4
  store i32 %xor, ptr %arrayidx, align 4, !tbaa !6
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 64
  br i1 %exitcond.not, label %for.inc5, label %for.body2, !llvm.loop !8

for.inc5:                                         ; preds = %for.body2
  %xor.lcssa = phi i32 [ %xor, %for.body2 ]
  %arrayidx4 = getelementptr inbounds [92 x i64], ptr @c, i64 0, i64 %indvars.iv12, !intel-tbaa !10
  %conv = sext i32 %xor.lcssa to i64
  store i64 %conv, ptr %arrayidx4, align 8, !tbaa !10
  %indvars.iv.next13 = add nsw i64 %indvars.iv12, 1
  %4 = trunc i64 %indvars.iv.next13 to i32
  %tobool.not = icmp eq i32 %4, 0
  br i1 %tobool.not, label %for.cond.for.end7_crit_edge, label %for.cond1.preheader, !llvm.loop !13

for.cond.for.end7_crit_edge:                      ; preds = %for.inc5
  store i32 64, ptr @a, align 4, !tbaa !2
  store i32 0, ptr @b, align 4, !tbaa !2
  br label %for.end7

for.end7:                                         ; preds = %for.cond.for.end7_crit_edge, %entry
  ret i32 0
}

attributes #0 = { nofree norecurse nounwind uwtable mustprogress "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"Intel(R) oneAPI DPC++ Compiler 2021.2.0 (YYYY.x.0.MMDD)"}
!2 = !{!3, !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C++ TBAA"}
!6 = !{!7, !3, i64 0}
!7 = !{!"array@_ZTSA6864_i", !3, i64 0}
!8 = distinct !{!8, !9}
!9 = !{!"llvm.loop.mustprogress"}
!10 = !{!11, !12, i64 0}
!11 = !{!"array@_ZTSA92_l", !12, i64 0}
!12 = !{!"long", !4, i64 0}
!13 = distinct !{!13, !9}
