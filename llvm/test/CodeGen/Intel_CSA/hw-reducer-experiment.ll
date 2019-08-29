; RUN: llc -csa-hw-reducer-experiment <%s | FileCheck --implicit-check-not redmulf --implicit-check-not fmredaf %s
target datalayout = "e-m:e-i64:64-n32:64"
target triple = "csa"

define float @addf32(float* %A) {
entry:
; CHECK-LABEL: addf32
; CHECK: redaddf32
  br label %loop

loop:
  %i = phi i64 [0, %entry], [%i_next, %loop]
  %cur_red = phi float [0.0, %entry], [%red, %loop]
  %A_idx = getelementptr inbounds float, float* %A, i64 %i
  %A_i = load float, float* %A_idx
  %red = fadd float %cur_red, %A_i
  %i_next = add nuw nsw i64 %i, 1
  %done = icmp eq i64 %i_next, 1000
  br i1 %done, label %end, label %loop

end:
  ret float %red
}

define float @subf32(float* %A) {
entry:
; CHECK-LABEL: subf32
; CHECK: redsubf32
  br label %loop

loop:
  %i = phi i64 [0, %entry], [%i_next, %loop]
  %cur_red = phi float [0.0, %entry], [%red, %loop]
  %A_idx = getelementptr inbounds float, float* %A, i64 %i
  %A_i = load float, float* %A_idx
  %red = fsub float %cur_red, %A_i
  %i_next = add nuw nsw i64 %i, 1
  %done = icmp eq i64 %i_next, 1000
  br i1 %done, label %end, label %loop

end:
  ret float %red
}

define float @mulf32(float* %A) {
entry:
; CHECK-LABEL: mulf32
  br label %loop

loop:
  %i = phi i64 [0, %entry], [%i_next, %loop]
  %cur_red = phi float [1.0, %entry], [%red, %loop]
  %A_idx = getelementptr inbounds float, float* %A, i64 %i
  %A_i = load float, float* %A_idx
  %red = fmul float %cur_red, %A_i
  %i_next = add nuw nsw i64 %i, 1
  %done = icmp eq i64 %i_next, 1000
  br i1 %done, label %end, label %loop

end:
  ret float %red
}

define float @fmaf32(float* %A) {
entry:
; CHECK-LABEL: fmaf32
; CHECK: redaddf32
  br label %loop

loop:
  %i = phi i64 [0, %entry], [%i_next, %loop]
  %cur_red = phi float [0.0, %entry], [%red, %loop]
  %A_idx = getelementptr inbounds float, float* %A, i64 %i
  %A_i = load float, float* %A_idx
  %red = call float @llvm.fma.f32(float %A_i, float 2.0, float %cur_red)
  %i_next = add nuw nsw i64 %i, 1
  %done = icmp eq i64 %i_next, 1000
  br i1 %done, label %end, label %loop

end:
  ret float %red
}

declare float @llvm.fma.f32(float, float, float)

define double @addf64(double* %A) {
entry:
; CHECK-LABEL: addf64
; CHECK: redaddf64
  br label %loop

loop:
  %i = phi i64 [0, %entry], [%i_next, %loop]
  %cur_red = phi double [0.0, %entry], [%red, %loop]
  %A_idx = getelementptr inbounds double, double* %A, i64 %i
  %A_i = load double, double* %A_idx
  %red = fadd double %cur_red, %A_i
  %i_next = add nuw nsw i64 %i, 1
  %done = icmp eq i64 %i_next, 1000
  br i1 %done, label %end, label %loop

end:
  ret double %red
}

define double @subf64(double* %A) {
entry:
; CHECK-LABEL: subf64
; CHECK: redsubf64
  br label %loop

loop:
  %i = phi i64 [0, %entry], [%i_next, %loop]
  %cur_red = phi double [0.0, %entry], [%red, %loop]
  %A_idx = getelementptr inbounds double, double* %A, i64 %i
  %A_i = load double, double* %A_idx
  %red = fsub double %cur_red, %A_i
  %i_next = add nuw nsw i64 %i, 1
  %done = icmp eq i64 %i_next, 1000
  br i1 %done, label %end, label %loop

end:
  ret double %red
}

