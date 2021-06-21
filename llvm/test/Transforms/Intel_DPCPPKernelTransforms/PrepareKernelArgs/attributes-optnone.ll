; RUN: opt -dpcpp-kernel-add-implicit-args -dpcpp-kernel-prepare-args -S %s | FileCheck %s
; RUN: opt -passes='dpcpp-kernel-add-implicit-args,dpcpp-kernel-prepare-args' -S %s | FileCheck %s

; RUN: opt -dpcpp-kernel-add-implicit-args -dpcpp-kernel-prepare-args -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes='dpcpp-kernel-add-implicit-args,dpcpp-kernel-prepare-args' -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s

; Checks that the attributes of the old kernel is copied to the wrapper. And
; for the old kernel, these attributes are also kept, and 'alwaysinline' is
; added, but 'optnone' and 'noinline' are removed.

; CHECK: define void @__test_separated_args
; CHECK-SAME: #[[ATTR0:[0-9]+]]
; CHECK: define void @test
; CHECK-SAME: #[[ATTR1:[0-9]+]]

; CHECK-DAG: attributes #[[ATTR0]] = { alwaysinline nounwind "failed-to-vectorize" "min-legal-vector-width"="0" }
; CHECK-DAG: attributes #[[ATTR1]] = { noinline nounwind optnone "failed-to-vectorize" "min-legal-vector-width"="0" }

define void @test(i32 %a) #0 {
  ret void
}

!sycl.kernels = !{!0}

!0 = !{void(i32)* @test}

attributes #0 = { noinline nounwind optnone "failed-to-vectorize" "min-legal-vector-width"="0" }

; DEBUGIFY-NOT: WARNING
; DEBUGIFY-COUNT-37: WARNING: Instruction with empty DebugLoc in function {{.*}}test{{.*}}
; DEBUGIFY-NOT: WARNING
