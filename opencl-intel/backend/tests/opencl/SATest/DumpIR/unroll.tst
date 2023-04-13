; RUN: SATest -BUILD --config=%s.cfg --dump-llvm-file - | FileCheck %s

; Check that compiler unrolls loops with and without pragma unroll.

; CHECK: = call{{.*}} float @{{.*}}sinf{{.*}}(float
; CHECK: = call{{.*}} float @{{.*}}sinf{{.*}}(float
; CHECK: = call{{.*}} float @{{.*}}sinf{{.*}}(float
; CHECK: = call{{.*}} float @{{.*}}sinf{{.*}}(float

; CHECK: = call{{.*}} float @{{.*}}cosf{{.*}}(float
; CHECK: = call{{.*}} float @{{.*}}cosf{{.*}}(float
; CHECK: = call{{.*}} float @{{.*}}cosf{{.*}}(float
; CHECK: = call{{.*}} float @{{.*}}cosf{{.*}}(float
