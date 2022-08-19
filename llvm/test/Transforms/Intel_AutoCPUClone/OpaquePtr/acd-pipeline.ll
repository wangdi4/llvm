; RUN: opt -passes='default<O2>' -debug-pass-manager < %s -o /dev/null 2>&1 | FileCheck %s
; RUN: opt -passes='default<O2>' -enable-ax -debug-pass-manager < %s -o /dev/null 2>&1 | FileCheck %s --check-prefix="CHECK-AX"
; RUN: opt -passes='lto<O2>' -debug-pass-manager < %s -o /dev/null 2>&1 | FileCheck %s --check-prefix="CHECK-AX"

; Verify that Auto CPU Dispatch is not run in the default pass pipeline unless enabled.

; CHECK-NOT: Running pass: AutoCPUClonePass on [module]

; Verify that Auto CPU Dispatch is run in the default pass pipeline when enabled.

; CHECK-AX: Running pass: AutoCPUClonePass on [module]

define void @f() {
  ret void
}
