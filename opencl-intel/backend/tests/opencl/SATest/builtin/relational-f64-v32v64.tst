; Check that v32 and v64 builtins are used.

; RUN: SATest -BUILD --config=%s.cfg -tsize=32 -cpuarch="skx" -llvm-option=-print-after=vplan-vec,dpcpp-kernel-builtin-import 2>&1 | FileCheck %s -check-prefix=CHECK32
; RUN: SATest -BUILD --config=%s.cfg -tsize=64 -cpuarch="skx" -llvm-option=-print-after=vplan-vec,dpcpp-kernel-builtin-import 2>&1 | FileCheck %s -check-prefix=CHECK64

; IR Dump After vplan-vec
; CHECK32: call <32 x i32> @_Z16isfinite_v1widenDv32_d(<32 x double> {{.*}})
; CHECK32: call <32 x i32> @_Z13isinf_v1widenDv32_d(<32 x double> {{.*}})
; CHECK32: call <32 x i32> @_Z13isnan_v1widenDv32_d(<32 x double> {{.*}})
; CHECK32: call <32 x i32> @_Z16isnormal_v1widenDv32_d(<32 x double> {{.*}})
; CHECK32: call <32 x i32> @_Z17isordered_v1widenDv32_dS_(<32 x double> {{.*}}, <32 x double> {{.*}})
; CHECK32: call <32 x i32> @_Z19isunordered_v1widenDv32_dS_(<32 x double> {{.*}}, <32 x double> {{.*}})
; CHECK32: call <32 x i32> @_Z15signbit_v1widenDv32_d(<32 x double> {{.*}})
; CHECK32: call <32 x double> @_Z9bitselectDv32_dS_S_(<32 x double> {{.*}}, <32 x double> {{.*}}, <32 x double> {{.*}})
; CHECK32: call <32 x double> @_Z14select_v1widenDv32_dS_Dv32_l(<32 x double> {{.*}}, <32 x double> {{.*}}, <32 x i64> {{.*}})
; CHECK32: call <32 x double> @_Z14select_v1widenDv32_dS_Dv32_m(<32 x double> {{.*}}, <32 x double> {{.*}}, <32 x i64> {{.*}})

; IR Dump After dpcpp-kernel-builtin-import
; CHECK32: call <32 x i32> @_Z16isfinite_v1widenDv32_d(<32 x double> {{.*}})
; CHECK32: call <32 x i32> @_Z13isinf_v1widenDv32_d(<32 x double> {{.*}})
; CHECK32: call <32 x i32> @_Z13isnan_v1widenDv32_d(<32 x double> {{.*}})
; CHECK32: call <32 x i32> @_Z16isnormal_v1widenDv32_d(<32 x double> {{.*}})
; CHECK32: call <32 x i32> @_Z17isordered_v1widenDv32_dS_(<32 x double> {{.*}}, <32 x double> {{.*}})
; CHECK32: call <32 x i32> @_Z19isunordered_v1widenDv32_dS_(<32 x double> {{.*}}, <32 x double> {{.*}})
; CHECK32: call <32 x i32> @_Z15signbit_v1widenDv32_d(<32 x double> {{.*}})
; CHECK32: call <32 x double> @_Z9bitselectDv32_dS_S_(<32 x double> {{.*}}, <32 x double> {{.*}}, <32 x double> {{.*}})
; CHECK32: call <32 x double> @_Z14select_v1widenDv32_dS_Dv32_l(<32 x double> {{.*}}, <32 x double> {{.*}}, <32 x i64> {{.*}})
; CHECK32: call <32 x double> @_Z14select_v1widenDv32_dS_Dv32_m(<32 x double> {{.*}}, <32 x double> {{.*}}, <32 x i64> {{.*}})

; IR Dump After vplan-vec
; CHECK64: call <64 x i32> @_Z16isfinite_v1widenDv64_d(<64 x double> {{.*}})
; CHECK64: call <64 x i32> @_Z13isinf_v1widenDv64_d(<64 x double> {{.*}})
; CHECK64: call <64 x i32> @_Z13isnan_v1widenDv64_d(<64 x double> {{.*}})
; CHECK64: call <64 x i32> @_Z16isnormal_v1widenDv64_d(<64 x double> {{.*}})
; CHECK64: call <64 x i32> @_Z17isordered_v1widenDv64_dS_(<64 x double> {{.*}}, <64 x double> {{.*}})
; CHECK64: call <64 x i32> @_Z19isunordered_v1widenDv64_dS_(<64 x double> {{.*}}, <64 x double> {{.*}})
; CHECK64: call <64 x i32> @_Z15signbit_v1widenDv64_d(<64 x double> {{.*}})
; CHECK64: call <64 x double> @_Z9bitselectDv64_dS_S_(<64 x double> {{.*}}, <64 x double> {{.*}}, <64 x double> {{.*}})
; CHECK64: call <64 x double> @_Z14select_v1widenDv64_dS_Dv64_l(<64 x double> {{.*}}, <64 x double> {{.*}}, <64 x i64> {{.*}})
; CHECK64: call <64 x double> @_Z14select_v1widenDv64_dS_Dv64_m(<64 x double> {{.*}}, <64 x double> {{.*}}, <64 x i64> {{.*}})

; IR Dump After dpcpp-kernel-builtin-import
; CHECK64: call <64 x i32> @_Z16isfinite_v1widenDv64_d(<64 x double> {{.*}})
; CHECK64: call <64 x i32> @_Z13isinf_v1widenDv64_d(<64 x double> {{.*}})
; CHECK64: call <64 x i32> @_Z13isnan_v1widenDv64_d(<64 x double> {{.*}})
; CHECK64: call <64 x i32> @_Z16isnormal_v1widenDv64_d(<64 x double> {{.*}})
; CHECK64: call <64 x i32> @_Z17isordered_v1widenDv64_dS_(<64 x double> {{.*}}, <64 x double> {{.*}})
; CHECK64: call <64 x i32> @_Z19isunordered_v1widenDv64_dS_(<64 x double> {{.*}}, <64 x double> {{.*}})
; CHECK64: call <64 x i32> @_Z15signbit_v1widenDv64_d(<64 x double> {{.*}})
; CHECK64: call <64 x double> @_Z9bitselectDv64_dS_S_(<64 x double> {{.*}}, <64 x double> {{.*}}, <64 x double> {{.*}})
; CHECK64: call <64 x double> @_Z14select_v1widenDv64_dS_Dv64_l(<64 x double> {{.*}}, <64 x double> {{.*}}, <64 x i64> {{.*}})
; CHECK64: call <64 x double> @_Z14select_v1widenDv64_dS_Dv64_m(<64 x double> {{.*}}, <64 x double> {{.*}}, <64 x i64> {{.*}})
