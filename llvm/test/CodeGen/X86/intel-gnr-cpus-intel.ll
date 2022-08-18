; INTEL_FEATURE_CPU_GNR
; REQUIRES: intel_feature_cpu_gnr
; Test that the CPU names work.
; CHECK-NO-ERROR-NOT: not a recognized processor for this target

; RUN: llc < %s -o /dev/null -mtriple=x86_64-unknown-unknown -mcpu=graniterapids 2>&1 | FileCheck %s --check-prefix=CHECK-NO-ERROR --allow-empty

; Needed to ensure a subtarget object gets created
define void @foo() {
  ret void
}

; end INTEL_FEATURE_CPU_GNR