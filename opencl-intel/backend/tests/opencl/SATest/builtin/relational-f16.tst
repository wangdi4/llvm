; Check fp16 implementation status.

; FIXME: SATest build will fail since we haven't implemented all relational builtins for fp16 yet.
; RUN: not SATest -BUILD --config=%s.cfg -tsize=32 -cpuarch="sapphirerapids" -llvm-option=-print-after=vplan-vec 2>&1 | FileCheck %s -check-prefix=CHECK32
; RUN: not SATest -BUILD --config=%s.cfg -tsize=64 -cpuarch="sapphirerapids" -llvm-option=-print-after=vplan-vec 2>&1 | FileCheck %s -check-prefix=CHECK64

; CHECK32x: call <32 x i32> @_Z16isfinite_v1widenDv32_Dh(<32 x half> {{.*}})
; CHECK32x: call <32 x i32> @_Z13isinf_v1widenDv32_Dh(<32 x half> {{.*}})
; CHECK32x: call <32 x i32> @_Z13isnan_v1widenDv32_Dh(<32 x half> {{.*}})
; CHECK32x: call <32 x i32> @_Z16isnormal_v1widenDv32_Dh(<32 x half> {{.*}})
; CHECK32x: call <32 x i32> @_Z17isordered_v1widenDv32_DhS_(<32 x half> {{.*}}, <32 x half> {{.*}})
; CHECK32x: call <32 x i32> @_Z19isunordered_v1widenDv32_DhS_(<32 x half> {{.*}}, <32 x half> {{.*}})
; CHECK32x: call <32 x i32> @_Z15signbit_v1widenDv32_Dh(<32 x half> {{.*}})
; CHECK32: call <32 x half> @_Z9bitselectDv32_DhS_S_(<32 x half> {{.*}}, <32 x half> {{.*}}, <32 x half> {{.*}})
; CHECK32: call <32 x half> @_Z14select_v1widenDv32_DhS_Dv32_s(<32 x half> {{.*}}, <32 x half> {{.*}}, <32 x i16> {{.*}})
; CHECK32: call <32 x half> @_Z14select_v1widenDv32_DhS_Dv32_t(<32 x half> {{.*}}, <32 x half> {{.*}}, <32 x i16> {{.*}})

; CHECK64x: call <64 x i32> @_Z16isfinite_v1widenDv64_Dh(<64 x half> {{.*}})
; CHECK64x: call <64 x i32> @_Z13isinf_v1widenDv64_Dh(<64 x half> {{.*}})
; CHECK64x: call <64 x i32> @_Z13isnan_v1widenDv64_Dh(<64 x half> {{.*}})
; CHECK64x: call <64 x i32> @_Z16isnormal_v1widenDv64_Dh(<64 x half> {{.*}})
; CHECK64x: call <64 x i32> @_Z17isordered_v1widenDv64_DhS_(<64 x half> {{.*}}, <64 x half> {{.*}})
; CHECK64x: call <64 x i32> @_Z19isunordered_v1widenDv64_DhS_(<64 x half> {{.*}}, <64 x half> {{.*}})
; CHECK64x: call <64 x i32> @_Z15signbit_v1widenDv64_Dh(<64 x half> {{.*}})
; CHECK64: call <64 x half> @_Z9bitselectDv64_DhS_S_(<64 x half> {{.*}}, <64 x half> {{.*}}, <64 x half> {{.*}})
; CHECK64: call <64 x half> @_Z14select_v1widenDv64_DhS_Dv64_s(<64 x half> {{.*}}, <64 x half> {{.*}}, <64 x i16> {{.*}})
; CHECK64: call <64 x half> @_Z14select_v1widenDv64_DhS_Dv64_t(<64 x half> {{.*}}, <64 x half> {{.*}}, <64 x i16> {{.*}})
