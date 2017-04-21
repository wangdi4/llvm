;  Test for Interchange
;  No Interchange expected when permutation is not met due to dependence 
;  and Nearby permutation is the same as original order   
;  
; REQUIRES: asserts 
; RUN: opt -hir-ssa-deconstruction -debug  -hir-loop-interchange  < %s 2>&1 | FileCheck %s
; CHECK-NOT: Interchanged:
;
;Module Before HIR; ModuleID = 'x.c'
source_filename = "x.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@matrix4x4_check = local_unnamed_addr global [6 x i32] zeroinitializer, align 16
@matrix8x8_check = local_unnamed_addr global [2 x i32] zeroinitializer, align 4
@present = common local_unnamed_addr global [6 x i32] zeroinitializer, align 16
@UseDefaultScalingMatrix4x4Flag = common local_unnamed_addr global [6 x i16] zeroinitializer, align 2
@quant_coef = external local_unnamed_addr constant [6 x [4 x [4 x i32]]], align 16
@Quant_intra_default = internal unnamed_addr constant [16 x i16] [i16 6, i16 13, i16 20, i16 28, i16 13, i16 20, i16 28, i16 32, i16 20, i16 28, i16 32, i16 37, i16 28, i16 32, i16 37, i16 42], align 16
@LevelScale4x4Luma_Intra = common local_unnamed_addr global [6 x [4 x [4 x i32]]] zeroinitializer, align 16
@dequant_coef = external local_unnamed_addr constant [6 x [4 x [4 x i32]]], align 16
@InvLevelScale4x4Luma_Intra = common local_unnamed_addr global [6 x [4 x [4 x i32]]] zeroinitializer, align 16
@ScalingList4x4 = common local_unnamed_addr global [6 x [16 x i16]] zeroinitializer, align 16
@LevelScale4x4Chroma_Intra = common local_unnamed_addr global [2 x [6 x [4 x [4 x i32]]]] zeroinitializer, align 16
@InvLevelScale4x4Chroma_Intra = common local_unnamed_addr global [2 x [6 x [4 x [4 x i32]]]] zeroinitializer, align 16
@Quant_inter_default = internal unnamed_addr constant [16 x i16] [i16 10, i16 14, i16 20, i16 24, i16 14, i16 20, i16 24, i16 27, i16 20, i16 24, i16 27, i16 30, i16 24, i16 27, i16 30, i16 34], align 16
@LevelScale4x4Luma_Inter = common local_unnamed_addr global [6 x [4 x [4 x i32]]] zeroinitializer, align 16
@InvLevelScale4x4Luma_Inter = common local_unnamed_addr global [6 x [4 x [4 x i32]]] zeroinitializer, align 16
@LevelScale4x4Chroma_Inter = common local_unnamed_addr global [2 x [6 x [4 x [4 x i32]]]] zeroinitializer, align 16
@InvLevelScale4x4Chroma_Inter = common local_unnamed_addr global [2 x [6 x [4 x [4 x i32]]]] zeroinitializer, align 16
@M = common local_unnamed_addr global [10 x i32] zeroinitializer, align 16
@LevelScale8x8Luma_Intra = common local_unnamed_addr global [6 x [8 x [8 x i32]]] zeroinitializer, align 16
@LevelScale8x8Luma_Inter = common local_unnamed_addr global [6 x [8 x [8 x i32]]] zeroinitializer, align 16
@InvLevelScale8x8Luma_Intra = common local_unnamed_addr global [6 x [8 x [8 x i32]]] zeroinitializer, align 16
@InvLevelScale8x8Luma_Inter = common local_unnamed_addr global [6 x [8 x [8 x i32]]] zeroinitializer, align 16
@ScalingList4x4input = common local_unnamed_addr global [6 x [16 x i16]] zeroinitializer, align 16
@ScalingList8x8input = common local_unnamed_addr global [2 x [64 x i16]] zeroinitializer, align 16
@ScalingList8x8 = common local_unnamed_addr global [2 x [64 x i16]] zeroinitializer, align 16
@UseDefaultScalingMatrix8x8Flag = common local_unnamed_addr global [2 x i16] zeroinitializer, align 2
@no_q_matrix = common local_unnamed_addr global i32 0, align 4

