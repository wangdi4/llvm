; RUN: %oclopt -add-implicit-args -prepare-kernel-args -S %s | FileCheck %s

; Checks that the attributes of the old kernel is moved to the wrapper, and
; attributes of the former one is set to 'alwaysinline'.

; CHECK: define void @__test_separated_args
; CHECK-SAME: #[[ATTR0:[0-9]+]]
; CHECK: define void @test
; CHECK-SAME: #[[ATTR1:[0-9]+]]

; CHECK-DAG: attributes #[[ATTR0]] = { alwaysinline }
; CHECK-DAG: attributes #[[ATTR1]] = { nounwind "failed-to-vectorize" }

define void @test(i32 %a) #0 {
  ret void
}

!opencl.kernels = !{!0}

!0 = !{void(i32)* @test}

attributes #0 = { nounwind "failed-to-vectorize" }
