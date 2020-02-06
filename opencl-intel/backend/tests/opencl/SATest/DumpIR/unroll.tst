; RUN: SATest -BUILD --config=%s.cfg --dump-llvm-file - | FileCheck %s

; Check that compiler unrolls loops with and without pragma unroll.

; CHECK: call {{.*}}ocl_svml{{.*}}sinf1
; CHECK: call {{.*}}ocl_svml{{.*}}sinf1
; CHECK: call {{.*}}ocl_svml{{.*}}sinf1
; CHECK: call {{.*}}ocl_svml{{.*}}sinf1

; CHECK: call {{.*}}ocl_svml{{.*}}cosf1
; CHECK: call {{.*}}ocl_svml{{.*}}cosf1
; CHECK: call {{.*}}ocl_svml{{.*}}cosf1
; CHECK: call {{.*}}ocl_svml{{.*}}cosf1

; XFAIL: *
; INTEL_CUSTOMIZATION
; Force fail to avoid unexpected passes.
; See CORC-7259
; CHECK: Force fail
; end INTEL_CUSTOMIZATION