; Function Attrs: norecurse nounwind uwtable
define void @CalculateQuantParam() local_unnamed_addr #0 {
entry:
  %0 = load i32, i32* getelementptr inbounds ([6 x i32], [6 x i32]* @present, i64 0, i64 0), align 16, !tbaa !1
  %tobool = icmp ne i32 %0, 0
  %1 = load i16, i16* getelementptr inbounds ([6 x i16], [6 x i16]* @UseDefaultScalingMatrix4x4Flag, i64 0, i64 0), align 2
  %tobool7 = icmp eq i16 %1, 0
  %or.cond = and i1 %tobool, %tobool7
  %2 = load i32, i32* getelementptr inbounds ([6 x i32], [6 x i32]* @present, i64 0, i64 1), align 4, !tbaa !1
  %tobool70 = icmp eq i32 %2, 0
  %3 = load i32, i32* getelementptr inbounds ([6 x i32], [6 x i32]* @present, i64 0, i64 2), align 8, !tbaa !1
  %tobool145 = icmp eq i32 %3, 0
  %4 = load i32, i32* getelementptr inbounds ([6 x i32], [6 x i32]* @present, i64 0, i64 3), align 4, !tbaa !1
  %tobool224 = icmp ne i32 %4, 0
  %5 = load i16, i16* getelementptr inbounds ([6 x i16], [6 x i16]* @UseDefaultScalingMatrix4x4Flag, i64 0, i64 3), align 2
  %tobool227 = icmp eq i16 %5, 0
  %or.cond651 = and i1 %tobool224, %tobool227
  %6 = load i32, i32* getelementptr inbounds ([6 x i32], [6 x i32]* @present, i64 0, i64 4), align 16, !tbaa !1
  %tobool297 = icmp eq i32 %6, 0
  %7 = load i32, i32* getelementptr inbounds ([6 x i32], [6 x i32]* @present, i64 0, i64 5), align 4, !tbaa !1
  %tobool376 = icmp eq i32 %7, 0
  %8 = load i16, i16* getelementptr inbounds ([6 x i16], [6 x i16]* @UseDefaultScalingMatrix4x4Flag, i64 0, i64 5), align 2
  %tobool411 = icmp ne i16 %8, 0
  %9 = select i1 %tobool411, [16 x i16]* @Quant_inter_default, [16 x i16]* getelementptr inbounds ([6 x [16 x i16]], [6 x [16 x i16]]* @ScalingList4x4, i64 0, i64 5)
  %10 = load i16, i16* getelementptr inbounds ([6 x i16], [6 x i16]* @UseDefaultScalingMatrix4x4Flag, i64 0, i64 4), align 2
  %tobool332 = icmp ne i16 %10, 0
  %11 = select i1 %tobool332, [16 x i16]* @Quant_inter_default, [16 x i16]* getelementptr inbounds ([6 x [16 x i16]], [6 x [16 x i16]]* @ScalingList4x4, i64 0, i64 4)
  %12 = load i16, i16* getelementptr inbounds ([6 x i16], [6 x i16]* @UseDefaultScalingMatrix4x4Flag, i64 0, i64 2), align 2
  %tobool180 = icmp ne i16 %12, 0
  %13 = select i1 %tobool180, [16 x i16]* @Quant_intra_default, [16 x i16]* getelementptr inbounds ([6 x [16 x i16]], [6 x [16 x i16]]* @ScalingList4x4, i64 0, i64 2)
  %14 = load i16, i16* getelementptr inbounds ([6 x i16], [6 x i16]* @UseDefaultScalingMatrix4x4Flag, i64 0, i64 1), align 2
  %tobool105 = icmp ne i16 %14, 0
  %15 = select i1 %tobool105, [16 x i16]* @Quant_intra_default, [16 x i16]* getelementptr inbounds ([6 x [16 x i16]], [6 x [16 x i16]]* @ScalingList4x4, i64 0, i64 1)
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.inc462, %entry
  %indvars.iv663 = phi i64 [ 0, %entry ], [ %indvars.iv.next664, %for.inc462 ]
  br label %for.cond4.preheader

