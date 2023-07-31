; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced
; RUN: opt < %s -disable-output -passes=vplan-vec -mtriple=x86_64-unknown-unknown -mattr=+avx2 -vplan-cost-model-print-analysis-for-vf=1 | FileCheck %s

; Check that FP inductions do not contribute into GPR register pressure.
; To control that we check CM dump output for VF=1. It should not contain
; report for spills.

; CHECK-NOT: Extra cost due to Spill/Fill heuristic

@arr1 = external local_unnamed_addr global [16 x float], align 16
@arr2 = external local_unnamed_addr global [16 x float], align 16
@arr3 = external local_unnamed_addr global [16 x float], align 16
@arr4 = external local_unnamed_addr global [16 x float], align 16
@arr5 = external local_unnamed_addr global [16 x float], align 16
@arr6 = external local_unnamed_addr global [16 x float], align 16
@arr7 = external local_unnamed_addr global [16 x float], align 16
@arr8 = external local_unnamed_addr global [16 x float], align 16

define void @check_spill_1() local_unnamed_addr {
entry:
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %for.body

for.body:
  %iv = phi i64 [ 0, %entry ], [ %iv.next, %for.body ]
  %fv1 = phi float [ 0., %entry ], [ %fv1.next, %for.body ]
  %fv2 = phi float [ 0., %entry ], [ %fv2.next, %for.body ]
  %fv3 = phi float [ 0., %entry ], [ %fv3.next, %for.body ]
  %fv4 = phi float [ 0., %entry ], [ %fv4.next, %for.body ]
  %fv5 = phi float [ 0., %entry ], [ %fv5.next, %for.body ]
  %fv6 = phi float [ 0., %entry ], [ %fv6.next, %for.body ]
  %fv7 = phi float [ 0., %entry ], [ %fv7.next, %for.body ]
  %fv8 = phi float [ 0., %entry ], [ %fv8.next, %for.body ]

  %idx1 = getelementptr inbounds [16 x float], ptr @arr1, i64 0, i64 %iv
  %idx2 = getelementptr inbounds [16 x float], ptr @arr2, i64 0, i64 %iv
  %idx3 = getelementptr inbounds [16 x float], ptr @arr3, i64 0, i64 %iv
  %idx4 = getelementptr inbounds [16 x float], ptr @arr4, i64 0, i64 %iv
  %idx5 = getelementptr inbounds [16 x float], ptr @arr5, i64 0, i64 %iv
  %idx6 = getelementptr inbounds [16 x float], ptr @arr6, i64 0, i64 %iv
  %idx7 = getelementptr inbounds [16 x float], ptr @arr7, i64 0, i64 %iv
  %idx8 = getelementptr inbounds [16 x float], ptr @arr8, i64 0, i64 %iv

  %ld1 = load float, ptr %idx1
  %ld2 = load float, ptr %idx2
  %ld3 = load float, ptr %idx3
  %ld4 = load float, ptr %idx4
  %ld5 = load float, ptr %idx5
  %ld6 = load float, ptr %idx6
  %ld7 = load float, ptr %idx7
  %ld8 = load float, ptr %idx8

  %v1 = fadd float %ld1, %fv1
  %v2 = fadd float %ld1, %fv2
  %v3 = fadd float %ld1, %fv3
  %v4 = fadd float %ld1, %fv4
  %v5 = fadd float %ld1, %fv5
  %v6 = fadd float %ld1, %fv6
  %v7 = fadd float %ld1, %fv7
  %v8 = fadd float %ld1, %fv8

  store float %v1, ptr %idx1
  store float %v2, ptr %idx2
  store float %v3, ptr %idx3
  store float %v4, ptr %idx4
  store float %v5, ptr %idx5
  store float %v6, ptr %idx6
  store float %v7, ptr %idx7
  store float %v8, ptr %idx8

  %iv.next = add nuw nsw i64 %iv, 1
  %fv1.next = fadd float %fv1, 1.
  %fv2.next = fadd float %fv2, 1.
  %fv3.next = fadd float %fv3, 1.
  %fv4.next = fadd float %fv4, 1.
  %fv5.next = fadd float %fv5, 1.
  %fv6.next = fadd float %fv6, 1.
  %fv7.next = fadd float %fv7, 1.
  %fv8.next = fadd float %fv8, 1.

  %exitcond = icmp eq i64 %iv.next, 16
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
