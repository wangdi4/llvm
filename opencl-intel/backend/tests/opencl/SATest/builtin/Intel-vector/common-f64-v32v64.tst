; Check that v32 and v64 builtins are used.

; RUN: SATest -BUILD --config=%s.cfg -tsize=32 -cpuarch="skx" -llvm-option=-print-after=vplan-vec 2>&1 | FileCheck %s -check-prefixes=CHECK,CHECK32
; RUN: SATest -BUILD --config=%s.cfg -tsize=64 -cpuarch="skx" -llvm-option=-print-after=vplan-vec 2>&1 | FileCheck %s -check-prefixes=CHECK,CHECK64

; CHECK32: call <32 x double> @_Z5clampDv32_dS_S_(<32 x double> {{.*}}, <32 x double> {{.*}}, <32 x double> {{.*}})
; CHECK32: call <32 x double> @_Z7degreesDv32_d(<32 x double> {{.*}})
; CHECK32: call <32 x double> @_Z3maxDv32_dS_(<32 x double> {{.*}}, <32 x double> {{.*}})
; CHECK32: call <32 x double> @_Z3minDv32_dS_(<32 x double> {{.*}}, <32 x double> {{.*}})
; CHECK32: call <32 x double> @_Z3mixDv32_dS_S_(<32 x double> {{.*}}, <32 x double> {{.*}}, <32 x double> {{.*}})
; CHECK32: call <32 x double> @_Z7radiansDv32_d(<32 x double> {{.*}})
; CHECK32: call <32 x double> @_Z4stepDv32_dS_(<32 x double> {{.*}}, <32 x double> {{.*}})
; CHECK32: call <32 x double> @_Z10smoothstepDv32_dS_S_(<32 x double> {{.*}}, <32 x double> {{.*}}, <32 x double> {{.*}})
; CHECK32: call <32 x double> @_Z4signDv32_d(<32 x double> {{.*}})

; CHECK64: call <64 x double> @_Z5clampDv64_dS_S_(<64 x double> {{.*}}, <64 x double> {{.*}}, <64 x double> {{.*}})
; CHECK64: call <64 x double> @_Z7degreesDv64_d(<64 x double> {{.*}})
; CHECK64: call <64 x double> @_Z3maxDv64_dS_(<64 x double> {{.*}}, <64 x double> {{.*}})
; CHECK64: call <64 x double> @_Z3minDv64_dS_(<64 x double> {{.*}}, <64 x double> {{.*}})
; CHECK64: call <64 x double> @_Z3mixDv64_dS_S_(<64 x double> {{.*}}, <64 x double> {{.*}}, <64 x double> {{.*}})
; CHECK64: call <64 x double> @_Z7radiansDv64_d(<64 x double> {{.*}})
; CHECK64: call <64 x double> @_Z4stepDv64_dS_(<64 x double> {{.*}}, <64 x double> {{.*}})
; CHECK64: call <64 x double> @_Z10smoothstepDv64_dS_S_(<64 x double> {{.*}}, <64 x double> {{.*}}, <64 x double> {{.*}})
; CHECK64: call <64 x double> @_Z4signDv64_d(<64 x double> {{.*}})

; CHECK: Test program was successfully built.