for.cond4.preheader:                              ; preds = %for.inc459, %for.cond1.preheader
  %indvars.iv660 = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next661, %for.inc459 ]
  br label %for.body6

for.body6:                                        ; preds = %if.end458, %for.cond4.preheader
  %indvars.iv = phi i64 [ 0, %for.cond4.preheader ], [ %indvars.iv.next, %if.end458 ]
  %16 = shl i64 %indvars.iv, 2
  %17 = add nuw nsw i64 %16, %indvars.iv660
  %arrayidx42 = getelementptr inbounds [6 x [4 x [4 x i32]]], [6 x [4 x [4 x i32]]]* @quant_coef, i64 0, i64 %indvars.iv663, i64 %indvars.iv660, i64 %indvars.iv
  %18 = load i32, i32* %arrayidx42, align 4, !tbaa !6
  %shl43 = shl i32 %18, 4
  %arrayidx45 = getelementptr inbounds [6 x [16 x i16]], [6 x [16 x i16]]* @ScalingList4x4, i64 0, i64 0, i64 %17
  %arrayidx14 = getelementptr inbounds [16 x i16], [16 x i16]* @Quant_intra_default, i64 0, i64 %17
  %19 = select i1 %or.cond, i16* %arrayidx45, i16* %arrayidx14
  %20 = load i16, i16* %19, align 2, !tbaa !10
  %conv46 = sext i16 %20 to i32
  %div47 = sdiv i32 %shl43, %conv46
  %arrayidx53 = getelementptr inbounds [6 x [4 x [4 x i32]]], [6 x [4 x [4 x i32]]]* @LevelScale4x4Luma_Intra, i64 0, i64 %indvars.iv663, i64 %indvars.iv660, i64 %indvars.iv
  store i32 %div47, i32* %arrayidx53, align 4, !tbaa !6
  %arrayidx59 = getelementptr inbounds [6 x [4 x [4 x i32]]], [6 x [4 x [4 x i32]]]* @dequant_coef, i64 0, i64 %indvars.iv663, i64 %indvars.iv660, i64 %indvars.iv
  %21 = load i32, i32* %arrayidx59, align 4, !tbaa !6
  %mul63 = mul nsw i32 %21, %conv46
  %arrayidx69 = getelementptr inbounds [6 x [4 x [4 x i32]]], [6 x [4 x [4 x i32]]]* @InvLevelScale4x4Luma_Intra, i64 0, i64 %indvars.iv663, i64 %indvars.iv660, i64 %indvars.iv
  store i32 %mul63, i32* %arrayidx69, align 4, !tbaa !6
  br i1 %tobool70, label %if.end144, label %if.else96

if.else96:                                        ; preds = %for.body6
  %arrayidx110 = getelementptr inbounds [16 x i16], [16 x i16]* %15, i64 0, i64 %17
  %22 = load i16, i16* %arrayidx110, align 2, !tbaa !10
  %conv111 = sext i16 %22 to i32
  %div112 = sdiv i32 %shl43, %conv111
  %mul137 = mul nsw i32 %21, %conv111
  br label %if.end144

if.end144:                                        ; preds = %for.body6, %if.else96
  %23 = phi i32 [ %div112, %if.else96 ], [ %div47, %for.body6 ]
  %24 = phi i32 [ %mul137, %if.else96 ], [ %mul63, %for.body6 ]
  %25 = getelementptr inbounds [2 x [6 x [4 x [4 x i32]]]], [2 x [6 x [4 x [4 x i32]]]]* @LevelScale4x4Chroma_Intra, i64 0, i64 0, i64 %indvars.iv663, i64 %indvars.iv660, i64 %indvars.iv
  store i32 %23, i32* %25, align 4
  %arrayidx143 = getelementptr inbounds [2 x [6 x [4 x [4 x i32]]]], [2 x [6 x [4 x [4 x i32]]]]* @InvLevelScale4x4Chroma_Intra, i64 0, i64 0, i64 %indvars.iv663, i64 %indvars.iv660, i64 %indvars.iv
  store i32 %24, i32* %arrayidx143, align 4, !tbaa !12
  br i1 %tobool145, label %if.end223, label %if.else171

