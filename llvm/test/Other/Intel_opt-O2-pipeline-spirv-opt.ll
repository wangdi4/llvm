; INTEL_CUSTOMIZATION
;RUN: opt -disable-verify -enable-new-pm=1 -passes='default<O2>' -paropt=31 -spirv-opt -debug-pass-manager -S -disable-output %s 2>&1 |  FileCheck %s
;CHECK-NOT:          Reassociate expressions

define void @foo() {
  ret void
}
; end INTEL_CUSTOMIZATION
