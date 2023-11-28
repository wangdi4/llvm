; RUN: opt -passes=auto-cpu-clone -generate-vector-variants -acd-enable-all < %s -S | FileCheck %s

; This test checks that AutoCPUClone pass does not modify spir64 modules.

; CHECK: define spir_func i32 @foo()
; CHECK-NOT: foo.A

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

@foo.alias = internal alias i32 (...), addrspacecast (i32 (...)* bitcast (i32 ()* @foo to i32 (...)*) to i32 (...) addrspace(1)*)

define spir_func i32 @foo() {
  ret i32 0
}
