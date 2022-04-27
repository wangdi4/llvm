; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -disable-output -hir-loop-fusion -hir-loop-fusion-skip-vec-prof-check -print-after=hir-loop-fusion < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-loop-fusion,print<hir>" -hir-loop-fusion-skip-vec-prof-check -aa-pipeline="basic-aa" -disable-output < %s 2>&1 | FileCheck %s

; Check that the loopnest is fused without assertions.
; A heap neighbor edge was not removed while collapsing.

; CHECK: DO i1
; CHECK:   DO i2
; CHECK:   DO i2
; CHECK:   DO i2
; CHECK-NOT: DO i2

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@global = common unnamed_addr global [4 x i8] zeroinitializer, align 32
@global.1 = common unnamed_addr global [4 x i8] zeroinitializer, align 32
@global.2 = common unnamed_addr global [4 x i8] zeroinitializer, align 32
@global.3 = common unnamed_addr global [4 x i8] zeroinitializer, align 32
@global.4 = common unnamed_addr global [44 x i8] zeroinitializer, align 32

; Function Attrs: nounwind readonly
define void @spam(i32* noalias nocapture %arg) local_unnamed_addr #0 {
bb:
  %tmp = load i32, i32* bitcast ([4 x i8]* @global to i32*), align 32
  %tmp1 = load i32, i32* bitcast ([4 x i8]* @global.2 to i32*), align 32
  %tmp2 = sext i32 %tmp to i64
  %tmp3 = icmp sgt i64 %tmp2, 0
  %tmp4 = select i1 %tmp3, i64 %tmp2, i64 0
  %tmp5 = sext i32 %tmp1 to i64
  %tmp6 = icmp sgt i64 %tmp5, 0
  %tmp7 = select i1 %tmp6, i64 %tmp5, i64 0
  %tmp8 = mul nuw nsw i64 %tmp4, 12
  %tmp9 = mul i64 %tmp8, %tmp7
  %tmp10 = lshr exact i64 %tmp9, 2
  %tmp11 = alloca float, i64 %tmp10, align 4
  %tmp12 = mul nuw nsw i64 %tmp4, 3
  %tmp13 = alloca float, i64 %tmp12, align 4
  %tmp14 = alloca float, i64 %tmp12, align 4
  %tmp15 = shl nsw i64 %tmp2, 2
  %tmp16 = mul nsw i64 %tmp15, %tmp5
  %tmp17 = icmp slt i32 %tmp, 1
  br i1 %tmp17, label %bb145, label %bb18

bb18:                                             ; preds = %bb
  %tmp19 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 %tmp15, i32* elementtype(i32) %arg, i64 2)
  %tmp20 = add nuw i32 %tmp, 1
  %tmp21 = zext i32 %tmp20 to i64
  br label %bb22

bb22:                                             ; preds = %bb141, %bb18
  %tmp23 = phi i64 [ 1, %bb18 ], [ %tmp142, %bb141 ]
  %tmp24 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* elementtype(i32) %tmp19, i64 %tmp23)
  %tmp25 = load i32, i32* %tmp24, align 4
  %tmp26 = sext i32 %tmp25 to i64
  %tmp27 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 0, i64 4, i32* elementtype(i32) bitcast ([44 x i8]* @global.4 to i32*), i64 %tmp26)
  %tmp28 = load i32, i32* %tmp27, align 4
  %tmp29 = icmp eq i32 %tmp28, 0
  br i1 %tmp29, label %bb141, label %bb30

bb30:                                             ; preds = %bb22
  br label %bb33

bb31:                                             ; preds = %bb33
  %tmp32 = phi float [ %tmp43, %bb33 ]
  br label %bb46

bb33:                                             ; preds = %bb33, %bb30
  %tmp34 = phi i64 [ %tmp44, %bb33 ], [ 1, %bb30 ]
  %tmp35 = phi float [ %tmp43, %bb33 ], [ 0.000000e+00, %bb30 ]
  %tmp36 = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 1, i64 1, i64 %tmp15, float* elementtype(float) nonnull %tmp13, i64 %tmp34)
  %tmp37 = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 1, i64 4, float* elementtype(float) nonnull %tmp36, i64 %tmp23)
  %tmp38 = load float, float* %tmp37, align 4
  %tmp39 = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 1, i64 1, i64 %tmp15, float* elementtype(float) nonnull %tmp14, i64 %tmp34)
  %tmp40 = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 1, i64 4, float* elementtype(float) nonnull %tmp39, i64 %tmp23)
  %tmp41 = load float, float* %tmp40, align 4
  %tmp42 = fmul float %tmp38, %tmp41
  %tmp43 = fadd float %tmp35, %tmp42
  %tmp44 = add nuw nsw i64 %tmp34, 1
  %tmp45 = icmp eq i64 %tmp44, 4
  br i1 %tmp45, label %bb31, label %bb33

