; Check that v32 and v64 builtins are used.

; RUN: SATest -BUILD --config=%s.cfg -tsize=32 -cpuarch="skx" -llvm-option=-print-after=vplan-vec 2>&1 | FileCheck %s -check-prefixes=CHECK,CHECK32
; RUN: SATest -BUILD --config=%s.cfg -tsize=64 -cpuarch="skx" -llvm-option=-print-after=vplan-vec 2>&1 | FileCheck %s -check-prefixes=CHECK,CHECK64

; CHECK32: call <32 x float> @_Z5clampDv32_fS_S_(<32 x float> {{.*}}, <32 x float> {{.*}}, <32 x float> {{.*}})
; CHECK32: call <32 x float> @_Z7degreesDv32_f(<32 x float> {{.*}})
; CHECK32: call <32 x float> @_Z3maxDv32_fS_(<32 x float> {{.*}}, <32 x float> {{.*}})
; CHECK32: call <32 x float> @_Z3minDv32_fS_(<32 x float> {{.*}}, <32 x float> {{.*}})
; CHECK32: call <32 x float> @_Z3mixDv32_fS_S_(<32 x float> {{.*}}, <32 x float> {{.*}}, <32 x float> {{.*}})
; CHECK32: call <32 x float> @_Z7radiansDv32_f(<32 x float> {{.*}})
; CHECK32: call <32 x float> @_Z4stepDv32_fS_(<32 x float> {{.*}}, <32 x float> {{.*}})
; CHECK32: call <32 x float> @_Z10smoothstepDv32_fS_S_(<32 x float> {{.*}}, <32 x float> {{.*}}, <32 x float> {{.*}})
; CHECK32: call <32 x float> @_Z4signDv32_f(<32 x float> {{.*}})
; CHECK32: call <32 x float> @_Z5clampDv32_fS_S_S_(<32 x float> {{.*}}, <32 x float> {{.*}}, <32 x float> {{.*}}, <32 x float> {{.*}})
; CHECK32: call <32 x float> @_Z7degreesDv32_fS_(<32 x float> {{.*}}, <32 x float> {{.*}})
; CHECK32: call <32 x float> @_Z3maxDv32_fS_S_(<32 x float> {{.*}}, <32 x float> {{.*}}, <32 x float> {{.*}})
; CHECK32: call <32 x float> @_Z3minDv32_fS_S_(<32 x float> {{.*}}, <32 x float> {{.*}}, <32 x float> {{.*}})
; CHECK32: call <32 x float> @_Z3mixDv32_fS_S_S_(<32 x float> {{.*}}, <32 x float> {{.*}}, <32 x float> {{.*}}, <32 x float> {{.*}})
; CHECK32: call <32 x float> @_Z7radiansDv32_fS_(<32 x float> {{.*}}, <32 x float> {{.*}})
; CHECK32: call <32 x float> @_Z4stepDv32_fS_S_(<32 x float> {{.*}}, <32 x float> {{.*}}, <32 x float> {{.*}})
; CHECK32: call <32 x float> @_Z10smoothstepDv32_fS_S_S_(<32 x float> {{.*}}, <32 x float> {{.*}}, <32 x float> {{.*}}, <32 x float> {{.*}})
; CHECK32: call <32 x float> @_Z4signDv32_fS_(<32 x float> {{.*}}, <32 x float> {{.*}})

; CHECK64: call <64 x float> @_Z5clampDv64_fS_S_(<64 x float> {{.*}}, <64 x float> {{.*}}, <64 x float> {{.*}})
; CHECK64: call <64 x float> @_Z7degreesDv64_f(<64 x float> {{.*}})
; CHECK64: call <64 x float> @_Z3maxDv64_fS_(<64 x float> {{.*}}, <64 x float> {{.*}})
; CHECK64: call <64 x float> @_Z3minDv64_fS_(<64 x float> {{.*}}, <64 x float> {{.*}})
; CHECK64: call <64 x float> @_Z3mixDv64_fS_S_(<64 x float> {{.*}}, <64 x float> {{.*}}, <64 x float> {{.*}})
; CHECK64: call <64 x float> @_Z7radiansDv64_f(<64 x float> {{.*}})
; CHECK64: call <64 x float> @_Z4stepDv64_fS_(<64 x float> {{.*}}, <64 x float> {{.*}})
; CHECK64: call <64 x float> @_Z10smoothstepDv64_fS_S_(<64 x float> {{.*}}, <64 x float> {{.*}}, <64 x float> {{.*}})
; CHECK64: call <64 x float> @_Z4signDv64_f(<64 x float> {{.*}})
; CHECK64: call <64 x float> @_Z5clampDv64_fS_S_S_(<64 x float> {{.*}}, <64 x float> {{.*}}, <64 x float> {{.*}}, <64 x float> {{.*}})
; CHECK64: call <64 x float> @_Z7degreesDv64_fS_(<64 x float> {{.*}}, <64 x float> {{.*}})
; CHECK64: call <64 x float> @_Z3maxDv64_fS_S_(<64 x float> {{.*}}, <64 x float> {{.*}}, <64 x float> {{.*}})
; CHECK64: call <64 x float> @_Z3minDv64_fS_S_(<64 x float> {{.*}}, <64 x float> {{.*}}, <64 x float> {{.*}})
; CHECK64: call <64 x float> @_Z3mixDv64_fS_S_S_(<64 x float> {{.*}}, <64 x float> {{.*}}, <64 x float> {{.*}}, <64 x float> {{.*}})
; CHECK64: call <64 x float> @_Z7radiansDv64_fS_(<64 x float> {{.*}}, <64 x float> {{.*}})
; CHECK64: call <64 x float> @_Z4stepDv64_fS_S_(<64 x float> {{.*}}, <64 x float> {{.*}}, <64 x float> {{.*}})
; CHECK64: call <64 x float> @_Z10smoothstepDv64_fS_S_S_(<64 x float> {{.*}}, <64 x float> {{.*}}, <64 x float> {{.*}}, <64 x float> {{.*}})
; CHECK64: call <64 x float> @_Z4signDv64_fS_(<64 x float> {{.*}}, <64 x float> {{.*}})

; CHECK: Test program was successfully built.