if.else171:                                       ; preds = %if.end144
  %arrayidx187 = getelementptr inbounds [16 x i16], [16 x i16]* %13, i64 0, i64 %17
  %26 = load i16, i16* %arrayidx187, align 2, !tbaa !10
  %conv188 = sext i16 %26 to i32
  %div191 = sdiv i32 %shl43, %conv188
  %mul216 = mul nsw i32 %21, %conv188
  br label %if.end223

if.end223:                                        ; preds = %if.end144, %if.else171
  %.sink666 = phi i32 [ %div191, %if.else171 ], [ %23, %if.end144 ]
  %mul216.sink = phi i32 [ %mul216, %if.else171 ], [ %24, %if.end144 ]
  %27 = getelementptr inbounds [2 x [6 x [4 x [4 x i32]]]], [2 x [6 x [4 x [4 x i32]]]]* @LevelScale4x4Chroma_Intra, i64 0, i64 1, i64 %indvars.iv663, i64 %indvars.iv660, i64 %indvars.iv
  store i32 %.sink666, i32* %27, align 4
  %arrayidx222 = getelementptr inbounds [2 x [6 x [4 x [4 x i32]]]], [2 x [6 x [4 x [4 x i32]]]]* @InvLevelScale4x4Chroma_Intra, i64 0, i64 1, i64 %indvars.iv663, i64 %indvars.iv660, i64 %indvars.iv
  store i32 %mul216.sink, i32* %arrayidx222, align 4, !tbaa !12
  %arrayidx271 = getelementptr inbounds [6 x [16 x i16]], [6 x [16 x i16]]* @ScalingList4x4, i64 0, i64 3, i64 %17
  %arrayidx237 = getelementptr inbounds [16 x i16], [16 x i16]* @Quant_inter_default, i64 0, i64 %17
  %28 = select i1 %or.cond651, i16* %arrayidx271, i16* %arrayidx237
  %29 = load i16, i16* %28, align 2, !tbaa !10
  %conv272 = sext i16 %29 to i32
  %div273 = sdiv i32 %shl43, %conv272
  %arrayidx279 = getelementptr inbounds [6 x [4 x [4 x i32]]], [6 x [4 x [4 x i32]]]* @LevelScale4x4Luma_Inter, i64 0, i64 %indvars.iv663, i64 %indvars.iv660, i64 %indvars.iv
  store i32 %div273, i32* %arrayidx279, align 4, !tbaa !6
  %mul289 = mul nsw i32 %21, %conv272
  %arrayidx295 = getelementptr inbounds [6 x [4 x [4 x i32]]], [6 x [4 x [4 x i32]]]* @InvLevelScale4x4Luma_Inter, i64 0, i64 %indvars.iv663, i64 %indvars.iv660, i64 %indvars.iv
  store i32 %mul289, i32* %arrayidx295, align 4, !tbaa !6
  br i1 %tobool297, label %if.end375, label %if.else323

if.else323:                                       ; preds = %if.end223
  %arrayidx339 = getelementptr inbounds [16 x i16], [16 x i16]* %11, i64 0, i64 %17
  %30 = load i16, i16* %arrayidx339, align 2, !tbaa !10
  %conv340 = sext i16 %30 to i32
  %div343 = sdiv i32 %shl43, %conv340
  %mul368 = mul nsw i32 %21, %conv340
  br label %if.end375

