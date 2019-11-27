; This test verifies that anders-aa should be able to handle
; FNeg instruction without bailing out early.

; RUN: opt < %s -anders-aa -print-anders-points-to -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -passes='require<anders-aa>' -print-anders-points-to -disable-output 2>&1 | FileCheck %s

; CHECK: foo:r  --> (0): <universal>

define <4 x float> @foo() {
  %r = fneg <4 x float> <float -0.0, float undef, float undef, float 1.0>
  ret <4 x float> %r
}

