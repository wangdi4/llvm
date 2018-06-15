; RUN: llc < %s | FileCheck %s
target datalayout = "e-m:e-i64:64-n32:64"
target triple = "csa"

; Function Attrs: nounwind uwtable
define void @norm_double({double, double}* noalias nocapture readonly %a, double* noalias nocapture %res, i32 %N) local_unnamed_addr #0 {
; CHECK-LABEL: norm_double
; CHECK: sld64x2
entry:
  %clie_pre = tail call i32 @llvm.csa.parallel.region.entry(i32 1027)
  %cmp25 = icmp sgt i32 %N, 0
  br i1 %cmp25, label %for.body.lr.ph, label %for.cond.cleanup

for.body.lr.ph:                                   ; preds = %entry
  %wide.trip.count = sext i32 %N to i64
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body, %entry
  tail call void @llvm.csa.parallel.region.exit(i32 %clie_pre)
  ret void

for.body:                                         ; preds = %for.body, %for.body.lr.ph
  %indvars.iv = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next, %for.body ]
  %clie_pse = tail call i32 @llvm.csa.parallel.section.entry(i32 %clie_pre)
  %_M_value.realp.i = getelementptr inbounds {double, double}, {double, double}* %a, i64 %indvars.iv, i32 0
  %0 = load double, double* %_M_value.realp.i, align 8
  %mul = fmul double %0, %0
  %_M_value.imagp.i23 = getelementptr inbounds {double, double}, {double, double}* %a, i64 %indvars.iv, i32 1
  %1 = load double, double* %_M_value.imagp.i23, align 8
  %mul10 = fmul double %1, %1
  %add = fadd double %mul, %mul10
  %2 = tail call double @llvm.sqrt.f64(double %add)
  %arrayidx12 = getelementptr inbounds double, double* %res, i64 %indvars.iv
  store double %2, double* %arrayidx12, align 8
  tail call void @llvm.csa.parallel.section.exit(i32 %clie_pse)
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.cond.cleanup, label %for.body
}

define void @norm_float({float, float}* noalias nocapture readonly %a, float* noalias nocapture %res, i32 %N) local_unnamed_addr #0 {
; CHECK-LABEL: norm_float
; CHECK: sld32x2
entry:
  %clie_pre = tail call i32 @llvm.csa.parallel.region.entry(i32 1027)
  %cmp25 = icmp sgt i32 %N, 0
  br i1 %cmp25, label %for.body.lr.ph, label %for.cond.cleanup

for.body.lr.ph:                                   ; preds = %entry
  %wide.trip.count = sext i32 %N to i64
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body, %entry
  tail call void @llvm.csa.parallel.region.exit(i32 %clie_pre)
  ret void

for.body:                                         ; preds = %for.body, %for.body.lr.ph
  %indvars.iv = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next, %for.body ]
  %clie_pse = tail call i32 @llvm.csa.parallel.section.entry(i32 %clie_pre)
  %_M_value.realp.i = getelementptr inbounds {float, float}, {float, float}* %a, i64 %indvars.iv, i32 0
  %0 = load float, float* %_M_value.realp.i, align 8
  %mul = fmul float %0, %0
  %_M_value.imagp.i23 = getelementptr inbounds {float, float}, {float, float}* %a, i64 %indvars.iv, i32 1
  %1 = load float, float* %_M_value.imagp.i23, align 8
  %mul10 = fmul float %1, %1
  %add = fadd float %mul, %mul10
  %2 = tail call float @llvm.sqrt.f32(float %add)
  %arrayidx12 = getelementptr inbounds float, float* %res, i64 %indvars.iv
  store float %2, float* %arrayidx12, align 8
  tail call void @llvm.csa.parallel.section.exit(i32 %clie_pse)
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.cond.cleanup, label %for.body
}

; NOTE: The above check sldx2 where the ordering output is %ign due to the
;       presence of a store. The cases below have no store, so they'll check
;       for the case when there is an actual ordering output.

; Function Attrs: nounwind uwtable
define double @norm_double_ret({double, double}* noalias nocapture readonly %a, i32 %N) local_unnamed_addr #0 {
; CHECK-LABEL: norm_double_ret
; CHECK: sld64x2
entry:
  %clie_pre = tail call i32 @llvm.csa.parallel.region.entry(i32 1027)
  %cmp25 = icmp sgt i32 %N, 0
  br i1 %cmp25, label %for.body.lr.ph, label %for.cond.cleanup

