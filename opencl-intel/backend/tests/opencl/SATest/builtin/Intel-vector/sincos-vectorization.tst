; Check that sincos is vectorized to linear-strided vector variant.

; RUN: SATest -BUILD --config=%s.cfg -tsize=16 -cpuarch="skx" -llvm-option=-print-after=vplan-vec 2>&1 | FileCheck %s

; CHECK: call{{.*}} <16 x float> @_Z6sincosDv16_fPf(<16 x float> {{.*}}, {{.*}})
; CHECK: call{{.*}} <16 x double> @_Z6sincosDv16_dPd(<16 x double> {{.*}}, {{.*}})
