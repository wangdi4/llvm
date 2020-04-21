; RUN: opt -O0 -enable-dpcpp-kernel-transforms=true -debug-pass=Structure < %s -o /dev/null 2>&1 | FileCheck %s

; REQUIRES: asserts

; CHECK-LABEL: Pass Arguments:
; CHECK-NEXT: Target Transform Information
; CHECK-NEXT: Xmain opt level pass
; CHECK-NEXT:   FunctionPass Manager
; CHECK-LABEL: Pass Arguments:
; CHECK-NEXT: Target Library Information
; CHECK-NEXT: Target Transform Information
; CHECK-NEXT: Xmain opt level pass
; CHECK-NEXT: Assumption Cache Tracker
; CHECK-NEXT: Profile summary info
; CHECK-NEXT:   ModulePass Manager
; CHECK:          Parse annotations and add a corresponding function attribute
; CHECK-NEXT:     FunctionPass Manager
; CHECK-NEXT:       Unify function exit nodes
; CHECK-NEXT:     WGLoopCreator
; CHECK-NEXT:       FunctionPass Manager
; CHECK-NEXT:         Unify function exit nodes

define void @f() {
  ret void
}