for.body.lr.ph:                                   ; preds = %entry
  %wide.trip.count = sext i32 %N to i64
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body, %entry
  %res_final = phi double [ 0.0, %entry ], [ %res_next, %for.body ]
  tail call void @llvm.csa.parallel.region.exit(i32 %clie_pre)
  ret double %res_final

for.body:                                         ; preds = %for.body, %for.body.lr.ph
  %res_prev = phi double [ 0.0, %for.body.lr.ph ], [ %res_next, %for.body ]
  %indvars.iv = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next, %for.body ]
  %clie_pse = tail call i32 @llvm.csa.parallel.section.entry(i32 %clie_pre)
  %_M_value.realp.i = getelementptr inbounds {double, double}, {double, double}* %a, i64 %indvars.iv, i32 0
  %0 = load double, double* %_M_value.realp.i, align 8
  %mul = fmul double %0, %0
  %_M_value.imagp.i23 = getelementptr inbounds {double, double}, {double, double}* %a, i64 %indvars.iv, i32 1
  %1 = load double, double* %_M_value.imagp.i23, align 8
  %mul10 = fmul double %1, %1
  %add = fadd double %mul, %mul10
  %2 = tail call double @llvm.sqrt.f64(double %add)
  %res_next = fadd double %res_prev, %2
  tail call void @llvm.csa.parallel.section.exit(i32 %clie_pse)
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.cond.cleanup, label %for.body
}

define float @norm_float_ret({float, float}* noalias nocapture readonly %a, i32 %N) local_unnamed_addr #0 {
; CHECK-LABEL: norm_float_ret
; CHECK: sld32x2
entry:
  %clie_pre = tail call i32 @llvm.csa.parallel.region.entry(i32 1027)
  %cmp25 = icmp sgt i32 %N, 0
  br i1 %cmp25, label %for.body.lr.ph, label %for.cond.cleanup

for.body.lr.ph:                                   ; preds = %entry
  %wide.trip.count = sext i32 %N to i64
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body, %entry
  %res_final = phi float [ 0.0, %entry ], [ %res_next, %for.body ]
  tail call void @llvm.csa.parallel.region.exit(i32 %clie_pre)
  ret float %res_final

for.body:                                         ; preds = %for.body, %for.body.lr.ph
  %res_prev = phi float [ 0.0, %for.body.lr.ph ], [ %res_next, %for.body ]
  %indvars.iv = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next, %for.body ]
  %clie_pse = tail call i32 @llvm.csa.parallel.section.entry(i32 %clie_pre)
  %_M_value.realp.i = getelementptr inbounds {float, float}, {float, float}* %a, i64 %indvars.iv, i32 0
  %0 = load float, float* %_M_value.realp.i, align 8
  %mul = fmul float %0, %0
  %_M_value.imagp.i23 = getelementptr inbounds {float, float}, {float, float}* %a, i64 %indvars.iv, i32 1
  %1 = load float, float* %_M_value.imagp.i23, align 8
  %mul10 = fmul float %1, %1
  %add = fadd float %mul, %mul10
  %2 = tail call float @llvm.sqrt.f32(float %add)
  %res_next = fadd float %res_prev, %2
  tail call void @llvm.csa.parallel.section.exit(i32 %clie_pse)
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.cond.cleanup, label %for.body
}

; Function Attrs: nounwind readnone speculatable
declare double @llvm.sqrt.f64(double) #1
declare float @llvm.sqrt.f32(float) #1

; Function Attrs: inaccessiblemem_or_argmemonly nounwind
declare i32 @llvm.csa.parallel.region.entry(i32) #2

; Function Attrs: inaccessiblemem_or_argmemonly nounwind
declare void @llvm.csa.parallel.region.exit(i32) #2

; Function Attrs: inaccessiblemem_or_argmemonly nounwind
declare i32 @llvm.csa.parallel.section.entry(i32) #2

; Function Attrs: inaccessiblemem_or_argmemonly nounwind
declare void @llvm.csa.parallel.section.exit(i32) #2

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone speculatable }
attributes #2 = { inaccessiblemem_or_argmemonly nounwind }