bb46:                                             ; preds = %bb46, %bb31
  %tmp47 = phi i64 [ 1, %bb31 ], [ %tmp54, %bb46 ]
  %tmp48 = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 1, i64 1, i64 %tmp15, float* elementtype(float) nonnull %tmp14, i64 %tmp47)
  %tmp49 = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 1, i64 4, float* elementtype(float) nonnull %tmp48, i64 %tmp23)
  %tmp50 = load float, float* %tmp49, align 4
  %tmp51 = fmul float %tmp32, %tmp50
  %tmp52 = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 1, i64 1, i64 %tmp15, float* elementtype(float) nonnull %tmp13, i64 %tmp47)
  %tmp53 = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 1, i64 4, float* elementtype(float) nonnull %tmp52, i64 %tmp23)
  store float %tmp51, float* %tmp53, align 4
  %tmp54 = add nuw nsw i64 %tmp47, 1
  %tmp55 = icmp eq i64 %tmp54, 4
  br i1 %tmp55, label %bb56, label %bb46

bb56:                                             ; preds = %bb46
  br label %bb59

bb57:                                             ; preds = %bb59
  %tmp58 = phi float [ %tmp70, %bb59 ]
  br label %bb75

bb59:                                             ; preds = %bb59, %bb56
  %tmp60 = phi float [ %tmp70, %bb59 ], [ 0.000000e+00, %bb56 ]
  %tmp61 = phi i64 [ %tmp71, %bb59 ], [ 1, %bb56 ]
  %tmp62 = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 2, i64 1, i64 %tmp16, float* elementtype(float) nonnull %tmp11, i64 %tmp61)
  %tmp63 = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 1, i64 1, i64 %tmp15, float* elementtype(float) nonnull %tmp62, i64 1)
  %tmp64 = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 1, i64 4, float* elementtype(float) nonnull %tmp63, i64 %tmp23)
  %tmp65 = load float, float* %tmp64, align 4
  %tmp66 = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 1, i64 1, i64 %tmp15, float* elementtype(float) nonnull %tmp14, i64 %tmp61)
  %tmp67 = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 1, i64 4, float* elementtype(float) nonnull %tmp66, i64 %tmp23)
  %tmp68 = load float, float* %tmp67, align 4
  %tmp69 = fmul float %tmp65, %tmp68
  %tmp70 = fadd float %tmp60, %tmp69
  %tmp71 = add nuw nsw i64 %tmp61, 1
  %tmp72 = icmp eq i64 %tmp71, 4
  br i1 %tmp72, label %bb57, label %bb59

bb73:                                             ; preds = %bb75
  %tmp74 = phi float [ %tmp86, %bb75 ]
  br label %bb91

bb75:                                             ; preds = %bb75, %bb57
  %tmp76 = phi float [ 0.000000e+00, %bb57 ], [ %tmp86, %bb75 ]
  %tmp77 = phi i64 [ 1, %bb57 ], [ %tmp87, %bb75 ]
  %tmp78 = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 2, i64 1, i64 %tmp16, float* elementtype(float) nonnull %tmp11, i64 %tmp77)
  %tmp79 = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 1, i64 1, i64 %tmp15, float* elementtype(float) nonnull %tmp78, i64 2)
  %tmp80 = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 1, i64 4, float* elementtype(float) nonnull %tmp79, i64 %tmp23)
  %tmp81 = load float, float* %tmp80, align 4
  %tmp82 = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 1, i64 1, i64 %tmp15, float* elementtype(float) nonnull %tmp14, i64 %tmp77)
  %tmp83 = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 1, i64 4, float* elementtype(float) nonnull %tmp82, i64 %tmp23)
  %tmp84 = load float, float* %tmp83, align 4
  %tmp85 = fmul float %tmp81, %tmp84
  %tmp86 = fadd float %tmp76, %tmp85
  %tmp87 = add nuw nsw i64 %tmp77, 1
  %tmp88 = icmp eq i64 %tmp87, 4
  br i1 %tmp88, label %bb73, label %bb75

bb89:                                             ; preds = %bb91
  %tmp90 = phi float [ %tmp102, %bb91 ]
  br label %bb105

bb91:                                             ; preds = %bb91, %bb73
  %tmp92 = phi float [ 0.000000e+00, %bb73 ], [ %tmp102, %bb91 ]
  %tmp93 = phi i64 [ 1, %bb73 ], [ %tmp103, %bb91 ]
  %tmp94 = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 2, i64 1, i64 %tmp16, float* elementtype(float) nonnull %tmp11, i64 %tmp93)
  %tmp95 = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 1, i64 1, i64 %tmp15, float* elementtype(float) nonnull %tmp94, i64 3)
  %tmp96 = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 1, i64 4, float* elementtype(float) nonnull %tmp95, i64 %tmp23)
  %tmp97 = load float, float* %tmp96, align 4
  %tmp98 = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 1, i64 1, i64 %tmp15, float* elementtype(float) nonnull %tmp14, i64 %tmp93)
  %tmp99 = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 1, i64 4, float* elementtype(float) nonnull %tmp98, i64 %tmp23)
  %tmp100 = load float, float* %tmp99, align 4
  %tmp101 = fmul float %tmp97, %tmp100
  %tmp102 = fadd float %tmp92, %tmp101
  %tmp103 = add nuw nsw i64 %tmp93, 1
  %tmp104 = icmp eq i64 %tmp103, 4
  br i1 %tmp104, label %bb89, label %bb91

