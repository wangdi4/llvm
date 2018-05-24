; INTEL - the test checks that optreport-options pass is not invalidated,
; RUN: opt < %s -S -passes='require<xm-optreport-options>,invalidate<xm-optreport-options>' -debug-pass-manager 2>&1 | FileCheck %s
target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"

; CHECK:     Running analysis: OptReportOptionsAnalysis
; CHECK-NOT: Invalidating analysis: OptReportOptionsAnalysis
define i32 @unroll_dce(i32* noalias nocapture readonly %b) {
entry:
  ret i32 1
}
