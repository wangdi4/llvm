; Check to see that the llvm.log.v4f32 intrinsic is translated to svml.

; RUN: opt -iml-trans -S < %s | FileCheck %s

; CHECK-LABEL: @foo
; CHECK: call <4 x float> @__svml_logf4
; CHECK: ret

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [6 x i8] c"%.2f\0A\00", align 1

; Function Attrs: nounwind uwtable
define void @foo(float* nocapture %array) #0 {
entry:
  br label %vector.body

vector.body:                                      ; preds = %vector.body, %entry
  %index = phi i64 [ 0, %entry ], [ %index.next, %vector.body ]
  %0 = trunc i64 %index to i32
  %broadcast.splatinsert6 = insertelement <4 x i32> undef, i32 %0, i32 0
  %broadcast.splat7 = shufflevector <4 x i32> %broadcast.splatinsert6, <4 x i32> undef, <4 x i32> zeroinitializer
  %induction8 = add <4 x i32> %broadcast.splat7, <i32 0, i32 1, i32 2, i32 3>
  %1 = sitofp <4 x i32> %induction8 to <4 x float>
  %2 = call <4 x float> @llvm.log.v4f32(<4 x float> %1)
  %3 = getelementptr inbounds float, float* %array, i64 %index
  %4 = bitcast float* %3 to <4 x float>*
  store <4 x float> %2, <4 x float>* %4, align 4
  %index.next = add i64 %index, 4
  %5 = icmp eq i64 %index.next, 1000
  br i1 %5, label %for.end, label %vector.body

for.end:                                          ; preds = %vector.body
  ret void
}

; Function Attrs: nounwind readnone
declare <4 x float> @llvm.log.v4f32(<4 x float>) #3

attributes #0 = { nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #3 = { nounwind readnone }