define double @mulf64(double* %A) {
entry:
; CHECK-LABEL: mulf64
  br label %loop

loop:
  %i = phi i64 [0, %entry], [%i_next, %loop]
  %cur_red = phi double [1.0, %entry], [%red, %loop]
  %A_idx = getelementptr inbounds double, double* %A, i64 %i
  %A_i = load double, double* %A_idx
  %red = fmul double %cur_red, %A_i
  %i_next = add nuw nsw i64 %i, 1
  %done = icmp eq i64 %i_next, 1000
  br i1 %done, label %end, label %loop

end:
  ret double %red
}

define double @fmaf64(double* %A) {
entry:
; CHECK-LABEL: fmaf64
; CHECK: redaddf64
  br label %loop

loop:
  %i = phi i64 [0, %entry], [%i_next, %loop]
  %cur_red = phi double [0.0, %entry], [%red, %loop]
  %A_idx = getelementptr inbounds double, double* %A, i64 %i
  %A_i = load double, double* %A_idx
  %red = call double @llvm.fma.f64(double %A_i, double 2.0, double %cur_red)
  %i_next = add nuw nsw i64 %i, 1
  %done = icmp eq i64 %i_next, 1000
  br i1 %done, label %end, label %loop

end:
  ret double %red
}

declare double @llvm.fma.f64(double, double, double)

define <2 x float> @addf32x2(<2 x float>* %A) {
entry:
; CHECK-LABEL: addf32x2
; CHECK: redaddf32x2
  br label %loop

loop:
  %i = phi i64 [0, %entry], [%i_next, %loop]
  %cur_red = phi <2 x float> [zeroinitializer, %entry], [%red, %loop]
  %A_idx = getelementptr inbounds <2 x float>, <2 x float>* %A, i64 %i
  %A_i = load <2 x float>, <2 x float>* %A_idx
  %red = fadd <2 x float> %cur_red, %A_i
  %i_next = add nuw nsw i64 %i, 1
  %done = icmp eq i64 %i_next, 1000
  br i1 %done, label %end, label %loop

end:
  ret <2 x float> %red
}

define <2 x float> @subf32x2(<2 x float>* %A) {
entry:
; CHECK-LABEL: subf32x2
; CHECK: redsubf32x2
  br label %loop

loop:
  %i = phi i64 [0, %entry], [%i_next, %loop]
  %cur_red = phi <2 x float> [zeroinitializer, %entry], [%red, %loop]
  %A_idx = getelementptr inbounds <2 x float>, <2 x float>* %A, i64 %i
  %A_i = load <2 x float>, <2 x float>* %A_idx
  %red = fsub <2 x float> %cur_red, %A_i
  %i_next = add nuw nsw i64 %i, 1
  %done = icmp eq i64 %i_next, 1000
  br i1 %done, label %end, label %loop

end:
  ret <2 x float> %red
}

define <2 x float> @mulf32x2(<2 x float>* %A) {
entry:
; CHECK-LABEL: mulf32x2
  br label %loop

loop:
  %i = phi i64 [0, %entry], [%i_next, %loop]
  %cur_red = phi <2 x float> [<float 1.0, float 1.0>, %entry], [%red, %loop]
  %A_idx = getelementptr inbounds <2 x float>, <2 x float>* %A, i64 %i
  %A_i = load <2 x float>, <2 x float>* %A_idx
  %red = fmul <2 x float> %cur_red, %A_i
  %i_next = add nuw nsw i64 %i, 1
  %done = icmp eq i64 %i_next, 1000
  br i1 %done, label %end, label %loop

end:
  ret <2 x float> %red
}

define <2 x float> @fmaf32x2(<2 x float>* %A) {
entry:
; CHECK-LABEL: fmaf32x2
; CHECK: redaddf32x2
  br label %loop

loop:
  %i = phi i64 [0, %entry], [%i_next, %loop]
  %cur_red = phi <2 x float> [zeroinitializer, %entry], [%red, %loop]
  %A_idx = getelementptr inbounds <2 x float>, <2 x float>* %A, i64 %i
  %A_i = load <2 x float>, <2 x float>* %A_idx
  %red = call <2 x float> @llvm.fma.v2f32(<2 x float> %A_i, <2 x float> <float 1.0, float 2.0>, <2 x float> %cur_red)
  %i_next = add nuw nsw i64 %i, 1
  %done = icmp eq i64 %i_next, 1000
  br i1 %done, label %end, label %loop

end:
  ret <2 x float> %red
}

declare <2 x float> @llvm.fma.v2f32(<2 x float>, <2 x float>, <2 x float>)
