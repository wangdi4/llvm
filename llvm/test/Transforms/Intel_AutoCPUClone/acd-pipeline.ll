; RUN: opt -passes='default<O2>' -debug-pass-manager < %s -o /dev/null 2>&1 | FileCheck %s --check-prefix=CHECK-VEC
; RUN: opt -passes='default<O2>' -enable-ax -debug-pass-manager < %s -o /dev/null 2>&1 | FileCheck %s --check-prefixes=CHECK-AX,CHECK-VEC
; RUN: opt -passes='lto<O2>' -debug-pass-manager < %s -o /dev/null 2>&1 | FileCheck %s --check-prefixes=CHECK-AX,CHECK-VEC

; Verify that Auto CPU Dispatch is run in the default pass pipeline when enabled.

; CHECK-AX:  Running pass: AutoCPUClonePass on [module]
; CHECK-VEC: Running pass: AutoCPUClonePass on [module]

define void @f() {
  ret void
}
