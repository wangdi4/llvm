; Check that v32 and v64 builtins are used.

; RUN: SATest -BUILD --config=%s.cfg -tsize=32 -cpuarch="skx" -llvm-option=-print-after=vplan-vec 2>&1 | FileCheck %s -check-prefix=CHECK32
; RUN: SATest -BUILD --config=%s.cfg -tsize=64 -cpuarch="skx" -llvm-option=-print-after=vplan-vec 2>&1 | FileCheck %s -check-prefix=CHECK64

; CHECK32: call <32 x i32> @_Z16isfinite_v1widenDv32_f(<32 x float> {{.*}})
; CHECK32: call <32 x i32> @_Z13isinf_v1widenDv32_f(<32 x float> {{.*}})
; CHECK32: call <32 x i32> @_Z13isnan_v1widenDv32_f(<32 x float> {{.*}})
; CHECK32: call <32 x i32> @_Z16isnormal_v1widenDv32_f(<32 x float> {{.*}})
; CHECK32: call <32 x i32> @_Z17isordered_v1widenDv32_fS_(<32 x float> {{.*}}, <32 x float> {{.*}})
; CHECK32: call <32 x i32> @_Z19isunordered_v1widenDv32_fS_(<32 x float> {{.*}}, <32 x float> {{.*}})
; CHECK32: call <32 x i32> @_Z15signbit_v1widenDv32_f(<32 x float> {{.*}})
; CHECK32: call <32 x float> @_Z9bitselectDv32_fS_S_(<32 x float> {{.*}}, <32 x float> {{.*}}, <32 x float> {{.*}})
; CHECK32: call <32 x float> @_Z14select_v1widenDv32_fS_Dv32_i(<32 x float> {{.*}}, <32 x float> {{.*}}, <32 x i32> {{.*}})
; CHECK32: call <32 x float> @_Z14select_v1widenDv32_fS_Dv32_j(<32 x float> {{.*}}, <32 x float> {{.*}}, <32 x i32> {{.*}})

; CHECK64: call <64 x i32> @_Z16isfinite_v1widenDv64_f(<64 x float> {{.*}})
; CHECK64: call <64 x i32> @_Z13isinf_v1widenDv64_f(<64 x float> {{.*}})
; CHECK64: call <64 x i32> @_Z13isnan_v1widenDv64_f(<64 x float> {{.*}})
; CHECK64: call <64 x i32> @_Z16isnormal_v1widenDv64_f(<64 x float> {{.*}})
; CHECK64: call <64 x i32> @_Z17isordered_v1widenDv64_fS_(<64 x float> {{.*}}, <64 x float> {{.*}})
; CHECK64: call <64 x i32> @_Z19isunordered_v1widenDv64_fS_(<64 x float> {{.*}}, <64 x float> {{.*}})
; CHECK64: call <64 x i32> @_Z15signbit_v1widenDv64_f(<64 x float> {{.*}})
; CHECK64: call <64 x float> @_Z9bitselectDv64_fS_S_(<64 x float> {{.*}}, <64 x float> {{.*}}, <64 x float> {{.*}})
; CHECK64: call <64 x float> @_Z14select_v1widenDv64_fS_Dv64_i(<64 x float> {{.*}}, <64 x float> {{.*}}, <64 x i32> {{.*}})
; CHECK64: call <64 x float> @_Z14select_v1widenDv64_fS_Dv64_j(<64 x float> {{.*}}, <64 x float> {{.*}}, <64 x i32> {{.*}})
