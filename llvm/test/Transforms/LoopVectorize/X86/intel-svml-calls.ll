; RUN: opt -vector-library=SVML -loop-vectorize -force-vector-width=4 -force-vector-interleave=1 -mattr=avx -S < %s | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

declare double @sind(double) #0
declare float @sindf(float) #0

declare double @cosd(double) #0
declare float @cosdf(float) #0

declare double @tand(double) #0
declare float @tandf(float) #0

declare void @sincos(double, double*, double*) #0
declare void @sincosf(float, float*, float*) #0

define void @sind_f64(double* nocapture %varray) {
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
  %arrayidx = getelementptr inbounds double, double* %varray, i64 %iv
  store double %call, double* %arrayidx, align 4
  %iv.next = add nuw nsw i64 %iv, 1
  %exitcond = icmp eq i64 %iv.next, 1000
  br i1 %exitcond, label %for.end, label %for.body

for.end:
  ret void
}

define void @sind_f32(float* nocapture %varray) {
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
  %arrayidx = getelementptr inbounds float, float* %varray, i64 %iv
  store float %call, float* %arrayidx, align 4
  %iv.next = add nuw nsw i64 %iv, 1
  %exitcond = icmp eq i64 %iv.next, 1000
  br i1 %exitcond, label %for.end, label %for.body

for.end:
  ret void
}

define void @cosd_f64(double* nocapture %varray) {
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
  %arrayidx = getelementptr inbounds double, double* %varray, i64 %iv
  store double %call, double* %arrayidx, align 4
  %iv.next = add nuw nsw i64 %iv, 1
  %exitcond = icmp eq i64 %iv.next, 1000
  br i1 %exitcond, label %for.end, label %for.body

for.end:
  ret void
}

define void @cosd_f32(float* nocapture %varray) {
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
  %arrayidx = getelementptr inbounds float, float* %varray, i64 %iv
  store float %call, float* %arrayidx, align 4
  %iv.next = add nuw nsw i64 %iv, 1
  %exitcond = icmp eq i64 %iv.next, 1000
  br i1 %exitcond, label %for.end, label %for.body

for.end:
  ret void
}

define void @tand_f64(double* nocapture %varray) {
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
  %arrayidx = getelementptr inbounds double, double* %varray, i64 %iv
  store double %call, double* %arrayidx, align 4
  %iv.next = add nuw nsw i64 %iv, 1
  %exitcond = icmp eq i64 %iv.next, 1000
  br i1 %exitcond, label %for.end, label %for.body

for.end:
  ret void
}

define void @tand_f32(float* nocapture %varray) {
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
  %arrayidx = getelementptr inbounds float, float* %varray, i64 %iv
  store float %call, float* %arrayidx, align 4
  %iv.next = add nuw nsw i64 %iv, 1
  %exitcond = icmp eq i64 %iv.next, 1000
  br i1 %exitcond, label %for.end, label %for.body

for.end:
  ret void
}

; sincos calls need special treatment, and shouldn't be vectorized by Loop
; Vectorizer for the time being
define void @sincos_f32(float* nocapture %varray) {
; CHECK-LABEL: @sincos_f32(
; CHECK: call {{.*}} @sincosf
; CHECK: call {{.*}} @sincosf
; CHECK: call {{.*}} @sincosf
; CHECK: call {{.*}} @sincosf
;
entry:
  %sinval = alloca float, align 4
  %cosval = alloca float, align 4
  %0 = bitcast float* %sinval to i8*
  %1 = bitcast float* %cosval to i8*
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  ret void

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %2 = trunc i64 %indvars.iv to i32
  %conv = sitofp i32 %2 to float
  call void @sincosf(float %conv, float* nonnull %sinval, float* nonnull %cosval)
  %3 = load float, float* %sinval, align 4
  %4 = load float, float* %cosval, align 4
  %add = fadd float %3, %4
  %ptridx = getelementptr inbounds float, float* %varray, i64 %indvars.iv
  store float %add, float* %ptridx, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1000
  br i1 %exitcond, label %for.cond.cleanup, label %for.body
}

define void @sincos_f64(double* nocapture %varray) {
; CHECK-LABEL: @sincos_f64(
; CHECK: call {{.*}} @sincos
; CHECK: call {{.*}} @sincos
; CHECK: call {{.*}} @sincos
; CHECK: call {{.*}} @sincos
;
entry:
  %sinval = alloca double, align 8
  %cosval = alloca double, align 8
  %0 = bitcast double* %sinval to i8*
  %1 = bitcast double* %cosval to i8*
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  ret void

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %2 = trunc i64 %indvars.iv to i32
  %conv = sitofp i32 %2 to double
  call void @sincos(double %conv, double* nonnull %sinval, double* nonnull %cosval)
  %3 = load double, double* %sinval, align 8
  %4 = load double, double* %cosval, align 8
  %add = fadd double %3, %4
  %ptridx = getelementptr inbounds double, double* %varray, i64 %indvars.iv
  store double %add, double* %ptridx, align 8
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1000
  br i1 %exitcond, label %for.cond.cleanup, label %for.body
}

attributes #0 = { nounwind readnone }
