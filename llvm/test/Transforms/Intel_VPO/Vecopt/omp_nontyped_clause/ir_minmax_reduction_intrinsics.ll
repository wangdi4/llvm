; Test to check correctness of VPlan LLVM-IR vectorizer legality and codegen for
; explicit min/max reductions that use llvm.minnum/maxnum intrinsics.

; RUN: opt -S -vplan-print-after-vpentity-instrs -vplan-entities-dump -vplan-vec -vplan-force-vf=4 %s | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define float @foo(float* nocapture readonly %A, i32 %N) {
; CHECK-LABEL:  VPlan after insertion of VPEntities instructions:
; CHECK-NEXT:  VPlan IR for: foo:for.body
; CHECK-NEXT:  Loop Entities of the loop with header [[BB0:BB[0-9]+]]
; CHECK-EMPTY:
; CHECK-NEXT:  Reduction list
; CHECK-NEXT:   (FloatMax) Start: float [[TMP0:%.*]] Exit: float [[VP__MAX_0:%.*]]
; CHECK-NEXT:    Linked values: float [[VP_MAX_014:%.*]], float [[VP__MAX_0]], float* [[VP_RED:%.*]], float [[VP_REDMINMAX_RED_INIT:%.*]], void [[VP_STORE:%.*]], float [[VP_REDMINMAX_RED_FINAL:%.*]],
; CHECK-NEXT:   Memory: float* [[RED0:%.*]]

; CHECK:        [[VPBB_PH:BB[0-9]+]]: # preds: {{BB[0-9]+}}
; CHECK-NEXT:     float* [[VP_RED]] = allocate-priv float*, OrigAlign = 4
; CHECK-NEXT:     i8* [[VP_RED_BCAST:%.*]] = bitcast float* [[VP_RED]]
; CHECK-NEXT:     call i64 4 i8* [[VP_RED_BCAST]] void (i64, i8*)* @llvm.lifetime.start.p0i8
; CHECK-NEXT:     float [[VP_REDMINMAX_RED_INIT]] = reduction-init float %0
; CHECK-NEXT:     store float [[VP_REDMINMAX_RED_INIT]] float* [[VP_RED]]

; CHECK:        [[VPBB_HEADER:BB[0-9]+]]: # preds: [[VPBB_PH]], [[VPBB_HEADER]]
; CHECK:          float [[VP_MAX_014]] = phi [ float [[VP__MAX_0]], [[VPBB_HEADER]] ], [ float [[VP_REDMINMAX_RED_INIT]], [[VPBB_PH]] ]
; CHECK:          float [[VP__MAX_0]] = call float [[VP_LOAD:%vp.*]] float [[VP_MAX_014]] float (float, float)* @llvm.maxnum.f32

; CHECK:        [[VPBB_EXIT:BB[0-9]+]]: # preds: [[VPBB_HEADER]]
; CHECK-NEXT:     float [[VP_REDMINMAX_RED_FINAL]] = reduction-final{fmax} float [[VP__MAX_0]]
; CHECK-NEXT:     store float [[VP_REDMINMAX_RED_FINAL]] float* [[RED0]]


; Checks for generated LLVM-IR
; CHECK-LABEL: @foo(
; CHECK:  VPlannedBB1:
; CHECK:    [[BROADCAST_SPLATINSERT0:%.*]] = insertelement <4 x float> poison, float [[TMP0:%.*]], i32 0
; CHECK:    [[BROADCAST_SPLAT0:%.*]] = shufflevector <4 x float> [[BROADCAST_SPLATINSERT0]], <4 x float> poison, <4 x i32> zeroinitializer

; CHECK:  vector.body:
; CHECK:    [[VEC_PHI50:%.*]] = phi <4 x float> [ [[TMP6:%.*]], [[VECTOR_BODY0:%.*]] ], [ [[BROADCAST_SPLAT0]], [[VPLANNEDBB20:%.*]] ]
; CHECK:    [[TMP6]] = call <4 x float> @llvm.maxnum.v4f32(<4 x float> [[WIDE_LOAD0I:%.*]], <4 x float> [[VEC_PHI50]])

; CHECK:  VPlannedBB6:
; CHECK:    [[TMP11:%.*]] = call float @llvm.vector.reduce.fmax.v4f32(<4 x float> [[TMP6]])
; CHECK:    store float [[TMP11]], float* [[RED0]], align 1

; CHECK:  final.merge:
; CHECK:    [[UNI_PHI120:%.*]] = phi float [ [[DOTMAX_00:%.*]], [[VPLANNEDBB110:%.*]] ], [ [[TMP11]], [[VPLANNEDBB80:%.*]] ]
;
entry:
  %red = alloca float, align 4
  br label %begin.simd

begin.simd:
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.REDUCTION.MAX"(float* %red) ]
  br label %DIR.QUAL.LIST.END.2

DIR.QUAL.LIST.END.2:
  %0 = load float, float* %red, align 4
  %cmp13 = icmp sgt i32 %N, 1
  br i1 %cmp13, label %for.body.ph, label %for.cond.cleanup

for.body.ph:                                 ; preds = %0
  %wide.trip.count = zext i32 %N to i64
  br label %for.body

for.body:                                           ; preds = %for.body.ph, %for.body
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body ], [ 1, %for.body.ph ]
  %Max.014 = phi float [ %.Max.0, %for.body ], [ %0, %for.body.ph ]
  %arrayidx1 = getelementptr inbounds float, float* %A, i64 %indvars.iv
  %1 = load float, float* %arrayidx1, align 4
  %.Max.0 = call float @llvm.maxnum.f32(float %1, float %Max.014)
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.cond.cleanup.loopexit, label %for.body

for.cond.cleanup.loopexit:                             ; preds = %for.body
  %.Max.0.lcssa = phi float [ %.Max.0, %for.body ]
  br label %for.cond.cleanup

for.cond.cleanup:                                      ; preds = %for.cond.cleanup.loopexit, %0
  %Max.0.lcssa = phi float [ %0, %DIR.QUAL.LIST.END.2 ], [ %.Max.0.lcssa, %for.cond.cleanup.loopexit ]
  br label %end.simd

end.simd:
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.QUAL.LIST.END.3

DIR.QUAL.LIST.END.3:
  ret float %Max.0.lcssa
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

; Function Attrs: nofree nosync nounwind readnone speculatable willreturn
declare float @llvm.maxnum.f32(float, float) #1

attributes #1 = { nofree nosync nounwind readnone speculatable willreturn }