if.end375:                                        ; preds = %if.end223, %if.else323
  %31 = phi i32 [ %div343, %if.else323 ], [ %div273, %if.end223 ]
  %32 = phi i32 [ %mul368, %if.else323 ], [ %mul289, %if.end223 ]
  %33 = getelementptr inbounds [2 x [6 x [4 x [4 x i32]]]], [2 x [6 x [4 x [4 x i32]]]]* @LevelScale4x4Chroma_Inter, i64 0, i64 0, i64 %indvars.iv663, i64 %indvars.iv660, i64 %indvars.iv
  store i32 %31, i32* %33, align 4
  %arrayidx374 = getelementptr inbounds [2 x [6 x [4 x [4 x i32]]]], [2 x [6 x [4 x [4 x i32]]]]* @InvLevelScale4x4Chroma_Inter, i64 0, i64 0, i64 %indvars.iv663, i64 %indvars.iv660, i64 %indvars.iv
  store i32 %32, i32* %arrayidx374, align 4, !tbaa !12
  br i1 %tobool376, label %if.end458, label %if.else402

if.else402:                                       ; preds = %if.end375
  %arrayidx418 = getelementptr inbounds [16 x i16], [16 x i16]* %9, i64 0, i64 %17
  %34 = load i16, i16* %arrayidx418, align 2, !tbaa !10
  %conv419 = sext i16 %34 to i32
  %div422 = sdiv i32 %shl43, %conv419
  %mul447 = mul nsw i32 %21, %conv419
  %arrayidx449 = getelementptr inbounds [10 x i32], [10 x i32]* @M, i64 0, i64 %indvars.iv
  %35 = load i32, i32* %arrayidx449, align 4, !tbaa !14
  %idxprom450 = sext i32 %35 to i64
  br label %if.end458

if.end458:                                        ; preds = %if.end375, %if.else402
  %.sink668 = phi i32 [ %div422, %if.else402 ], [ %31, %if.end375 ]
  %36 = phi i64 [ %idxprom450, %if.else402 ], [ 1, %if.end375 ]
  %mul447.sink = phi i32 [ %mul447, %if.else402 ], [ %32, %if.end375 ]
  %37 = getelementptr inbounds [2 x [6 x [4 x [4 x i32]]]], [2 x [6 x [4 x [4 x i32]]]]* @LevelScale4x4Chroma_Inter, i64 0, i64 1, i64 %indvars.iv663, i64 %indvars.iv660, i64 %indvars.iv
  store i32 %.sink668, i32* %37, align 4
  %arrayidx457 = getelementptr inbounds [2 x [6 x [4 x [4 x i32]]]], [2 x [6 x [4 x [4 x i32]]]]* @InvLevelScale4x4Chroma_Inter, i64 0, i64 %36, i64 %indvars.iv663, i64 %indvars.iv660, i64 %indvars.iv
  store i32 %mul447.sink, i32* %arrayidx457, align 4, !tbaa !12
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 4
  br i1 %exitcond, label %for.inc459, label %for.body6

for.inc459:                                       ; preds = %if.end458
  %indvars.iv.next661 = add nuw nsw i64 %indvars.iv660, 1
  %exitcond662 = icmp eq i64 %indvars.iv.next661, 4
  br i1 %exitcond662, label %for.inc462, label %for.cond4.preheader

for.inc462:                                       ; preds = %for.inc459
  %indvars.iv.next664 = add nuw nsw i64 %indvars.iv663, 1
  %exitcond665 = icmp eq i64 %indvars.iv.next664, 6
  br i1 %exitcond665, label %for.end464, label %for.cond1.preheader

for.end464:                                       ; preds = %for.inc462
  ret void
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 4.0.0 (trunk 21010) (llvm/branches/loopopt 21118)"}
!1 = !{!2, !3, i64 0}
!2 = !{!"array@_ZTSA6_i", !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
!6 = !{!7, !3, i64 0}
!7 = !{!"array@_ZTSA6_A4_A4_i", !8, i64 0}
!8 = !{!"array@_ZTSA4_A4_i", !9, i64 0}
!9 = !{!"array@_ZTSA4_i", !3, i64 0}
!10 = !{!11, !11, i64 0}
!11 = !{!"short", !4, i64 0}
!12 = !{!13, !3, i64 0}
!13 = !{!"array@_ZTSA2_A6_A4_A4_i", !7, i64 0}
!14 = !{!15, !3, i64 0}
!15 = !{!"array@_ZTSA10_i", !3, i64 0}

