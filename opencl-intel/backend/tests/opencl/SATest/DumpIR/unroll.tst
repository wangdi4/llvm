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
