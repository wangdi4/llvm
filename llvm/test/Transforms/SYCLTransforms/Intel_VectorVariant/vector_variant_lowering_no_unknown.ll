; RUN: opt -passes=sycl-kernel-vector-variant-lowering -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-vector-variant-lowering -S %s | FileCheck %s

; Check that vector-variants are not updated if 'unknown' isn't present.

; IR is dumped before VectorVariantLowering pass when compiling following kernel:
;   kernel void foo(global float *a, global float *b) {
;     float sum = 0.f;
;     for (int i = 0; i < 4096; ++i)
;       sum += sin(a[i]);
;     *b = sum;
;   }

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

define dso_local void @foo(ptr addrspace(1) nocapture noundef readonly align 4 %a, ptr addrspace(1) nocapture noundef writeonly align 4 %b) {
entry:
  br label %simd.begin.region

simd.end.region:                                  ; preds = %for.body
  call void @llvm.directive.region.exit(token %region.entry) [ "DIR.OMP.END.SIMD"() ]
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %simd.end.region
  store float %add, ptr addrspace(1) %b, align 4, !tbaa !1
  ret void

simd.begin.region:                                ; preds = %entry
  %region.entry = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 16) ]
  br label %for.body

for.body:                                         ; preds = %for.body, %simd.begin.region
  %indvars.iv = phi i64 [ 0, %simd.begin.region ], [ %indvars.iv.next, %for.body ]
  %sum.04 = phi float [ 0.000000e+00, %simd.begin.region ], [ %add, %for.body ]
  %arrayidx = getelementptr inbounds float, ptr addrspace(1) %a, i64 %indvars.iv
  %0 = load float, ptr addrspace(1) %arrayidx, align 4, !tbaa !1
  %call = tail call float @_Z3sinf(float noundef %0) #1
  %add = fadd float %sum.04, %call
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 4096
  br i1 %exitcond.not, label %simd.end.region, label %for.body
}

declare float @_Z3sinf(float noundef) local_unnamed_addr

declare token @llvm.directive.region.entry() #0

declare void @llvm.directive.region.exit(token) #0

; CHECK: attributes #{{.*}} "vector-variants"="_ZGVbM16v__Z3sinf(_Z3sinDv16_fS_),_ZGVbN16v__Z3sinf(_Z3sinDv16_f)"

attributes #0 = { nounwind }
attributes #1 = { "call-params-num"="1" "vector-variants"="_ZGVbM16v__Z3sinf(_Z3sinDv16_fS_),_ZGVbN16v__Z3sinf(_Z3sinDv16_f)" }

!sycl.kernels = !{!0}

!0 = !{ptr @foo}
!1 = !{!2, !2, i64 0}
!2 = !{!"float", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}

; DEBUGIFY-NOT: WARNING
