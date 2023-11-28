; This test verifies that VPlan will strictly prefer vector library based vectorization
; over vector-variants if either options are viable for vectorizing given scalar call.

; RUN: opt -S < %s -passes='hir-ssa-deconstruction,hir-vplan-vec,print<hir>' \
; RUN:   -vplan-enable-peeling -vplan-vec-scenario='s1;v2;v2s1' --vector-library=SVML \
; RUN:   -disable-output -vplan-enable-peel-rem-strip=0 2>&1 | FileCheck %s --check-prefix=NOMASK

; NOMASK-LABEL: DO i1 = {{.*}} <vector-peel>
; NOMASK:         @__svml_sincos1
; NOMASK-NOT:     @sincos
; NOMASK:       END LOOP

; NOMASK-LABEL: DO i1 = {{.*}} <simd-vectorized>
; NOMASK:         @__svml_sincos2
; NOMASK-NOT:     @_ZGVbN2vvv_sincos
; NOMASK:       END LOOP

; NOMASK-LABEL: DO i1 = {{.*}} <vector-remainder>
; NOMASK:         @__svml_sincos2
; NOMASK-NOT:     @_ZGVbN2vvv_sincos
; NOMASK:       END LOOP

; NOMASK-LABEL: DO i1 = {{.*}} <vector-remainder>
; NOMASK:         @__svml_sincos1
; NOMASK-NOT:     @sincos
; NOMASK:       END LOOP

; RUN: opt -S < %s -passes='hir-ssa-deconstruction,hir-vplan-vec,print<hir>' \
; RUN:   -vplan-enable-peeling -vplan-vec-scenario='s1;v2;m2' --vector-library=SVML \
; RUN:   -disable-output -vplan-enable-peel-rem-strip=0 2>&1 | FileCheck %s --check-prefix=MASKREM

; MASKREM-LABEL: DO i1 = {{.*}} <vector-peel>
; MASKREM:         @__svml_sincos1
; MASKREM-NOT:     @sincos
; MASKREM:       END LOOP

; MASKREM-LABEL: DO i1 = {{.*}} <simd-vectorized>
; MASKREM:         @__svml_sincos2
; MASKREM-NOT:     @_ZGVbN2vvv_sincos
; MASKREM:       END LOOP

; MASKREM-LABEL: DO i1 = {{.*}} <vector-remainder>
; MASKREM:         @__svml_sincos2_mask
; MASKREM-NOT:     @_ZGVbM2vvv_sincos
; MASKREM:       END LOOP

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@sin.arr = external global [256 x double], align 16
@cos.arr = external global [256 x double], align 16

define dso_local void @test_sincos_peel_remainder(i64 %n) {
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %for.preheader

for.preheader:
  br label %for.body

for.body:
  %iv = phi i64 [ %n, %for.preheader ], [ %iv.next, %for.body ]

  %sin = getelementptr inbounds [256 x double], ptr @sin.arr, i64 0, i64 %iv
  %cos = getelementptr inbounds [256 x double], ptr @cos.arr, i64 0, i64 %iv

  call void @sincos(double 1.0, ptr %sin, ptr %cos)

  %iv.next = add nuw nsw i64 %iv, 1
  %cmp = icmp ult i64 %iv.next, 256
  br i1 %cmp, label %for.body, label %for.end

for.end:
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token %0)

; Function Attrs: nofree nounwind
declare dso_local void @sincos(double, ptr, ptr) local_unnamed_addr #1

attributes #1 = { nofree nounwind "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" "vector-variants"="_ZGVbN2vvv_sincos,_ZGVcN4vvv_sincos,_ZGVdN4vvv_sincos,_ZGVeN8vvv_sincos,_ZGVbM2vvv_sincos,_ZGVcM4vvv_sincos,_ZGVdM4vvv_sincos,_ZGVeM8vvv_sincos" }
