; Test to check correctness of VPlan LLVM-IR vectorizer legality and codegen for
; explicit min/max reductions that use llvm.minimum/maximum intrinsics.

; RUN: opt -S -vplan-print-after-vpentity-instrs -vplan-entities-dump -passes=vplan-vec -vplan-force-vf=4 %s | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define float @foo(float* %A) {
; CHECK-LABEL:  VPlan after insertion of VPEntities instructions:
; CHECK-NEXT:  VPlan IR for: foo:for.body
; CHECK-NEXT:  Loop Entities of the loop with header [[BB0:BB[0-9]+]]
; CHECK-EMPTY:
; CHECK-NEXT:  Reduction list
; CHECK-NEXT:   (FloatMaximum) Start: float [[TMP0:%.*]] Exit: float [[VP__MAX_0:%.*]]
; CHECK-NEXT:    Linked values: float [[VP_MAX_014:%.*]], float [[VP__MAX_0]], float [[VP_REDMINMAX_RED_INIT:%.*]], float [[VP_REDMINMAX_RED_FINAL:%.*]],

; CHECK:        [[VPBB_PH:BB[0-9]+]]: # preds: {{BB[0-9]+}}
; CHECK-NEXT:     float [[VP_REDMINMAX_RED_INIT]] = reduction-init float %0

; CHECK:        [[VPBB_HEADER:BB[0-9]+]]: # preds: [[VPBB_PH]], [[VPBB_HEADER]]
; CHECK:          float [[VP_MAX_014]] = phi [ float [[VP__MAX_0]], [[VPBB_HEADER]] ], [ float [[VP_REDMINMAX_RED_INIT]], [[VPBB_PH]] ]
; CHECK:          float [[VP__MAX_0]] = call float [[VP_LOAD:%vp.*]] float [[VP_MAX_014]] float (float, float)* @llvm.maximum.f32

; CHECK:        [[VPBB_EXIT:BB[0-9]+]]: # preds: [[VPBB_HEADER]]
; CHECK-NEXT:     float [[VP_REDMINMAX_RED_FINAL]] = reduction-final{fmaximum} float [[VP__MAX_0]]


; Checks for generated LLVM-IR
; CHECK-LABEL: @foo(
; CHECK:  VPlannedBB:
; CHECK:    [[BROADCAST_SPLATINSERT0:%.*]] = insertelement <4 x float> poison, float [[TMP0:%.*]], i64 0
; CHECK:    [[BROADCAST_SPLAT0:%.*]] = shufflevector <4 x float> [[BROADCAST_SPLATINSERT0]], <4 x float> poison, <4 x i32> zeroinitializer

; CHECK:  vector.body:
; CHECK:    [[VEC_PHI50:%.*]] = phi <4 x float> [ [[TMP6:%.*]], [[VECTOR_BODY0:%.*]] ], [ [[BROADCAST_SPLAT0]], [[VPLANNEDBB20:%.*]] ]
; CHECK:    [[TMP6]] = call <4 x float> @llvm.maximum.v4f32(<4 x float> [[WIDE_LOAD0I:%.*]], <4 x float> [[VEC_PHI50]])

; CHECK:  VPlannedBB5:
; CHECK:    [[TMP11:%.*]] = call float @llvm.vector.reduce.fmaximum.v4f32(<4 x float> [[TMP6]])

entry:
  %red = alloca float, align 4
  br label %begin.simd

begin.simd:
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.REDUCTION.MAX:TYPED"(float* %red, float zeroinitializer, i32 1) ]
  br label %DIR.QUAL.LIST.END.2

DIR.QUAL.LIST.END.2:
  %0 = load float, float* %red, align 4
  br label %for.body

for.body:
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body ], [ 1, %DIR.QUAL.LIST.END.2 ]
  %Max.014 = phi float [ %.Max.0, %for.body ], [ %0, %DIR.QUAL.LIST.END.2 ]
  %arrayidx1 = getelementptr inbounds float, float* %A, i64 %indvars.iv
  %1 = load float, float* %arrayidx1, align 4
  %.Max.0 = call float @llvm.maximum.f32(float %1, float %Max.014)
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 2049
  br i1 %exitcond, label %for.cond.cleanup.loopexit, label %for.body

for.cond.cleanup.loopexit:
  %.Max.0.lcssa = phi float [ %.Max.0, %for.body ]
  br label %end.simd

end.simd:
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.QUAL.LIST.END.3

DIR.QUAL.LIST.END.3:
  ret float %.Max.0.lcssa
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

; Function Attrs: nofree nosync nounwind readnone speculatable willreturn
declare float @llvm.maximum.f32(float, float) #1

attributes #1 = { nofree nosync nounwind readnone speculatable willreturn }
