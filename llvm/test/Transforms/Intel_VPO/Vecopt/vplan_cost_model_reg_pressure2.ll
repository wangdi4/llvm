; RUN: opt < %s -hir-ssa-deconstruction -hir-vec-dir-insert -hir-vplan-vec \
; RUN:     -mtriple=x86_64-unknown-unknown -mattr=+sse2 -S | FileCheck %s

; The test purpose is to verify that having high register pressure even on scalar
; VPlan doesn't block vectorization.

; CHECK: <{{[4|8]}} x float>

@arr.float.1 = external local_unnamed_addr global [1024 x float], align 16
@arr.float.2 = external local_unnamed_addr global [1024 x float], align 16
@arr.float.3 = external local_unnamed_addr global [1024 x float], align 16
@arr.float.4 = external local_unnamed_addr global [1024 x float], align 16

define void @check_spill_2() local_unnamed_addr {
entry:
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %for.body

for.body:
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %ld1.0.idx = getelementptr inbounds [1024 x float], [1024 x float]* @arr.float.1, i64 0, i64 %indvars.iv
  %ld1.1.idx = getelementptr inbounds [1024 x float], [1024 x float]* @arr.float.1, i64 1, i64 %indvars.iv
  %ld1.2.idx = getelementptr inbounds [1024 x float], [1024 x float]* @arr.float.1, i64 2, i64 %indvars.iv
  %ld1.3.idx = getelementptr inbounds [1024 x float], [1024 x float]* @arr.float.1, i64 3, i64 %indvars.iv
  %ld1.4.idx = getelementptr inbounds [1024 x float], [1024 x float]* @arr.float.1, i64 4, i64 %indvars.iv

  %ld2.0.idx = getelementptr inbounds [1024 x float], [1024 x float]* @arr.float.2, i64 0, i64 %indvars.iv
  %ld2.1.idx = getelementptr inbounds [1024 x float], [1024 x float]* @arr.float.2, i64 1, i64 %indvars.iv
  %ld2.2.idx = getelementptr inbounds [1024 x float], [1024 x float]* @arr.float.2, i64 2, i64 %indvars.iv
  %ld2.3.idx = getelementptr inbounds [1024 x float], [1024 x float]* @arr.float.2, i64 3, i64 %indvars.iv
  %ld2.4.idx = getelementptr inbounds [1024 x float], [1024 x float]* @arr.float.2, i64 4, i64 %indvars.iv

  %st1.idx = getelementptr inbounds [1024 x float], [1024 x float]* @arr.float.3, i64 0, i64 %indvars.iv
  %st2.idx = getelementptr inbounds [1024 x float], [1024 x float]* @arr.float.4, i64 0, i64 %indvars.iv

  %ld1.0 = load float, float* %ld1.0.idx
  %ld2.0 = load float, float* %ld2.0.idx
  %ld1.1 = load float, float* %ld1.1.idx
  %ld2.1 = load float, float* %ld2.1.idx
  %ld1.2 = load float, float* %ld1.2.idx
  %ld2.2 = load float, float* %ld2.2.idx
  %ld1.3 = load float, float* %ld1.3.idx
  %ld2.3 = load float, float* %ld2.3.idx
  %ld1.4 = load float, float* %ld1.4.idx
  %ld2.4 = load float, float* %ld2.4.idx

  %val1.0 = fadd float %ld1.0, %ld2.0
  %val2.0 = fsub float %ld1.0, %ld2.0
  %val1.1 = fadd float %ld1.1, %ld2.1
  %val2.1 = fsub float %ld1.1, %ld2.1
  %val1.2 = fadd float %ld1.2, %ld2.2
  %val2.2 = fsub float %ld1.2, %ld2.2
  %val1.3 = fadd float %ld1.3, %ld2.3
  %val2.3 = fsub float %ld1.3, %ld2.3
  %val1.4 = fadd float %ld1.4, %ld2.4
  %val2.4 = fsub float %ld1.4, %ld2.4

  %val3.0 = fmul float %ld1.0, %val1.0
  %val4.0 = fdiv float %ld2.0, %val2.0
  %val3.1 = fmul float %ld1.1, %val1.1
  %val4.1 = fdiv float %ld2.1, %val2.1
  %val3.2 = fmul float %ld1.2, %val1.2
  %val4.2 = fdiv float %ld2.2, %val2.2
  %val3.3 = fmul float %ld1.3, %val1.3
  %val4.3 = fdiv float %ld2.3, %val2.3
  %val3.4 = fmul float %ld1.4, %val1.4
  %val4.4 = fdiv float %ld2.4, %val2.4

  %val1.st.0  = fadd float %ld1.0, %val1.0
  %val1.st.1  = fadd float %ld1.1, %val1.st.0
  %val1.st.2  = fadd float %val1.1, %val1.st.1
  %val1.st.3  = fadd float %ld1.2, %val1.st.2
  %val1.st.4  = fadd float %val1.2, %val1.st.3
  %val1.st.5  = fadd float %ld1.3, %val1.st.4
  %val1.st.6  = fadd float %val1.3, %val1.st.5
  %val1.st.7  = fadd float %ld1.4, %val1.st.6
  %val1.st.8  = fadd float %val1.4, %val1.st.7

  %val2.st.0  = fadd float %ld2.0, %val2.0
  %val2.st.1  = fadd float %ld2.1, %val2.st.0
  %val2.st.2  = fadd float %val2.1, %val2.st.1
  %val2.st.3  = fadd float %ld2.2, %val2.st.2
  %val2.st.4  = fadd float %val2.2, %val2.st.3
  %val2.st.5  = fadd float %ld2.3, %val2.st.4
  %val2.st.6  = fadd float %val2.3, %val2.st.5
  %val2.st.7  = fadd float %ld2.4, %val2.st.6
  %val2.st.8  = fadd float %val2.4, %val2.st.7

  store float %val1.st.8, float* %st1.idx
  store float %val2.st.8, float* %st2.idx

  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"()]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #1 = { nounwind }
