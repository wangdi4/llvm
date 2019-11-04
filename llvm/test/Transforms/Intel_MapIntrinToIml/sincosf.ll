; Check to see that __svml_sincosf4 is translated to the high accuracy svml variant.

; RUN: opt -iml-trans -S < %s | FileCheck %s
; RUN: opt -passes="map-intrin-to-iml" -S < %s | FileCheck %s

; CHECK-LABEL: @vector_foo
; CHECK: call svml_cc <8 x float> @__svml_sincosf4_ha
; CHECK: ret

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define void @vector_foo(float* nocapture readonly %input, float* %vsin, float* %vcos) #0 {
entry:
  br label %vector.body

vector.body:                                      ; preds = %vector.body, %entry
  %index = phi i64 [ 0, %entry ], [ %index.next, %vector.body ]
  %broadcast.splatinsert = insertelement <4 x i64> undef, i64 %index, i32 0
  %broadcast.splat = shufflevector <4 x i64> %broadcast.splatinsert, <4 x i64> undef, <4 x i32> zeroinitializer
  %induction = add <4 x i64> %broadcast.splat, <i64 0, i64 1, i64 2, i64 3>
  %0 = getelementptr inbounds float, float* %input, i64 %index
  %1 = extractelement <4 x i64> %induction, i32 1
  %2 = extractelement <4 x i64> %induction, i32 2
  %3 = extractelement <4 x i64> %induction, i32 3
  %4 = bitcast float* %0 to <4 x float>*
  %wide.load = load <4 x float>, <4 x float>* %4, align 4
  %5 = getelementptr inbounds float, float* %vsin, i64 %index
  %6 = insertelement <4 x float*> undef, float* %5, i32 0
  %7 = getelementptr inbounds float, float* %vsin, i64 %1
  %8 = insertelement <4 x float*> %6, float* %7, i32 1
  %9 = getelementptr inbounds float, float* %vsin, i64 %2
  %10 = insertelement <4 x float*> %8, float* %9, i32 2
  %11 = getelementptr inbounds float, float* %vsin, i64 %3
  %12 = insertelement <4 x float*> %10, float* %11, i32 3
  %13 = getelementptr inbounds float, float* %vcos, i64 %index
  %14 = insertelement <4 x float*> undef, float* %13, i32 0
  %15 = getelementptr inbounds float, float* %vcos, i64 %1
  %16 = insertelement <4 x float*> %14, float* %15, i32 1
  %17 = getelementptr inbounds float, float* %vcos, i64 %2
  %18 = insertelement <4 x float*> %16, float* %17, i32 2
  %19 = getelementptr inbounds float, float* %vcos, i64 %3
  %20 = insertelement <4 x float*> %18, float* %19, i32 3
  call void @__svml_sincosf4(<4 x float> %wide.load, <4 x float*> "stride"="1" %12, <4 x float*> "stride"="1" %20)
  %index.next = add i64 %index, 4
  %21 = icmp eq i64 %index.next, 16
  br i1 %21, label %for.end, label %vector.body

for.end:                                          ; preds = %vector.body
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @__svml_sincosf4(<4 x float>, <4 x float*>, <4 x float*>) #1

attributes #0 = { nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }
