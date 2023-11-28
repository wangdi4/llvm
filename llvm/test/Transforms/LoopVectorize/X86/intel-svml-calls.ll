; RUN: opt -passes="inject-tli-mappings,loop-vectorize" -vector-library=SVML -force-vector-width=4 -force-vector-interleave=1 -mattr=avx -S < %s | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

declare double @sind(double) #0
declare float @sindf(float) #0

declare double @cosd(double) #0
declare float @cosdf(float) #0

declare double @tand(double) #0
declare float @tandf(float) #0

declare void @sincos(double, ptr, ptr) #0
declare void @sincosf(float, ptr, ptr) #0

declare double @ldexp(double, i32) #0
declare float @ldexpf(float, i32) #0

declare double @llvm.ldexp.f64(double, i32) #0
declare float @llvm.ldexp.f32(float, i32) #0

declare double @acospi(double) #0
declare float @acospif(float) #0

declare double @asinpi(double) #0
declare float @asinpif(float) #0

declare double @atanpi(double) #0
declare float @atanpif(float) #0

declare double @tanpi(double) #0
declare float @tanpif(float) #0

declare double @atan2pi(double, double) #0
declare float @atan2pif(float, float) #0

declare double @fdim(double, double) #0
declare float @fdimf(float, float) #0

declare double @maxmag(double, double) #0
declare float @maxmagf(float, float) #0

declare double @minmag(double, double) #0
declare float @minmagf(float, float) #0

declare double @pow2o3(double) #0
declare float @pow2o3f(float) #0

declare double @pow3o2(double) #0
declare float @pow3o2f(float) #0

declare double @powr(double, double) #0
declare float @powrf(float, float) #0

declare double @nextafter(double, double) #0
declare float @nextafterf(float, float) #0

declare double @remainder(double, double) #0
declare float @remainderf(float, float) #0

