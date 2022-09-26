; Check that v32 and v64 builtins are used.

; RUN: SATest -BUILD --config=%s.cfg -tsize=32 -cpuarch="skx" -llvm-option=-print-after=vplan-vec,dpcpp-kernel-builtin-import 2>&1 | FileCheck %s -check-prefix=CHECK32
; RUN: SATest -BUILD --config=%s.cfg -tsize=64 -cpuarch="skx" -llvm-option=-print-after=vplan-vec,dpcpp-kernel-builtin-import 2>&1 | FileCheck %s -check-prefix=CHECK64

; IR Dump After vplan-vec
; CHECK32: call <32 x i32> @_Z16isfinite_v1widenDv32_d(<32 x double> {{.*}})
; CHECK32: call <32 x i32> @_Z13isinf_v1widenDv32_d(<32 x double> {{.*}})
; CHECK32: [[ISNAN:%.*]] = fcmp uno <32 x double> {{.*}}, zeroinitializer
; CHECK32-NEXT: zext <32 x i1> [[ISNAN]] to <32 x i32>
; CHECK32: call <32 x i32> @_Z16isnormal_v1widenDv32_d(<32 x double> {{.*}})
; CHECK32: [[ISORDERED:%.*]] = fcmp ord <32 x double> {{.*}}, {{.*}}
; CHECK32-NEXT: zext <32 x i1> [[ISORDERED]] to <32 x i32>
; CHECK32: [[ISUNORDERED:%.*]] = fcmp uno <32 x double> {{.*}}, {{.*}}
; CHECK32-NEXT: zext <32 x i1> [[ISUNORDERED]] to <32 x i32>
; CHECK32: [[BITCAST:%.*]] = bitcast <32 x double> {{.*}} to <32 x i64>
; CHECK32-NEXT: lshr <32 x i64> [[BITCAST]], <i64 63, i64 63,
; CHECK32: call <32 x double> @_Z9bitselectDv32_dS_S_(<32 x double> {{.*}}, <32 x double> {{.*}}, <32 x double> {{.*}})
; CHECK32: call <32 x double> @_Z14select_v1widenDv32_dS_Dv32_l(<32 x double> {{.*}}, <32 x double> {{.*}}, <32 x i64> {{.*}})
; CHECK32: call <32 x double> @_Z14select_v1widenDv32_dS_Dv32_m(<32 x double> {{.*}}, <32 x double> {{.*}}, <32 x i64> {{.*}})

; IR Dump After dpcpp-kernel-builtin-import
; CHECK32: call <32 x i32> @_Z16isfinite_v1widenDv32_d(<32 x double> {{.*}})
; CHECK32: call <32 x i32> @_Z13isinf_v1widenDv32_d(<32 x double> {{.*}})
; CHECK32: call <32 x i32> @_Z16isnormal_v1widenDv32_d(<32 x double> {{.*}})
; CHECK32: call <32 x double> @_Z9bitselectDv32_dS_S_(<32 x double> {{.*}}, <32 x double> {{.*}}, <32 x double> {{.*}})
; CHECK32: call <32 x double> @_Z14select_v1widenDv32_dS_Dv32_l(<32 x double> {{.*}}, <32 x double> {{.*}}, <32 x i64> {{.*}})
; CHECK32: call <32 x double> @_Z14select_v1widenDv32_dS_Dv32_m(<32 x double> {{.*}}, <32 x double> {{.*}}, <32 x i64> {{.*}})

; IR Dump After vplan-vec
; CHECK64: call <64 x i32> @_Z16isfinite_v1widenDv64_d(<64 x double> {{.*}})
; CHECK64: call <64 x i32> @_Z13isinf_v1widenDv64_d(<64 x double> {{.*}})
; CHECK64: [[ISNAN:%.*]] = fcmp uno <64 x double> {{.*}}, zeroinitializer
; CHECK64-NEXT: zext <64 x i1> [[ISNAN]] to <64 x i32>
; CHECK64: call <64 x i32> @_Z16isnormal_v1widenDv64_d(<64 x double> {{.*}})
; CHECK64: [[ISORDERED:%.*]] = fcmp ord <64 x double> {{.*}}, {{.*}}
; CHECK64-NEXT: zext <64 x i1> [[ISORDERED]] to <64 x i32>
; CHECK64: [[ISUNORDERED:%.*]] = fcmp uno <64 x double> {{.*}}, {{.*}}
; CHECK64-NEXT: zext <64 x i1> [[ISUNORDERED]] to <64 x i32>
; CHECK64: [[BITCAST:%.*]] = bitcast <64 x double> {{.*}} to <64 x i64>
; CHECK64-NEXT: lshr <64 x i64> [[BITCAST]], <i64 63, i64 63,
; CHECK64: call <64 x double> @_Z9bitselectDv64_dS_S_(<64 x double> {{.*}}, <64 x double> {{.*}}, <64 x double> {{.*}})
; CHECK64: call <64 x double> @_Z14select_v1widenDv64_dS_Dv64_l(<64 x double> {{.*}}, <64 x double> {{.*}}, <64 x i64> {{.*}})
; CHECK64: call <64 x double> @_Z14select_v1widenDv64_dS_Dv64_m(<64 x double> {{.*}}, <64 x double> {{.*}}, <64 x i64> {{.*}})

; IR Dump After dpcpp-kernel-builtin-import
; CHECK64: call <64 x i32> @_Z16isfinite_v1widenDv64_d(<64 x double> {{.*}})
; CHECK64: call <64 x i32> @_Z13isinf_v1widenDv64_d(<64 x double> {{.*}})
; CHECK64: call <64 x i32> @_Z16isnormal_v1widenDv64_d(<64 x double> {{.*}})
; CHECK64: call <64 x double> @_Z9bitselectDv64_dS_S_(<64 x double> {{.*}}, <64 x double> {{.*}}, <64 x double> {{.*}})
; CHECK64: call <64 x double> @_Z14select_v1widenDv64_dS_Dv64_l(<64 x double> {{.*}}, <64 x double> {{.*}}, <64 x i64> {{.*}})
; CHECK64: call <64 x double> @_Z14select_v1widenDv64_dS_Dv64_m(<64 x double> {{.*}}, <64 x double> {{.*}}, <64 x i64> {{.*}})
