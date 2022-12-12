; RUN: SATest -BUILD -dump-kernel-property -config=%s.cfg 2>&1 | FileCheck %s

; CHECK: [Kernel properties]
; CHECK:     kernelAttributes: vec_type_hint(half)
; CHECK: [Kernel properties]
; CHECK:     kernelAttributes: vec_type_hint(half2)
