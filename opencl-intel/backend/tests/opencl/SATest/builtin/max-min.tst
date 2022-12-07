; Check that int v32 and v64 builtins are used.

; RUN: SATest -BUILD --config=%s.cfg -tsize=4 -cpuarch="skx" -llvm-option=-print-after=vplan-vec 2>&1 | FileCheck %s -check-prefixes=CHECK,CHECK4
; RUN: SATest -BUILD --config=%s.cfg -tsize=8 -cpuarch="skx" -llvm-option=-print-after=vplan-vec 2>&1 | FileCheck %s -check-prefixes=CHECK,CHECK8
; RUN: SATest -BUILD --config=%s.cfg -tsize=16 -cpuarch="skx" -llvm-option=-print-after=vplan-vec 2>&1 | FileCheck %s -check-prefixes=CHECK,CHECK16
; RUN: SATest -BUILD --config=%s.cfg -tsize=32 -cpuarch="skx" -llvm-option=-print-after=vplan-vec 2>&1 | FileCheck %s -check-prefixes=CHECK,CHECK32
; RUN: SATest -BUILD --config=%s.cfg -tsize=64 -cpuarch="skx" -llvm-option=-print-after=vplan-vec 2>&1 | FileCheck %s -check-prefixes=CHECK,CHECK64

; CHECK4: call <4 x i32> @_Z3maxDv4_iS_(<4 x i32> {{.*}}, <4 x i32> {{.*}})
; CHECK4: call <4 x i32> @_Z3minDv4_iS_(<4 x i32> {{.*}}, <4 x i32> {{.*}})

; CHECK8: call <8 x i32> @_Z3maxDv8_iS_(<8 x i32> {{.*}}, <8 x i32> {{.*}})
; CHECK8: call <8 x i32> @_Z3minDv8_iS_(<8 x i32> {{.*}}, <8 x i32> {{.*}})

; CHECK16: call <16 x i32> @_Z3maxDv16_iS_(<16 x i32> {{.*}}, <16 x i32> {{.*}})
; CHECK16: call <16 x i32> @_Z3minDv16_iS_(<16 x i32> {{.*}}, <16 x i32> {{.*}})

; CHECK32: call <32 x i32> @_Z3maxDv32_iS_(<32 x i32> {{.*}}, <32 x i32> {{.*}})
; CHECK32: call <32 x i32> @_Z3minDv32_iS_(<32 x i32> {{.*}}, <32 x i32> {{.*}})

; CHECK64: call <64 x i32> @_Z3maxDv64_iS_(<64 x i32> {{.*}}, <64 x i32> {{.*}})
; CHECK64: call <64 x i32> @_Z3minDv64_iS_(<64 x i32> {{.*}}, <64 x i32> {{.*}})

; CHECK: Test program was successfully built.
