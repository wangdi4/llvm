; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced
; RUN: opt < %s -hir-ssa-deconstruction -hir-vec-dir-insert -hir-vplan-vec \
; RUN:     -mtriple=x86_64-unknown-unknown -mattr=+sse2 \
; RUN:     -disable-output -vplan-cost-model-print-analysis-for-vf=16 \
; RUN:     | FileCheck %s --check-prefix=CHECK-VF16-SSE2

; RUN: opt < %s -hir-ssa-deconstruction -hir-vec-dir-insert -hir-vplan-vec \
; RUN:     -mtriple=x86_64-unknown-unknown -mattr=+sse2 \
; RUN:     -disable-output -vplan-cost-model-print-analysis-for-vf=8 \
; RUN:     | FileCheck %s --check-prefix=CHECK-VF8-SSE2

; RUN: opt < %s -hir-ssa-deconstruction -hir-vec-dir-insert -hir-vplan-vec \
; RUN:     -mtriple=x86_64-unknown-unknown -mattr=+avx2 \
; RUN:     -disable-output -vplan-cost-model-print-analysis-for-vf=16 \
; RUN:     | FileCheck %s --check-prefix=CHECK-VF16-AVX2

; RUN: opt < %s -hir-ssa-deconstruction -hir-vec-dir-insert -hir-vplan-vec \
; RUN:     -mtriple=x86_64-unknown-unknown -mattr=+avx2 \
; RUN:     -disable-output -vplan-cost-model-print-analysis-for-vf=8 \
; RUN:     | FileCheck %s --check-prefix=CHECK-VF8-AVX2

; Rhe test checks that there is additional spill/fill cost for VF = 16, but no such
; penalty for VF < 16 for sse2 target, and no spill/fill penalty even for VF = 16
; for avx2 target.

; CHECK-VF16-SSE2: Extra cost due to Spill/Fill heuristic
; CHECK-VF8-SSE2-NOT: Extra cost due to Spill/Fill heuristic
; CHECK-VF16-AVX2-NOT: Extra cost due to Spill/Fill heuristic
; CHECK-VF8-AVX2-NOT: Extra cost due to Spill/Fill heuristic

@arr.double.1 = external local_unnamed_addr global [1024 x double], align 16
@arr.double.2 = external local_unnamed_addr global [1024 x double], align 16
@arr.double.3 = external local_unnamed_addr global [1024 x double], align 16
@arr.double.4 = external local_unnamed_addr global [1024 x double], align 16

define void @check_spill_1() local_unnamed_addr {
entry:
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %for.body

for.body:
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %ld1.idx = getelementptr inbounds [1024 x double], [1024 x double]* @arr.double.1, i64 0, i64 %indvars.iv
  %ld2.idx = getelementptr inbounds [1024 x double], [1024 x double]* @arr.double.2, i64 0, i64 %indvars.iv
  %st1.idx = getelementptr inbounds [1024 x double], [1024 x double]* @arr.double.3, i64 0, i64 %indvars.iv
  %st2.idx = getelementptr inbounds [1024 x double], [1024 x double]* @arr.double.4, i64 0, i64 %indvars.iv

  %ld1 = load double, double* %ld1.idx
  %ld2 = load double, double* %ld2.idx

  %val1 = fadd double %ld1, %ld2
  %val2 = fsub double %ld1, %ld2
  %val3 = fmul double %ld1, %val1
  %val4 = fdiv double %ld2, %val2

  store double %val3, double* %st1.idx
  store double %val4, double* %st2.idx

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
; end INTEL_FEATURE_SW_ADVANCED
