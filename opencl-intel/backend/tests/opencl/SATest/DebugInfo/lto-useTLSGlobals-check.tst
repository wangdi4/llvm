; RUN: SATest -BUILD -pass-manager-type=lto-legacy -config=%s.cfg 2>&1 | FileCheck %s
; RUN: SATest -BUILD -config=%s.cfg 2>&1 | FileCheck %s

; This test checks that useTLSGlobals inside CPUProgramBuilder::CreateKernels
; is correct.

; CHECK: Test program was successfully built.