bb105:                                            ; preds = %bb105, %bb89
  %tmp106 = phi i64 [ 1, %bb89 ], [ %tmp114, %bb105 ]
  %tmp107 = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 1, i64 1, i64 %tmp15, float* elementtype(float) nonnull %tmp14, i64 %tmp106)
  %tmp108 = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 1, i64 4, float* elementtype(float) nonnull %tmp107, i64 %tmp23)
  %tmp109 = load float, float* %tmp108, align 4
  %tmp110 = fmul float %tmp58, %tmp109
  %tmp111 = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 2, i64 1, i64 %tmp16, float* elementtype(float) nonnull %tmp11, i64 %tmp106)
  %tmp112 = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 1, i64 1, i64 %tmp15, float* elementtype(float) nonnull %tmp111, i64 1)
  %tmp113 = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 1, i64 4, float* elementtype(float) nonnull %tmp112, i64 %tmp23)
  store float %tmp110, float* %tmp113, align 4
  %tmp114 = add nuw nsw i64 %tmp106, 1
  %tmp115 = icmp eq i64 %tmp114, 4
  br i1 %tmp115, label %bb116, label %bb105

bb116:                                            ; preds = %bb105
  br label %bb117

bb117:                                            ; preds = %bb117, %bb116
  %tmp118 = phi i64 [ %tmp126, %bb117 ], [ 1, %bb116 ]
  %tmp119 = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 1, i64 1, i64 %tmp15, float* elementtype(float) nonnull %tmp14, i64 %tmp118)
  %tmp120 = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 1, i64 4, float* elementtype(float) nonnull %tmp119, i64 %tmp23)
  %tmp121 = load float, float* %tmp120, align 4
  %tmp122 = fmul float %tmp74, %tmp121
  %tmp123 = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 2, i64 1, i64 %tmp16, float* elementtype(float) nonnull %tmp11, i64 %tmp118)
  %tmp124 = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 1, i64 1, i64 %tmp15, float* elementtype(float) nonnull %tmp123, i64 2)
  %tmp125 = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 1, i64 4, float* elementtype(float) nonnull %tmp124, i64 %tmp23)
  store float %tmp122, float* %tmp125, align 4
  %tmp126 = add nuw nsw i64 %tmp118, 1
  %tmp127 = icmp eq i64 %tmp126, 4
  br i1 %tmp127, label %bb128, label %bb117

bb128:                                            ; preds = %bb117
  br label %bb129

bb129:                                            ; preds = %bb129, %bb128
  %tmp130 = phi i64 [ %tmp138, %bb129 ], [ 1, %bb128 ]
  %tmp131 = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 1, i64 1, i64 %tmp15, float* elementtype(float) nonnull %tmp14, i64 %tmp130)
  %tmp132 = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 1, i64 4, float* elementtype(float) nonnull %tmp131, i64 %tmp23)
  %tmp133 = load float, float* %tmp132, align 4
  %tmp134 = fmul float %tmp90, %tmp133
  %tmp135 = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 2, i64 1, i64 %tmp16, float* elementtype(float) nonnull %tmp11, i64 %tmp130)
  %tmp136 = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 1, i64 1, i64 %tmp15, float* elementtype(float) nonnull %tmp135, i64 3)
  %tmp137 = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 1, i64 4, float* elementtype(float) nonnull %tmp136, i64 %tmp23)
  store float %tmp134, float* %tmp137, align 4
  %tmp138 = add nuw nsw i64 %tmp130, 1
  %tmp139 = icmp eq i64 %tmp138, 4
  br i1 %tmp139, label %bb140, label %bb129

bb140:                                            ; preds = %bb129
  br label %bb141

bb141:                                            ; preds = %bb140, %bb22
  %tmp142 = add nuw nsw i64 %tmp23, 1
  %tmp143 = icmp eq i64 %tmp142, %tmp21
  br i1 %tmp143, label %bb144, label %bb22

bb144:                                            ; preds = %bb141
  br label %bb145

bb145:                                            ; preds = %bb144, %bb
  ret void
}

; Function Attrs: nounwind readnone speculatable
declare i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8, i64, i64, i32*, i64) #1

; Function Attrs: nounwind readnone speculatable
declare float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8, i64, i64, float*, i64) #1

attributes #0 = { nounwind readonly "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="corei7-avx" "target-features"="+avx,+cx16,+cx8,+fxsr,+mmx,+pclmul,+popcnt,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" }
attributes #1 = { nounwind readnone speculatable }

