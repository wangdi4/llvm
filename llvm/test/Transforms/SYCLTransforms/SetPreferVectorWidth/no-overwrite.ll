; RUN: opt -sycl-force-prefer-vector-width=256 -passes=sycl-kernel-set-prefer-vector-width -S %s | FileCheck %s
; RUN: opt -sycl-force-prefer-vector-width=256 -passes=sycl-kernel-set-prefer-vector-width -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; Check that the pass won't overwrite existing "prefer-vector-width".

define void @foo() #0 {
; CHECK-LABEL: define void @foo
; CHECK-SAME: [[ATTR:#[0-9]+]]
  ret void
}

declare void @bar() #0
; CHECK-LABEL: declare void @bar
; CHECK-SAME: [[ATTR]]

attributes #0 = { nounwind willreturn "prefer-vector-width"="512" }
; CHECK: attributes [[ATTR]] =
; CHECK-SAME: "prefer-vector-width"="512"

; DEBUGIFY-NOT: WARNING
