; Check to see that __svml_atan2f4 is translated to the high accuracy svml variant.

; RUN: opt -iml-trans -S < %s | FileCheck %s

; CHECK-LABEL: @vector_foo
; CHECK: call <4 x float> @__svml_atan2f4_ha
; CHECK: ret

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define void @vector_foo([10 x float]* nocapture %varray) #0 {
entry:
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.inc7, %entry
  %indvars.iv19 = phi i64 [ 0, %entry ], [ %indvars.iv.next20, %for.inc7 ]
  %0 = trunc i64 %indvars.iv19 to i32
  %conv = sitofp i32 %0 to float
  %broadcast.splatinsert22 = insertelement <4 x float> undef, float %conv, i32 0
  %broadcast.splat23 = shufflevector <4 x float> %broadcast.splatinsert22, <4 x float> undef, <4 x i32> zeroinitializer
  br label %vector.body

vector.body:                                      ; preds = %vector.body, %for.cond1.preheader
  %index = phi i64 [ 0, %for.cond1.preheader ], [ %index.next, %vector.body ]
  %broadcast.splatinsert = insertelement <4 x i64> undef, i64 %index, i32 0
  %broadcast.splat = shufflevector <4 x i64> %broadcast.splatinsert, <4 x i64> undef, <4 x i32> zeroinitializer
  %1 = add <4 x i64> %broadcast.splat, <i64 1, i64 2, i64 3, i64 4>
  %2 = trunc <4 x i64> %1 to <4 x i32>
  %3 = sitofp <4 x i32> %2 to <4 x float>
  %4 = call fast <4 x float> @__svml_atan2f4(<4 x float> %broadcast.splat23, <4 x float> %3)
  %5 = getelementptr inbounds [10 x float], [10 x float]* %varray, i64 %indvars.iv19, i64 %index
  %6 = bitcast float* %5 to <4 x float>*
  store <4 x float> %4, <4 x float>* %6, align 4
  %index.next = add i64 %index, 4
  %7 = icmp eq i64 %index.next, 8
  br i1 %7, label %for.body3, label %vector.body

for.body3:                                        ; preds = %vector.body, %for.body3
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body3 ], [ 8, %vector.body ]
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %8 = trunc i64 %indvars.iv.next to i32
  %conv4 = sitofp i32 %8 to float
  %call = tail call fast float @atan2f(float %conv, float %conv4) #6
  %arrayidx6 = getelementptr inbounds [10 x float], [10 x float]* %varray, i64 %indvars.iv19, i64 %indvars.iv
  store float %call, float* %arrayidx6, align 4
  %exitcond = icmp eq i64 %indvars.iv.next, 10
  br i1 %exitcond, label %for.inc7, label %for.body3

for.inc7:                                         ; preds = %for.body3
  %indvars.iv.next20 = add nuw nsw i64 %indvars.iv19, 1
  %exitcond21 = icmp eq i64 %indvars.iv.next20, 10
  br i1 %exitcond21, label %for.end9, label %for.cond1.preheader

for.end9:                                         ; preds = %for.inc7
  ret void
}

; Function Attrs: nounwind readnone
declare float @atan2f(float, float) #2

; Function Attrs: nounwind readnone
declare <4 x float> @__svml_atan2f4(<4 x float>, <4 x float>) #6

attributes #0 = { nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #2 = { nounwind readnone "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #6 = { nounwind readnone }
