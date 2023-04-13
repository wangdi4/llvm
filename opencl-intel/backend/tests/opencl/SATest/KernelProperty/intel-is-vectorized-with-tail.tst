; RUN: SATest -BUILD -dump-kernel-property -config=%s.cfg 2>&1 | FileCheck %s

; Check isVectorizedWithTail is true for kernel 'foo' which has no barrier path.
; Check isVectorizedWithTail is false for kernel 'bar' which has no barrier path
; but intel_reqd_sub_group_size is set to 1.

; CHECK: [Kernel properties]
; CHECK:     isVectorizedWithTail: 1
; CHECK: [Kernel properties]
; CHECK:     isVectorizedWithTail: 0