define void @sind_f64(ptr nocapture %varray) {
; CHECK-LABEL: @sind_f64(
; CHECK:    [[TMP5:%.*]] = call <4 x double> @__svml_sind4(<4 x double> [[TMP4:%.*]])
; CHECK:    ret void
;
entry:
  br label %for.body

for.body:
  %iv = phi i64 [ 0, %entry ], [ %iv.next, %for.body ]
  %tmp = trunc i64 %iv to i32
  %conv = sitofp i32 %tmp to double
  %call = tail call double @sind(double %conv)
  %arrayidx = getelementptr inbounds double, ptr %varray, i64 %iv
  store double %call, ptr %arrayidx, align 4
  %iv.next = add nuw nsw i64 %iv, 1
  %exitcond = icmp eq i64 %iv.next, 1000
  br i1 %exitcond, label %for.end, label %for.body

for.end:
  ret void
}

define void @sind_f32(ptr nocapture %varray) {
; CHECK-LABEL: @sind_f32(
; CHECK:    [[TMP5:%.*]] = call <4 x float> @__svml_sindf4(<4 x float> [[TMP4:%.*]])
; CHECK:    ret void
;
entry:
  br label %for.body

for.body:
  %iv = phi i64 [ 0, %entry ], [ %iv.next, %for.body ]
  %tmp = trunc i64 %iv to i32
  %conv = sitofp i32 %tmp to float
  %call = tail call float @sindf(float %conv)
  %arrayidx = getelementptr inbounds float, ptr %varray, i64 %iv
  store float %call, ptr %arrayidx, align 4
  %iv.next = add nuw nsw i64 %iv, 1
  %exitcond = icmp eq i64 %iv.next, 1000
  br i1 %exitcond, label %for.end, label %for.body

for.end:
  ret void
}

define void @cosd_f64(ptr nocapture %varray) {
; CHECK-LABEL: @cosd_f64(
; CHECK:    [[TMP5:%.*]] = call <4 x double> @__svml_cosd4(<4 x double> [[TMP4:%.*]])
; CHECK:    ret void
;
entry:
  br label %for.body

for.body:
  %iv = phi i64 [ 0, %entry ], [ %iv.next, %for.body ]
  %tmp = trunc i64 %iv to i32
  %conv = sitofp i32 %tmp to double
  %call = tail call double @cosd(double %conv)
  %arrayidx = getelementptr inbounds double, ptr %varray, i64 %iv
  store double %call, ptr %arrayidx, align 4
  %iv.next = add nuw nsw i64 %iv, 1
  %exitcond = icmp eq i64 %iv.next, 1000
  br i1 %exitcond, label %for.end, label %for.body

for.end:
  ret void
}

define void @cosd_f32(ptr nocapture %varray) {
; CHECK-LABEL: @cosd_f32(
; CHECK:    [[TMP5:%.*]] = call <4 x float> @__svml_cosdf4(<4 x float> [[TMP4:%.*]])
; CHECK:    ret void
;
entry:
  br label %for.body

for.body:
  %iv = phi i64 [ 0, %entry ], [ %iv.next, %for.body ]
  %tmp = trunc i64 %iv to i32
  %conv = sitofp i32 %tmp to float
  %call = tail call float @cosdf(float %conv)
  %arrayidx = getelementptr inbounds float, ptr %varray, i64 %iv
  store float %call, ptr %arrayidx, align 4
  %iv.next = add nuw nsw i64 %iv, 1
  %exitcond = icmp eq i64 %iv.next, 1000
  br i1 %exitcond, label %for.end, label %for.body

for.end:
  ret void
}

define void @tand_f64(ptr nocapture %varray) {
; CHECK-LABEL: @tand_f64(
; CHECK:    [[TMP5:%.*]] = call <4 x double> @__svml_tand4(<4 x double> [[TMP4:%.*]])
; CHECK:    ret void
;
entry:
  br label %for.body

for.body:
  %iv = phi i64 [ 0, %entry ], [ %iv.next, %for.body ]
  %tmp = trunc i64 %iv to i32
  %conv = sitofp i32 %tmp to double
  %call = tail call double @tand(double %conv)
  %arrayidx = getelementptr inbounds double, ptr %varray, i64 %iv
  store double %call, ptr %arrayidx, align 4
  %iv.next = add nuw nsw i64 %iv, 1
  %exitcond = icmp eq i64 %iv.next, 1000
  br i1 %exitcond, label %for.end, label %for.body

for.end:
  ret void
}

define void @tand_f32(ptr nocapture %varray) {
; CHECK-LABEL: @tand_f32(
; CHECK:    [[TMP5:%.*]] = call <4 x float> @__svml_tandf4(<4 x float> [[TMP4:%.*]])
; CHECK:    ret void
;
entry:
  br label %for.body

for.body:
  %iv = phi i64 [ 0, %entry ], [ %iv.next, %for.body ]
  %tmp = trunc i64 %iv to i32
  %conv = sitofp i32 %tmp to float
  %call = tail call float @tandf(float %conv)
  %arrayidx = getelementptr inbounds float, ptr %varray, i64 %iv
  store float %call, ptr %arrayidx, align 4
  %iv.next = add nuw nsw i64 %iv, 1
  %exitcond = icmp eq i64 %iv.next, 1000
  br i1 %exitcond, label %for.end, label %for.body

for.end:
  ret void
}

; sincos calls need special treatment, and shouldn't be vectorized by Loop
; Vectorizer for the time being.
define void @sincos_f32(ptr nocapture %varray) {
; CHECK-NOT: <4 x float
; CHECK: call {{.*}} @sincosf
;
entry:
  %sinval = alloca float, align 4
  %cosval = alloca float, align 4
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  ret void

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %0 = trunc i64 %indvars.iv to i32
  %conv = sitofp i32 %0 to float
  %sinval.iv = getelementptr inbounds float, ptr %sinval, i64 %indvars.iv
  %cosval.iv = getelementptr inbounds float, ptr %cosval, i64 %indvars.iv
  call void @sincosf(float %conv, ptr nonnull %sinval.iv, ptr nonnull %cosval.iv)
  %1 = load float, ptr %sinval.iv, align 4
  %2 = load float, ptr %cosval.iv, align 4
  %add = fadd float %1, %2
  %ptridx = getelementptr inbounds float, ptr %varray, i64 %indvars.iv
  store float %add, ptr %ptridx, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1000
  br i1 %exitcond, label %for.cond.cleanup, label %for.body
}

define void @sincos_f64(ptr nocapture %varray) {
; CHECK-LABEL: @sincos_f64(
; CHECK-NOT: <4 x float
; CHECK: call {{.*}} @sincos
;
entry:
  %sinval = alloca double, align 8
  %cosval = alloca double, align 8
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  ret void

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %0 = trunc i64 %indvars.iv to i32
  %conv = sitofp i32 %0 to double
  %sinval.iv = getelementptr inbounds double, ptr %sinval, i64 %indvars.iv
  %cosval.iv = getelementptr inbounds double, ptr %cosval, i64 %indvars.iv
  call void @sincos(double %conv, ptr nonnull %sinval.iv, ptr nonnull %cosval.iv)
  %1 = load double, ptr %sinval.iv, align 8
  %2 = load double, ptr %cosval.iv, align 8
  %add = fadd double %1, %2
  %ptridx = getelementptr inbounds double, ptr %varray, i64 %indvars.iv
  store double %add, ptr %ptridx, align 8
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1000
  br i1 %exitcond, label %for.cond.cleanup, label %for.body
}

define void @sincos_uniform_pointers(ptr nocapture %varray) {
; CHECK-LABEL: @sincos_uniform_pointers(
; CHECK-NOT: <4 x float
; CHECK: call {{.*}} @sincos
;
entry:
  %sinval = alloca double, align 8
  %cosval = alloca double, align 8
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  ret void

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %0 = trunc i64 %indvars.iv to i32
  %conv = sitofp i32 %0 to double
  call void @sincos(double %conv, ptr nonnull %sinval, ptr nonnull %cosval)
  %1 = load double, ptr %sinval, align 8
  %2 = load double, ptr %cosval, align 8
  %add = fadd double %1, %2
  %ptridx = getelementptr inbounds double, ptr %varray, i64 %indvars.iv
  store double %add, ptr %ptridx, align 8
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1000
  br i1 %exitcond, label %for.cond.cleanup, label %for.body
}

define void @ldexp_f64(ptr nocapture %varray) {
; CHECK-LABEL: @ldexp_f64(
; CHECK:    [[TMP5:%.*]] = call <4 x double> @__svml_ldexp4(<4 x double> [[TMP4:%.*]], <4 x i32> [[TMP3:%.*]])
; CHECK:    ret void
;
entry:
  br label %for.body

for.body:
  %iv = phi i64 [ 0, %entry ], [ %iv.next, %for.body ]
  %tmp = trunc i64 %iv to i32
  %conv = sitofp i32 %tmp to double
  %call = tail call double @ldexp(double %conv, i32 %tmp)
  %arrayidx = getelementptr inbounds double, ptr %varray, i64 %iv
  store double %call, ptr %arrayidx, align 4
  %iv.next = add nuw nsw i64 %iv, 1
  %exitcond = icmp eq i64 %iv.next, 1000
  br i1 %exitcond, label %for.end, label %for.body

for.end:
  ret void
}

define void @ldexpf_f32(ptr nocapture %varray) {
; CHECK-LABEL: @ldexpf_f32(
; CHECK:    [[TMP5:%.*]] = call <4 x float> @__svml_ldexpf4(<4 x float> [[TMP4:%.*]], <4 x i32> [[TMP3:%.*]])
; CHECK:    ret void
;
entry:
  br label %for.body

for.body:
  %iv = phi i64 [ 0, %entry ], [ %iv.next, %for.body ]
  %tmp = trunc i64 %iv to i32
  %conv = sitofp i32 %tmp to float
  %call = tail call float @ldexpf(float %conv, i32 %tmp)
  %arrayidx = getelementptr inbounds float, ptr %varray, i64 %iv
  store float %call, ptr %arrayidx, align 4
  %iv.next = add nuw nsw i64 %iv, 1
  %exitcond = icmp eq i64 %iv.next, 1000
  br i1 %exitcond, label %for.end, label %for.body

for.end:
  ret void
}

define void @lldexp_f64(ptr nocapture %varray) {
; CHECK-LABEL: @lldexp_f64(
; CHECK:    [[TMP5:%.*]] = call <4 x double> @__svml_ldexp4(<4 x double> [[TMP4:%.*]], <4 x i32> [[TMP3:%.*]])
; CHECK:    ret void
;
entry:
  br label %for.body

for.body:
  %iv = phi i64 [ 0, %entry ], [ %iv.next, %for.body ]
  %tmp = trunc i64 %iv to i32
  %conv = sitofp i32 %tmp to double
  %call = tail call double @llvm.ldexp.f64(double %conv, i32 %tmp)
  %arrayidx = getelementptr inbounds double, ptr %varray, i64 %iv
  store double %call, ptr %arrayidx, align 4
  %iv.next = add nuw nsw i64 %iv, 1
  %exitcond = icmp eq i64 %iv.next, 1000
  br i1 %exitcond, label %for.end, label %for.body

for.end:
  ret void
}

define void @lldexpf_f32(ptr nocapture %varray) {
; CHECK-LABEL: @lldexpf_f32(
; CHECK:    [[TMP5:%.*]] = call <4 x float> @__svml_ldexpf4(<4 x float> [[TMP4:%.*]], <4 x i32> [[TMP3:%.*]])
; CHECK:    ret void
;
entry:
  br label %for.body

for.body:
  %iv = phi i64 [ 0, %entry ], [ %iv.next, %for.body ]
  %tmp = trunc i64 %iv to i32
  %conv = sitofp i32 %tmp to float
  %call = tail call float @llvm.ldexp.f32(float %conv, i32 %tmp)
  %arrayidx = getelementptr inbounds float, ptr %varray, i64 %iv
  store float %call, ptr %arrayidx, align 4
  %iv.next = add nuw nsw i64 %iv, 1
  %exitcond = icmp eq i64 %iv.next, 1000
  br i1 %exitcond, label %for.end, label %for.body

for.end:
  ret void
}

define void @acospi_f64(ptr nocapture %varray) {
; CHECK-LABEL: @acospi_f64(
; CHECK:    [[TMP5:%.*]] = call <4 x double> @__svml_acospi4(<4 x double> [[TMP4:%.*]])
; CHECK:    ret void
;
entry:
  br label %for.body

for.body:
  %iv = phi i64 [ 0, %entry ], [ %iv.next, %for.body ]
  %tmp = trunc i64 %iv to i32
  %conv = sitofp i32 %tmp to double
  %call = tail call double @acospi(double %conv)
  %arrayidx = getelementptr inbounds double, ptr %varray, i64 %iv
  store double %call, ptr %arrayidx, align 4
  %iv.next = add nuw nsw i64 %iv, 1
  %exitcond = icmp eq i64 %iv.next, 1000
  br i1 %exitcond, label %for.end, label %for.body

for.end:
  ret void
}

define void @acospif_f32(ptr nocapture %varray) {
; CHECK-LABEL: @acospif_f32(
; CHECK:    [[TMP5:%.*]] = call <4 x float> @__svml_acospif4(<4 x float> [[TMP4:%.*]])
; CHECK:    ret void
;
entry:
  br label %for.body

for.body:
  %iv = phi i64 [ 0, %entry ], [ %iv.next, %for.body ]
  %tmp = trunc i64 %iv to i32
  %conv = sitofp i32 %tmp to float
  %call = tail call float @acospif(float %conv)
  %arrayidx = getelementptr inbounds float, ptr %varray, i64 %iv
  store float %call, ptr %arrayidx, align 4
  %iv.next = add nuw nsw i64 %iv, 1
  %exitcond = icmp eq i64 %iv.next, 1000
  br i1 %exitcond, label %for.end, label %for.body

for.end:
  ret void
}

define void @asinpi_f64(ptr nocapture %varray) {
; CHECK-LABEL: @asinpi_f64(
; CHECK:    [[TMP5:%.*]] = call <4 x double> @__svml_asinpi4(<4 x double> [[TMP4:%.*]])
; CHECK:    ret void
;
entry:
  br label %for.body

for.body:
  %iv = phi i64 [ 0, %entry ], [ %iv.next, %for.body ]
  %tmp = trunc i64 %iv to i32
  %conv = sitofp i32 %tmp to double
  %call = tail call double @asinpi(double %conv)
  %arrayidx = getelementptr inbounds double, ptr %varray, i64 %iv
  store double %call, ptr %arrayidx, align 4
  %iv.next = add nuw nsw i64 %iv, 1
  %exitcond = icmp eq i64 %iv.next, 1000
  br i1 %exitcond, label %for.end, label %for.body

for.end:
  ret void
}

define void @asinpif_f32(ptr nocapture %varray) {
; CHECK-LABEL: @asinpif_f32(
; CHECK:    [[TMP5:%.*]] = call <4 x float> @__svml_asinpif4(<4 x float> [[TMP4:%.*]])
; CHECK:    ret void
;
entry:
  br label %for.body

for.body:
  %iv = phi i64 [ 0, %entry ], [ %iv.next, %for.body ]
  %tmp = trunc i64 %iv to i32
  %conv = sitofp i32 %tmp to float
  %call = tail call float @asinpif(float %conv)
  %arrayidx = getelementptr inbounds float, ptr %varray, i64 %iv
  store float %call, ptr %arrayidx, align 4
  %iv.next = add nuw nsw i64 %iv, 1
  %exitcond = icmp eq i64 %iv.next, 1000
  br i1 %exitcond, label %for.end, label %for.body

for.end:
  ret void
}

define void @atanpi_f64(ptr nocapture %varray) {
; CHECK-LABEL: @atanpi_f64(
; CHECK:    [[TMP5:%.*]] = call <4 x double> @__svml_atanpi4(<4 x double> [[TMP4:%.*]])
; CHECK:    ret void
;
entry:
  br label %for.body

for.body:
  %iv = phi i64 [ 0, %entry ], [ %iv.next, %for.body ]
  %tmp = trunc i64 %iv to i32
  %conv = sitofp i32 %tmp to double
  %call = tail call double @atanpi(double %conv)
  %arrayidx = getelementptr inbounds double, ptr %varray, i64 %iv
  store double %call, ptr %arrayidx, align 4
  %iv.next = add nuw nsw i64 %iv, 1
  %exitcond = icmp eq i64 %iv.next, 1000
  br i1 %exitcond, label %for.end, label %for.body

for.end:
  ret void
}

define void @atanpif_f32(ptr nocapture %varray) {
; CHECK-LABEL: @atanpif_f32(
; CHECK:    [[TMP5:%.*]] = call <4 x float> @__svml_atanpif4(<4 x float> [[TMP4:%.*]])
; CHECK:    ret void
;
entry:
  br label %for.body

for.body:
  %iv = phi i64 [ 0, %entry ], [ %iv.next, %for.body ]
  %tmp = trunc i64 %iv to i32
  %conv = sitofp i32 %tmp to float
  %call = tail call float @atanpif(float %conv)
  %arrayidx = getelementptr inbounds float, ptr %varray, i64 %iv
  store float %call, ptr %arrayidx, align 4
  %iv.next = add nuw nsw i64 %iv, 1
  %exitcond = icmp eq i64 %iv.next, 1000
  br i1 %exitcond, label %for.end, label %for.body

for.end:
  ret void
}

define void @tanpi_f64(ptr nocapture %varray) {
; CHECK-LABEL: @tanpi_f64(
; CHECK:    [[TMP5:%.*]] = call <4 x double> @__svml_tanpi4(<4 x double> [[TMP4:%.*]])
; CHECK:    ret void
;
entry:
  br label %for.body

for.body:
  %iv = phi i64 [ 0, %entry ], [ %iv.next, %for.body ]
  %tmp = trunc i64 %iv to i32
  %conv = sitofp i32 %tmp to double
  %call = tail call double @tanpi(double %conv)
  %arrayidx = getelementptr inbounds double, ptr %varray, i64 %iv
  store double %call, ptr %arrayidx, align 4
  %iv.next = add nuw nsw i64 %iv, 1
  %exitcond = icmp eq i64 %iv.next, 1000
  br i1 %exitcond, label %for.end, label %for.body

for.end:
  ret void
}

define void @tanpif_f32(ptr nocapture %varray) {
; CHECK-LABEL: @tanpif_f32(
; CHECK:    [[TMP5:%.*]] = call <4 x float> @__svml_tanpif4(<4 x float> [[TMP4:%.*]])
; CHECK:    ret void
;
entry:
  br label %for.body

for.body:
  %iv = phi i64 [ 0, %entry ], [ %iv.next, %for.body ]
  %tmp = trunc i64 %iv to i32
  %conv = sitofp i32 %tmp to float
  %call = tail call float @tanpif(float %conv)
  %arrayidx = getelementptr inbounds float, ptr %varray, i64 %iv
  store float %call, ptr %arrayidx, align 4
  %iv.next = add nuw nsw i64 %iv, 1
  %exitcond = icmp eq i64 %iv.next, 1000
  br i1 %exitcond, label %for.end, label %for.body

for.end:
  ret void
}

define void @atan2pi_f64(ptr nocapture %varray, ptr nocapture readonly %varray2) {
; CHECK-LABEL: @atan2pi_f64(
; CHECK:    [[TMP5:%.*]] = call <4 x double> @__svml_atan2pi4(<4 x double> [[TMP4:%.*]], <4 x double> [[TMP3:%.*]])
; CHECK:    ret void
;
entry:
  br label %for.body

for.body:
  %iv = phi i64 [ 0, %entry ], [ %iv.next, %for.body ]
  %tmp = trunc i64 %iv to i32
  %conv = sitofp i32 %tmp to double
  %arrayidx = getelementptr inbounds double, ptr %varray2, i64 %iv
  %tmp1 = load double, ptr %arrayidx, align 4
  %tmp2 = tail call double @atan2pi(double %conv, double %tmp1)
  %arrayidx2 = getelementptr inbounds double, ptr %varray, i64 %iv
  store double %tmp2, ptr %arrayidx2, align 4
  %iv.next = add nuw nsw i64 %iv, 1
  %exitcond = icmp eq i64 %iv.next, 1000
  br i1 %exitcond, label %for.end, label %for.body

for.end:
  ret void
}

define void @atan2pif_f32(ptr nocapture %varray, ptr nocapture readonly %varray2) {
; CHECK-LABEL: @atan2pif_f32(
; CHECK:    [[TMP5:%.*]] = call <4 x float> @__svml_atan2pif4(<4 x float> [[TMP4:%.*]], <4 x float> [[TMP3:%.*]])
; CHECK:    ret void
;
entry:
  br label %for.body

for.body:
  %iv = phi i64 [ 0, %entry ], [ %iv.next, %for.body ]
  %tmp = trunc i64 %iv to i32
  %conv = sitofp i32 %tmp to float
  %arrayidx = getelementptr inbounds float, ptr %varray2, i64 %iv
  %tmp1 = load float, ptr %arrayidx, align 4
  %tmp2 = tail call float @atan2pif(float %conv, float %tmp1)
  %arrayidx2 = getelementptr inbounds float, ptr %varray, i64 %iv
  store float %tmp2, ptr %arrayidx2, align 4
  %iv.next = add nuw nsw i64 %iv, 1
  %exitcond = icmp eq i64 %iv.next, 1000
  br i1 %exitcond, label %for.end, label %for.body

for.end:
  ret void
}

define void @fdim_f64(ptr nocapture %varray, ptr nocapture readonly %varray2) {
; CHECK-LABEL: @fdim_f64(
; CHECK:    [[TMP5:%.*]] = call <4 x double> @__svml_fdim4(<4 x double> [[TMP4:%.*]], <4 x double> [[TMP3:%.*]])
; CHECK:    ret void
;
entry:
  br label %for.body

for.body:
  %iv = phi i64 [ 0, %entry ], [ %iv.next, %for.body ]
  %tmp = trunc i64 %iv to i32
  %conv = sitofp i32 %tmp to double
  %arrayidx = getelementptr inbounds double, ptr %varray2, i64 %iv
  %tmp1 = load double, ptr %arrayidx, align 4
  %tmp2 = tail call double @fdim(double %conv, double %tmp1)
  %arrayidx2 = getelementptr inbounds double, ptr %varray, i64 %iv
  store double %tmp2, ptr %arrayidx2, align 4
  %iv.next = add nuw nsw i64 %iv, 1
  %exitcond = icmp eq i64 %iv.next, 1000
  br i1 %exitcond, label %for.end, label %for.body

for.end:
  ret void
}

define void @fdimf_f32(ptr nocapture %varray, ptr nocapture readonly %varray2) {
; CHECK-LABEL: @fdimf_f32(
; CHECK:    [[TMP5:%.*]] = call <4 x float> @__svml_fdimf4(<4 x float> [[TMP4:%.*]], <4 x float> [[TMP3:%.*]])
; CHECK:    ret void
;
entry:
  br label %for.body

for.body:
  %iv = phi i64 [ 0, %entry ], [ %iv.next, %for.body ]
  %tmp = trunc i64 %iv to i32
  %conv = sitofp i32 %tmp to float
  %arrayidx = getelementptr inbounds float, ptr %varray2, i64 %iv
  %tmp1 = load float, ptr %arrayidx, align 4
  %tmp2 = tail call float @fdimf(float %conv, float %tmp1)
  %arrayidx2 = getelementptr inbounds float, ptr %varray, i64 %iv
  store float %tmp2, ptr %arrayidx2, align 4
  %iv.next = add nuw nsw i64 %iv, 1
  %exitcond = icmp eq i64 %iv.next, 1000
  br i1 %exitcond, label %for.end, label %for.body

for.end:
  ret void
}

define void @maxmag_f64(ptr nocapture %varray, ptr nocapture readonly %varray2) {
; CHECK-LABEL: @maxmag_f64(
; CHECK:    [[TMP5:%.*]] = call <4 x double> @__svml_maxmag4(<4 x double> [[TMP4:%.*]], <4 x double> [[TMP3:%.*]])
; CHECK:    ret void
;
entry:
  br label %for.body

for.body:
  %iv = phi i64 [ 0, %entry ], [ %iv.next, %for.body ]
  %tmp = trunc i64 %iv to i32
  %conv = sitofp i32 %tmp to double
  %arrayidx = getelementptr inbounds double, ptr %varray2, i64 %iv
  %tmp1 = load double, ptr %arrayidx, align 4
  %tmp2 = tail call double @maxmag(double %conv, double %tmp1)
  %arrayidx2 = getelementptr inbounds double, ptr %varray, i64 %iv
  store double %tmp2, ptr %arrayidx2, align 4
  %iv.next = add nuw nsw i64 %iv, 1
  %exitcond = icmp eq i64 %iv.next, 1000
  br i1 %exitcond, label %for.end, label %for.body

for.end:
  ret void
}

define void @maxmagf_f32(ptr nocapture %varray, ptr nocapture readonly %varray2) {
; CHECK-LABEL: @maxmagf_f32(
; CHECK:    [[TMP5:%.*]] = call <4 x float> @__svml_maxmagf4(<4 x float> [[TMP4:%.*]], <4 x float> [[TMP3:%.*]])
; CHECK:    ret void
;
entry:
  br label %for.body

for.body:
  %iv = phi i64 [ 0, %entry ], [ %iv.next, %for.body ]
  %tmp = trunc i64 %iv to i32
  %conv = sitofp i32 %tmp to float
  %arrayidx = getelementptr inbounds float, ptr %varray2, i64 %iv
  %tmp1 = load float, ptr %arrayidx, align 4
  %tmp2 = tail call float @maxmagf(float %conv, float %tmp1)
  %arrayidx2 = getelementptr inbounds float, ptr %varray, i64 %iv
  store float %tmp2, ptr %arrayidx2, align 4
  %iv.next = add nuw nsw i64 %iv, 1
  %exitcond = icmp eq i64 %iv.next, 1000
  br i1 %exitcond, label %for.end, label %for.body

for.end:
  ret void
}

define void @minmag_f64(ptr nocapture %varray, ptr nocapture readonly %varray2) {
; CHECK-LABEL: @minmag_f64(
; CHECK:    [[TMP5:%.*]] = call <4 x double> @__svml_minmag4(<4 x double> [[TMP4:%.*]], <4 x double> [[TMP3:%.*]])
; CHECK:    ret void
;
entry:
  br label %for.body

for.body:
  %iv = phi i64 [ 0, %entry ], [ %iv.next, %for.body ]
  %tmp = trunc i64 %iv to i32
  %conv = sitofp i32 %tmp to double
  %arrayidx = getelementptr inbounds double, ptr %varray2, i64 %iv
  %tmp1 = load double, ptr %arrayidx, align 4
  %tmp2 = tail call double @minmag(double %conv, double %tmp1)
  %arrayidx2 = getelementptr inbounds double, ptr %varray, i64 %iv
  store double %tmp2, ptr %arrayidx2, align 4
  %iv.next = add nuw nsw i64 %iv, 1
  %exitcond = icmp eq i64 %iv.next, 1000
  br i1 %exitcond, label %for.end, label %for.body

for.end:
  ret void
}

define void @minmagf_f32(ptr nocapture %varray, ptr nocapture readonly %varray2) {
; CHECK-LABEL: @minmagf_f32(
; CHECK:    [[TMP5:%.*]] = call <4 x float> @__svml_minmagf4(<4 x float> [[TMP4:%.*]], <4 x float> [[TMP3:%.*]])
; CHECK:    ret void
;
entry:
  br label %for.body

for.body:
  %iv = phi i64 [ 0, %entry ], [ %iv.next, %for.body ]
  %tmp = trunc i64 %iv to i32
  %conv = sitofp i32 %tmp to float
  %arrayidx = getelementptr inbounds float, ptr %varray2, i64 %iv
  %tmp1 = load float, ptr %arrayidx, align 4
  %tmp2 = tail call float @minmagf(float %conv, float %tmp1)
  %arrayidx2 = getelementptr inbounds float, ptr %varray, i64 %iv
  store float %tmp2, ptr %arrayidx2, align 4
  %iv.next = add nuw nsw i64 %iv, 1
  %exitcond = icmp eq i64 %iv.next, 1000
  br i1 %exitcond, label %for.end, label %for.body

for.end:
  ret void
}

define void @pow2o3_f64(ptr nocapture %varray) {
; CHECK-LABEL: @pow2o3_f64(
; CHECK:    [[TMP5:%.*]] = call <4 x double> @__svml_pow2o34(<4 x double> [[TMP4:%.*]])
; CHECK:    ret void
;
entry:
  br label %for.body

for.body:
  %iv = phi i64 [ 0, %entry ], [ %iv.next, %for.body ]
  %tmp = trunc i64 %iv to i32
  %conv = sitofp i32 %tmp to double
  %call = tail call double @pow2o3(double %conv)
  %arrayidx = getelementptr inbounds double, ptr %varray, i64 %iv
  store double %call, ptr %arrayidx, align 4
  %iv.next = add nuw nsw i64 %iv, 1
  %exitcond = icmp eq i64 %iv.next, 1000
  br i1 %exitcond, label %for.end, label %for.body

for.end:
  ret void
}

define void @pow2o3f_f32(ptr nocapture %varray) {
; CHECK-LABEL: @pow2o3f_f32(
; CHECK:    [[TMP5:%.*]] = call <4 x float> @__svml_pow2o3f4(<4 x float> [[TMP4:%.*]])
; CHECK:    ret void
;
entry:
  br label %for.body

for.body:
  %iv = phi i64 [ 0, %entry ], [ %iv.next, %for.body ]
  %tmp = trunc i64 %iv to i32
  %conv = sitofp i32 %tmp to float
  %call = tail call float @pow2o3f(float %conv)
  %arrayidx = getelementptr inbounds float, ptr %varray, i64 %iv
  store float %call, ptr %arrayidx, align 4
  %iv.next = add nuw nsw i64 %iv, 1
  %exitcond = icmp eq i64 %iv.next, 1000
  br i1 %exitcond, label %for.end, label %for.body

for.end:
  ret void
}

define void @pow3o2_f64(ptr nocapture %varray) {
; CHECK-LABEL: @pow3o2_f64(
; CHECK:    [[TMP5:%.*]] = call <4 x double> @__svml_pow3o24(<4 x double> [[TMP4:%.*]])
; CHECK:    ret void
;
entry:
  br label %for.body

for.body:
  %iv = phi i64 [ 0, %entry ], [ %iv.next, %for.body ]
  %tmp = trunc i64 %iv to i32
  %conv = sitofp i32 %tmp to double
  %call = tail call double @pow3o2(double %conv)
  %arrayidx = getelementptr inbounds double, ptr %varray, i64 %iv
  store double %call, ptr %arrayidx, align 4
  %iv.next = add nuw nsw i64 %iv, 1
  %exitcond = icmp eq i64 %iv.next, 1000
  br i1 %exitcond, label %for.end, label %for.body

for.end:
  ret void
}

define void @pow3o2f_f32(ptr nocapture %varray) {
; CHECK-LABEL: @pow3o2f_f32(
; CHECK:    [[TMP5:%.*]] = call <4 x float> @__svml_pow3o2f4(<4 x float> [[TMP4:%.*]])
; CHECK:    ret void
;
entry:
  br label %for.body

for.body:
  %iv = phi i64 [ 0, %entry ], [ %iv.next, %for.body ]
  %tmp = trunc i64 %iv to i32
  %conv = sitofp i32 %tmp to float
  %call = tail call float @pow3o2f(float %conv)
  %arrayidx = getelementptr inbounds float, ptr %varray, i64 %iv
  store float %call, ptr %arrayidx, align 4
  %iv.next = add nuw nsw i64 %iv, 1
  %exitcond = icmp eq i64 %iv.next, 1000
  br i1 %exitcond, label %for.end, label %for.body

for.end:
  ret void
}

define void @powr_f64(ptr nocapture %varray, ptr nocapture readonly %varray2) {
; CHECK-LABEL: @powr_f64(
; CHECK:    [[TMP5:%.*]] = call <4 x double> @__svml_powr4(<4 x double> [[TMP4:%.*]], <4 x double> [[TMP3:%.*]])
; CHECK:    ret void
;
entry:
  br label %for.body

for.body:
  %iv = phi i64 [ 0, %entry ], [ %iv.next, %for.body ]
  %tmp = trunc i64 %iv to i32
  %conv = sitofp i32 %tmp to double
  %arrayidx = getelementptr inbounds double, ptr %varray2, i64 %iv
  %tmp1 = load double, ptr %arrayidx, align 4
  %tmp2 = tail call double @powr(double %conv, double %tmp1)
  %arrayidx2 = getelementptr inbounds double, ptr %varray, i64 %iv
  store double %tmp2, ptr %arrayidx2, align 4
  %iv.next = add nuw nsw i64 %iv, 1
  %exitcond = icmp eq i64 %iv.next, 1000
  br i1 %exitcond, label %for.end, label %for.body

for.end:
  ret void
}

define void @powrf_f32(ptr nocapture %varray, ptr nocapture readonly %varray2) {
; CHECK-LABEL: @powrf_f32(
; CHECK:    [[TMP5:%.*]] = call <4 x float> @__svml_powrf4(<4 x float> [[TMP4:%.*]], <4 x float> [[TMP3:%.*]])
; CHECK:    ret void
;
entry:
  br label %for.body

for.body:
  %iv = phi i64 [ 0, %entry ], [ %iv.next, %for.body ]
  %tmp = trunc i64 %iv to i32
  %conv = sitofp i32 %tmp to float
  %arrayidx = getelementptr inbounds float, ptr %varray2, i64 %iv
  %tmp1 = load float, ptr %arrayidx, align 4
  %tmp2 = tail call float @powrf(float %conv, float %tmp1)
  %arrayidx2 = getelementptr inbounds float, ptr %varray, i64 %iv
  store float %tmp2, ptr %arrayidx2, align 4
  %iv.next = add nuw nsw i64 %iv, 1
  %exitcond = icmp eq i64 %iv.next, 1000
  br i1 %exitcond, label %for.end, label %for.body

for.end:
  ret void
}

define void @nextafter_f64(ptr nocapture %varray, ptr nocapture readonly %varray2, ptr nocapture readonly %varray3) #0 {
; CHECK-LABEL: @nextafter_f64(
; CHECK:    [[TMP5:%.*]] = call <4 x double> @__svml_nextafter4(<4 x double> [[TMP4:%.*]], <4 x double> [[TMP3:%.*]])
; CHECK:    ret void
entry:
  br label %for.body

for.body:
  %iv = phi i64 [ 0, %entry ], [ %iv.next, %for.body ]
  %arrayidx = getelementptr inbounds double, ptr %varray2, i64 %iv
  %tmp = load double, ptr %arrayidx, align 4
  %arrayidx2 = getelementptr inbounds double, ptr %varray3, i64 %iv
  %tmp1 = load double, ptr %arrayidx2, align 4
  %call = tail call double @nextafter(double %tmp, double %tmp1)
  %arrayidx4 = getelementptr inbounds double, ptr %varray, i64 %iv
  store double %call, ptr %arrayidx4, align 4
  %iv.next = add nuw nsw i64 %iv, 1
  %exitcond = icmp eq i64 %iv.next, 1000
  br i1 %exitcond, label %for.end, label %for.body

for.end:
  ret void
}

define void @nextafterf_f32(ptr nocapture %varray, ptr nocapture readonly %varray2, ptr nocapture readonly %varray3) #0 {
; CHECK-LABEL: @nextafterf_f32(
; CHECK:    [[TMP5:%.*]] = call <4 x float> @__svml_nextafterf4(<4 x float> [[TMP4:%.*]], <4 x float> [[TMP3:%.*]])
; CHECK:    ret void
entry:
  br label %for.body

for.body:
  %iv = phi i64 [ 0, %entry ], [ %iv.next, %for.body ]
  %arrayidx = getelementptr inbounds float, ptr %varray2, i64 %iv
  %tmp = load float, ptr %arrayidx, align 4
  %arrayidx2 = getelementptr inbounds float, ptr %varray3, i64 %iv
  %tmp1 = load float, ptr %arrayidx2, align 4
  %call = tail call float @nextafterf(float %tmp, float %tmp1)
  %arrayidx4 = getelementptr inbounds float, ptr %varray, i64 %iv
  store float %call, ptr %arrayidx4, align 4
  %iv.next = add nuw nsw i64 %iv, 1
  %exitcond = icmp eq i64 %iv.next, 1000
  br i1 %exitcond, label %for.end, label %for.body

for.end:
  ret void
}

define void @remainder_f64(ptr nocapture %varray, ptr nocapture readonly %varray2, ptr nocapture readonly %varray3) #0 {
; CHECK-LABEL: @remainder_f64(
; CHECK:    [[TMP5:%.*]] = call <4 x double> @__svml_remainder4(<4 x double> [[TMP4:%.*]], <4 x double> [[TMP3:%.*]])
; CHECK:    ret void
entry:
  br label %for.body

for.body:
  %iv = phi i64 [ 0, %entry ], [ %iv.next, %for.body ]
  %arrayidx = getelementptr inbounds double, ptr %varray2, i64 %iv
  %tmp = load double, ptr %arrayidx, align 4
  %arrayidx2 = getelementptr inbounds double, ptr %varray3, i64 %iv
  %tmp1 = load double, ptr %arrayidx2, align 4
  %call = tail call double @remainder(double %tmp, double %tmp1)
  %arrayidx4 = getelementptr inbounds double, ptr %varray, i64 %iv
  store double %call, ptr %arrayidx4, align 4
  %iv.next = add nuw nsw i64 %iv, 1
  %exitcond = icmp eq i64 %iv.next, 1000
  br i1 %exitcond, label %for.end, label %for.body

for.end:
  ret void
}

define void @remainderf_f32(ptr nocapture %varray, ptr nocapture readonly %varray2, ptr nocapture readonly %varray3) #0 {
; CHECK-LABEL: @remainderf_f32(
; CHECK:    [[TMP5:%.*]] = call <4 x float> @__svml_remainderf4(<4 x float> [[TMP4:%.*]], <4 x float> [[TMP3:%.*]])
; CHECK:    ret void
entry:
  br label %for.body

for.body:
  %iv = phi i64 [ 0, %entry ], [ %iv.next, %for.body ]
  %arrayidx = getelementptr inbounds float, ptr %varray2, i64 %iv
  %tmp = load float, ptr %arrayidx, align 4
  %arrayidx2 = getelementptr inbounds float, ptr %varray3, i64 %iv
  %tmp1 = load float, ptr %arrayidx2, align 4
  %call = tail call float @remainderf(float %tmp, float %tmp1)
  %arrayidx4 = getelementptr inbounds float, ptr %varray, i64 %iv
  store float %call, ptr %arrayidx4, align 4
  %iv.next = add nuw nsw i64 %iv, 1
  %exitcond = icmp eq i64 %iv.next, 1000
  br i1 %exitcond, label %for.end, label %for.body

for.end:
  ret void
}

attributes #0 = { nounwind readnone }
