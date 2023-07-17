; RUN: opt -passes='require<sycl-kernel-builtin-info-analysis>,print<sycl-kernel-work-item-analysis>' %s -disable-output 2>&1 | FileCheck %s

;kernel void test_fmul(global int *in, global float *out) {
;  int gid = get_global_id(0);
;  float x = gid * 10 + 2;
;  float y = 12;
;  int i = 0;
;  float z;
;
;  for (i = 0; i < 2000; i++) {
;    if (i > 300) {
;      x += x;
;    }
;  }
;
;  z = x * y;
;
;  out[gid] = z;
;
;  z = z * 100;
;  out[gid + 10] = z;
;}

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; CHECK: WorkItemAnalysis for function test_fmul:
; CHECK-NEXT: SEQ   %call = tail call i64 @_Z13get_global_idj(i32 0) #2
; CHECK-NEXT: SEQ   %conv = trunc i64 %call to i32
; CHECK-NEXT: STR   %mul = mul nsw i32 %conv, 10
; CHECK-NEXT: STR   %add = add nsw i32 %mul, 2
; CHECK-NEXT: STR   %conv1 = sitofp i32 %add to float
; CHECK-NEXT: UNI   br label %for.body
; CHECK-NEXT: STR   %x.016 = phi float [ %conv1, %entry ], [ %spec.select, %for.body ]
; CHECK-NEXT: UNI   %i.015 = phi i32 [ 0, %entry ], [ %inc, %for.body ]
; CHECK-NEXT: UNI   %cmp3 = icmp ugt i32 %i.015, 300
; CHECK-NEXT: STR   %add5 = select i1 %cmp3, float %x.016, float -0.000000e+00
; CHECK-NEXT: STR   %spec.select = fadd float %x.016, %add5
; CHECK-NEXT: UNI   %inc = add nuw nsw i32 %i.015, 1
; CHECK-NEXT: UNI   %exitcond.not = icmp eq i32 %inc, 2000
; CHECK-NEXT: UNI   br i1 %exitcond.not, label %for.end, label %for.body
; CHECK-NEXT: STR   %mul6 = fmul float %spec.select, 1.200000e+01
; CHECK-NEXT: STR   %sext = shl i64 %call, 32
; CHECK-NEXT: SEQ   %idxprom = ashr exact i64 %sext, 32
; CHECK-NEXT: PTR   %arrayidx = getelementptr inbounds float, ptr addrspace(1) %out, i64 %idxprom
; CHECK-NEXT: RND   store float %mul6, ptr addrspace(1) %arrayidx, align 4, !tbaa
; CHECK-NEXT: STR   %mul7 = fmul float %mul6, 1.000000e+02
; CHECK-NEXT: SEQ   %add8 = add nsw i32 %conv, 10
; CHECK-NEXT: SEQ   %idxprom9 = sext i32 %add8 to i64
; CHECK-NEXT: PTR   %arrayidx10 = getelementptr inbounds float, ptr addrspace(1) %out, i64 %idxprom9
; CHECK-NEXT: RND   store float %mul7, ptr addrspace(1) %arrayidx10, align 4, !tbaa
; CHECK-NEXT: UNI   ret void

; Function Attrs: convergent norecurse nounwind
define dso_local void @test_fmul(ptr addrspace(1) %in, ptr addrspace(1) %out) local_unnamed_addr #0 !kernel_arg_base_type !6 !arg_type_null_val !7 {
entry:
  %call = tail call i64 @_Z13get_global_idj(i32 0) #2
  %conv = trunc i64 %call to i32
  %mul = mul nsw i32 %conv, 10
  %add = add nsw i32 %mul, 2
  %conv1 = sitofp i32 %add to float
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %x.016 = phi float [ %conv1, %entry ], [ %spec.select, %for.body ]
  %i.015 = phi i32 [ 0, %entry ], [ %inc, %for.body ]
  %cmp3 = icmp ugt i32 %i.015, 300
  %add5 = select i1 %cmp3, float %x.016, float -0.000000e+00
  %spec.select = fadd float %x.016, %add5
  %inc = add nuw nsw i32 %i.015, 1
  %exitcond.not = icmp eq i32 %inc, 2000
  br i1 %exitcond.not, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  %mul6 = fmul float %spec.select, 1.200000e+01
  %sext = shl i64 %call, 32
  %idxprom = ashr exact i64 %sext, 32
  %arrayidx = getelementptr inbounds float, ptr addrspace(1) %out, i64 %idxprom
  store float %mul6, ptr addrspace(1) %arrayidx, align 4, !tbaa !2
  %mul7 = fmul float %mul6, 1.000000e+02
  %add8 = add nsw i32 %conv, 10
  %idxprom9 = sext i32 %add8 to i64
  %arrayidx10 = getelementptr inbounds float, ptr addrspace(1) %out, i64 %idxprom9
  store float %mul7, ptr addrspace(1) %arrayidx10, align 4, !tbaa !2
  ret void
}

; Function Attrs: convergent mustprogress nofree nounwind readnone willreturn
declare i64 @_Z13get_global_idj(i32) local_unnamed_addr #1

attributes #0 = { convergent norecurse nounwind "frame-pointer"="none" "min-legal-vector-width"="0" "no-trapping-math"="true" "prefer-vector-width"="512" "stack-protector-buffer-size"="8" "stackrealign" "uniform-work-group-size"="false" }
attributes #1 = { convergent mustprogress nofree nounwind readnone willreturn "frame-pointer"="none" "no-trapping-math"="true" "prefer-vector-width"="512" "stack-protector-buffer-size"="8" "stackrealign" }
attributes #2 = { convergent nounwind readnone willreturn }

!opencl.ocl.version = !{!0}
!opencl.spir.version = !{!0}
!sycl.kernels = !{!1}

!0 = !{i32 2, i32 0}
!1 = !{ptr @test_fmul}
!2 = !{!3, !3, i64 0}
!3 = !{!"float", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
!6 = !{!"int*", !"int*"}
!7 = !{ptr addrspace(1) null, ptr addrspace(1) null}
