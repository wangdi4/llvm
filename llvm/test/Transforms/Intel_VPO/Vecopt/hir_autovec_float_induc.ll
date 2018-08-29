; RUN: opt < %s  -O3 -enable-lv -mcpu=core-avx2 -mtriple=x86_64-unknown-linux-gnu -S | FileCheck --check-prefix AUTO_VEC %s
; This test copied from LoopVectorize/X86.
; It started failing when the "external_use" functions were added.
; We changed the expected code sequences. The original test is marked as *XFAIL*
; for the xmain.

; This test checks auto-vectorization with FP induction variable.
; The FP operation is not "fast" and requires "fast-math" function attribute.

;void fp_iv_loop1(float * __restrict__ A, int N) {
;  float x = 1.0;
;  for (int i=0; i < N; ++i) {
;    A[i] = x;
;    x += 0.5;
;  }
;}

; This test doesn't work correctly with loopopt. It should be enabled when fp IV support is enabled.
; XFAIL: *


; AUTO_VEC-LABEL: @fp_iv_loop1(
; AUTO_VEC: vector.body
; AUTO_VEC: store <8 x float>

define void @fp_iv_loop1(float* noalias nocapture %A, i32 %N) #0 {
entry:
  %cmp4 = icmp sgt i32 %N, 0
  br i1 %cmp4, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.body
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body ], [ 0, %for.body.preheader ]
  %x.06 = phi float [ %conv1, %for.body ], [ 1.000000e+00, %for.body.preheader ]
  %arrayidx = getelementptr inbounds float, float* %A, i64 %indvars.iv
  store float %x.06, float* %arrayidx, align 4
  %conv1 = fadd float %x.06, 5.000000e-01
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next to i32
  %exitcond = icmp eq i32 %lftr.wideiv, %N
  br i1 %exitcond, label %for.end.loopexit, label %for.body

for.end.loopexit:                                 ; preds = %for.body
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  ret void
}

; The same as the previous, FP operation is not fast, different function attribute
; Vectorization should be rejected.
;void fp_iv_loop2(float * __restrict__ A, int N) {
;  float x = 1.0;
;  for (int i=0; i < N; ++i) {
;    A[i] = x;
;    x += 0.5;
;  }
;}

; AUTO_VEC-LABEL: @fp_iv_loop2(
; AUTO_VEC-NOT: vector.body
; AUTO_VEC-NOT: store <{{.*}} x float>

define void @fp_iv_loop2(float* noalias nocapture %A, i32 %N) #1 {
entry:
  %cmp4 = icmp sgt i32 %N, 0
  br i1 %cmp4, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.body
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body ], [ 0, %for.body.preheader ]
  %x.06 = phi float [ %conv1, %for.body ], [ 1.000000e+00, %for.body.preheader ]
  %arrayidx = getelementptr inbounds float, float* %A, i64 %indvars.iv
  store float %x.06, float* %arrayidx, align 4
  %conv1 = fadd float %x.06, 5.000000e-01
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next to i32
  %exitcond = icmp eq i32 %lftr.wideiv, %N
  br i1 %exitcond, label %for.end.loopexit, label %for.body

for.end.loopexit:                                 ; preds = %for.body
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  ret void
}

; begin INTEL_CUSTOMIZATION
; VPO  vectorizer generates a different sequence
; AUTO_VEC-LABEL: @external_use_with_fast_math(
; AUTO_VEC-NEXT:  entry:
; AUTO_VEC-NEXT:    [[TMP0:%.*]] = icmp sgt i64 %n, 1
; AUTO_VEC-NEXT:    [[SMAX:%.*]] = select i1 [[TMP0]], i64 %n, i64 1
; AUTO_VEC:         br i1 {{.*}}
; AUTO_VEC:       vector.ph:
; AUTO_VEC-NEXT:    [[N_VEC:%.*]] = and i64 {{.*}}, -16
; AUTO_VEC:         br i1 {{.*}}
; AUTO_VEC:       middle.block:
; AUTO_VEC:         [[TMP11:%.*]] = add i64 [[N_VEC]], -1
; AUTO_VEC-NEXT:    [[CAST_CMO:%.*]] = sitofp i64 [[TMP11]] to double
; AUTO_VEC-NEXT:    [[TMP12:%.*]] = fmul fast double [[CAST_CMO]], 3.000000e+00
; AUTO_VEC-NEXT:    fadd fast double {{.*}}, [[TMP12:%.*]]
; AUTO_VEC-NEXT:    br i1 {{.*}}
; AUTO_VEC:  ifmerge
; AUTO_VEC:    [[J_LCSSA:%.*]] = phi double
; AUTO_VEC:    ret double [[J_LCSSA]]
; end INTEL_CUSTOMIZATION
define double @external_use_with_fast_math(double* %a, i64 %n) {
entry:
  br label %for.body

for.body:
  %i = phi i64 [ 0, %entry ], [%i.next, %for.body]
  %j = phi double [ 0.0, %entry ], [ %j.next, %for.body ]
  %tmp0 = getelementptr double, double* %a, i64 %i
  store double %j, double* %tmp0
  %i.next = add i64 %i, 1
  %j.next = fadd fast double %j, 3.0
  %cond = icmp slt i64 %i.next, %n
  br i1 %cond, label %for.body, label %for.end

for.end:
  %tmp1 = phi double [ %j, %for.body ]
  ret double %tmp1
}

; begin INTEL_CUSTOMIZATION
; VPO  vectorizer generates a different sequence
; AUTO_VEC-LABEL: @external_use_without_fast_math(
; AUTO_VEC:    br label %[[LoopUnr:.*]]
; AUTO_VEC:  [[LoopUnr]]:
; AUTO_VEC:    %[[Sum:.*]] = phi double
; AUTO_VEC:    store double %[[Sum]]
; AUTO_VEC:    %[[Sum1:.*]] = fadd double %[[Sum]], 3.000000e+00
; AUTO_VEC:    store
; AUTO_VEC:    %[[Sum2:.*]] = fadd double %[[Sum1]], 3.000000e+00
; AUTO_VEC:    store

; AUTO_VEC:    br label %[[Loop:.*]]
; AUTO_VEC:   [[Loop]]:
; AUTO_VEC:      [[J:%.*]] = phi double [ {{.*}} ], [ [[J_NEXT:%.*]], %[[Loop]] ]
; AUTO_VEC:      [[J_NEXT]] = fadd double [[J]], 3.000000e+00
; AUTO_VEC:      br i1 {{.*}}, label %[[Loop]], label %[[ForEnd:.*]]
; AUTO_VEC:   [[ForEnd]]:
; AUTO_VEC:        %[[Res:.*]] = phi double
; AUTO_VEC-NEXT:    ret double %[[Res]]
; end INTEL_CUSTOMIZATION

define double @external_use_without_fast_math(double* %a, i64 %n) {
entry:
  br label %for.body

for.body:
  %i = phi i64 [ 0, %entry ], [%i.next, %for.body]
  %j = phi double [ 0.0, %entry ], [ %j.next, %for.body ]
  %tmp0 = getelementptr double, double* %a, i64 %i
  store double %j, double* %tmp0
  %i.next = add i64 %i, 1
  %j.next = fadd double %j, 3.0
  %cond = icmp slt i64 %i.next, %n
  br i1 %cond, label %for.body, label %for.end

for.end:
  %tmp1 = phi double [ %j, %for.body ]
  ret double %tmp1
}

attributes #0 = { "no-nans-fp-math"="true" }
attributes #1 = { "no-nans-fp-math"="false" }
