; Test that CallGraphAnalysis is invalidated.

; RUN: opt -disable-output -disable-verify -debug-pass-manager %s 2>&1 \
; RUN:     -passes='sycl-kernel-add-implicit-args,sycl-kernel-resolve-wi-call' \
; RUN:     | FileCheck %s

; CHECK: Running pass: ResolveWICallPass
; CHECK: Running analysis: CallGraphAnalysis
; CHECK: Invalidating analysis: CallGraphAnalysis

define void @foo() {
entry:
  ret void
}
