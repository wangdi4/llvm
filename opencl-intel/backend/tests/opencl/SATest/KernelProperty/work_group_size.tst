; RUN: SATest -BUILD -build-iterations=1 -dump-kernel-property -config=%s.cfg 2>&1 | FileCheck %s

; This test checks that kernel properties hintWGSize and reqdWGSize are correct.

; CHECK: [Kernel properties]
; CHECK:      reqdWGSize: {0, 0, 0}
; CHECK-NEXT: hintWGSize: {8, 1, 1}
; CHECK: [Kernel properties]
; CHECK:      reqdWGSize: {0, 0, 0}
; CHECK-NEXT: hintWGSize: {8, 16, 1}
; CHECK: [Kernel properties]
; CHECK:      reqdWGSize: {0, 0, 0}
; CHECK-NEXT: hintWGSize: {8, 16, 32}
; CHECK: [Kernel properties]
; CHECK:      reqdWGSize: {8, 8, 8}
; CHECK-NEXT: hintWGSize: {0, 0, 0}
