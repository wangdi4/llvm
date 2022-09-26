; Test to check VPlan's cost modeling results for calls using sinpi/cospi
; functions. These calls will be lowered using SVML vector versions and
; for cost-model's purpose we treat them like llvm.sin/llvm.cos intrinsics
; instead of unknown user calls.

; RUN: opt -hir-ssa-deconstruction -hir-framework -hir-vplan-vec -vector-library=SVML -vplan-cost-model-print-analysis-for-vf=4 -disable-output < %s | FileCheck %s
; RUN: opt -vplan-vec -vector-library=SVML -vplan-cost-model-print-analysis-for-vf=4 -disable-output < %s | FileCheck %s

; CHECK-LABEL:   Cost Model for VPlan foo:{{.*}} with VF = 4:
; CHECK:         Cost 26 for float {{%vp.*}} = call float [[ARG:%vp.*]] __svml_cospif4 [x 1]
; CHECK-NEXT:    Cost 26 for float {{%vp.*}} = call float [[ARG]] __svml_sinpif4 [x 1]

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.kValues = type { float, float, float }

define void @foo(%struct.kValues* %src, float* %dest, i64 %wide.trip.count) {
for.preheader:
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %for.body

for.body:
  %indvars.iv = phi i64 [ 0, %for.preheader ], [ %indvars.iv.next, %for.body ]
  %Kx = getelementptr inbounds %struct.kValues, %struct.kValues* %src, i64 %indvars.iv, i32 0
  %Kx.load = load float, float* %Kx, align 4
  %Ky = getelementptr inbounds %struct.kValues, %struct.kValues* %src, i64 %indvars.iv, i32 1
  %Ky.load = load float, float* %Ky, align 4
  %add28 = fadd fast float %Kx.load, %Ky.load
  %Kz = getelementptr inbounds %struct.kValues, %struct.kValues* %src, i64 %indvars.iv, i32 2
  %Kz.load = load float, float* %Kz, align 4
  %add34 = fadd fast float %add28, %Kz.load
  %mul35.overpi = fmul fast float %add34, 2.000000e+00
  %cos.val = call fast float @cospif(float %mul35.overpi)
  %sin.val = call fast float @sinpif(float %mul35.overpi)
  %res.val = fadd fast float %cos.val, %sin.val
  %res.ptr = getelementptr inbounds float, float* %dest, i64 %indvars.iv
  store float %res.val, float* %res.ptr, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.exit, label %for.body

for.exit:
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

; Function Attrs: nofree nosync nounwind readnone speculatable willreturn
declare float @cospif(float) local_unnamed_addr #1

; Function Attrs: nofree nosync nounwind readnone speculatable willreturn
declare float @sinpif(float) local_unnamed_addr #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry()

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token)

attributes #1 = { nofree nosync nounwind readnone speculatable willreturn }
