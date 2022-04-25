; Test that CallGraphAnalysis is invalidated.

; RUN: opt -disable-output -disable-verify -debug-pass-manager %s 2>&1 \
; RUN:     -passes='dpcpp-kernel-add-implicit-args,dpcpp-kernel-resolve-wi-call' \
; RUN:     | FileCheck %s

; CHECK: Running pass: ResolveWICallPass
; CHECK: Running analysis: CallGraphAnalysis
; CHECK: Invalidating analysis: CallGraphAnalysis

; RUN: opt -enable-new-pm=0 -disable-output -disable-verify -debug-pass=Details %s 2>&1 \
; RUN:     -dpcpp-kernel-add-implicit-args -dpcpp-kernel-resolve-wi-call \
; RUN:     | FileCheck %s --check-prefix=CHECK-LEGACY

; CHECK-LEGACY: 'ResolveWICallLegacy' is not preserving 'CallGraph Construction'

define void @foo() {
entry:
  ret void
}
