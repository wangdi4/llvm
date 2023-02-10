; Check fp16 implementation status.

; FIXME: SATest build will fail since we haven't implemented all relational builtins for fp16 yet.
; RUN: not SATest -BUILD --config=%s.cfg -tsize=32 -cpuarch="sapphirerapids" -llvm-option=-print-after=vplan-vec 2>&1 | FileCheck %s -check-prefix=CHECK32
; RUN: not SATest -BUILD --config=%s.cfg -tsize=64 -cpuarch="sapphirerapids" -llvm-option=-print-after=vplan-vec 2>&1 | FileCheck %s -check-prefix=CHECK64

; CHECK32x: call <32 x i32> @_Z16isfinite_v1widenDv32_Dh(<32 x half> {{.*}})
; CHECK32x: call <32 x i32> @_Z13isinf_v1widenDv32_Dh(<32 x half> {{.*}})
; CHECK32: [[ISNAN:%.*]] = fcmp uno <32 x half> {{.*}}, zeroinitializer
; CHECK32-NEXT: zext <32 x i1> [[ISNAN]] to <32 x i32>
; CHECK32x: call <32 x i32> @_Z16isnormal_v1widenDv32_Dh(<32 x half> {{.*}})
; CHECK32: [[ISORDERED:%.*]] = fcmp ord <32 x half> {{.*}}, {{.*}}
; CHECK32-NEXT: zext <32 x i1> [[ISORDERED]] to <32 x i32>
; CHECK32: [[ISUNORDERED:%.*]] = fcmp uno <32 x half> {{.*}}, {{.*}}
; CHECK32-NEXT: zext <32 x i1> [[ISUNORDERED]] to <32 x i32>
; CHECK32: [[BITCAST:%.*]] = bitcast <32 x half> {{.*}} to <32 x i16>
; CHECK32-NEXT: lshr <32 x i16> [[BITCAST]], <i16 15, i16 15,
; CHECK32: call <32 x half> @_Z9bitselectDv32_DhS_S_(<32 x half> {{.*}}, <32 x half> {{.*}}, <32 x half> {{.*}})
; CHECK32: call <32 x half> @_Z14select_v1widenDv32_DhS_Dv32_s(<32 x half> {{.*}}, <32 x half> {{.*}}, <32 x i16> {{.*}})
; CHECK32: call <32 x half> @_Z14select_v1widenDv32_DhS_Dv32_t(<32 x half> {{.*}}, <32 x half> {{.*}}, <32 x i16> {{.*}})

; CHECK64x: call <64 x i32> @_Z16isfinite_v1widenDv64_Dh(<64 x half> {{.*}})
; CHECK64x: call <64 x i32> @_Z13isinf_v1widenDv64_Dh(<64 x half> {{.*}})
; CHECK64: [[ISNAN:%.*]] = fcmp uno <64 x half> {{.*}}, zeroinitializer
; CHECK64-NEXT: zext <64 x i1> [[ISNAN]] to <64 x i32>
; CHECK64x: call <64 x i32> @_Z16isnormal_v1widenDv64_Dh(<64 x half> {{.*}})
; CHECK64: [[ISORDERED:%.*]] = fcmp ord <64 x half> {{.*}}, {{.*}}
; CHECK64-NEXT: zext <64 x i1> [[ISORDERED]] to <64 x i32>
; CHECK64: [[ISUNORDERED:%.*]] = fcmp uno <64 x half> {{.*}}, {{.*}}
; CHECK64-NEXT: zext <64 x i1> [[ISUNORDERED]] to <64 x i32>
; CHECK64: [[BITCAST:%.*]] = bitcast <64 x half> {{.*}} to <64 x i16>
; CHECK64-NEXT: lshr <64 x i16> [[BITCAST]], <i16 15, i16 15,
; CHECK64: call <64 x half> @_Z9bitselectDv64_DhS_S_(<64 x half> {{.*}}, <64 x half> {{.*}}, <64 x half> {{.*}})
; CHECK64: call <64 x half> @_Z14select_v1widenDv64_DhS_Dv64_s(<64 x half> {{.*}}, <64 x half> {{.*}}, <64 x i16> {{.*}})
; CHECK64: call <64 x half> @_Z14select_v1widenDv64_DhS_Dv64_t(<64 x half> {{.*}}, <64 x half> {{.*}}, <64 x i16> {{.*}})
