; RUN: opt -O0 -enable-matrix -debug-pass=Structure < %s -o /dev/null 2>&1 | FileCheck %s

; REQUIRES: asserts

; CHECK:      Pass Arguments:
; CHECK-NEXT: Target Transform Information
; INTEL
; CHECK-NEXT: Xmain opt level pass
; end INTEL
; CHECK-NEXT:   FunctionPass Manager
; CHECK-NEXT:     Module Verifier
; INTEL
; CHECK-NEXT:     Subscript Intrinsic Lowering
; end INTEL
; CHECK-NEXT:     Instrument function entry/exit with calls to e.g. mcount() (pre inlining)
; CHECK-NEXT:     Lower the matrix intrinsics (minimal)


define void @f() {
  ret void
}
