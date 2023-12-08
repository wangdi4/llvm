; RUN: opt -passes=sycl-kernel-wg-loop-bound %s -S -enable-debugify -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY
; RUN: opt -passes=sycl-kernel-wg-loop-bound %s -S -debug -disable-output 2>&1 | FileCheck %s

; The source code for the test is:
; __kernel void test(__global float* a, long uniform1, long uniform2) {
;     size_t lid = get_local_id(0);
;     if (lid + uniform1 <= uniform2) return;
;     a[lid] = 3.f;
; }

; CHECK: WGLoopBoundaries
; CHECK: found 0 early exit boundaries
; CHECK: found 0 uniform early exit conditions

target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v16:16:16-v24:32:32-v32:32:32-v48:64:64-v64:64:64-v96:128:128-v128:128:128-v192:256:256-v256:256:256-v512:512:512-v1024:1024:1024-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; Function Attrs: nounwind
define void @test(ptr addrspace(1) %a, i64 %uniform1, i64 %uniform2) #0 !no_barrier_path !1 {
  %1 = call i64 @_Z12get_local_idj(i32 0) #2
  %2 = add i64 %1, %uniform1
  %3 = icmp ugt i64 %2, %uniform2
  br i1 %3, label %4, label %6

; <label>:4                                       ; preds = %0
  %5 = getelementptr inbounds float, ptr addrspace(1) %a, i64 %1
  store float 3.000000e+00, ptr addrspace(1) %5, align 4
  br label %6

; <label>:6                                       ; preds = %0, %4
  ret void
}

; Function Attrs: nounwind readnone
declare i64 @_Z12get_local_idj(i32) #1

attributes #0 = { nounwind "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-frame-pointer-elim-non-leaf"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-frame-pointer-elim-non-leaf"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind readnone }

!sycl.kernels = !{!0}

!0 = !{ptr @test}
!1 = !{i1 true}

; DEBUGIFY-NOT: WARNING
