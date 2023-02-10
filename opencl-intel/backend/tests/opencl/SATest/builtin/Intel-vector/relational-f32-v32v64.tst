; Check that v32 and v64 builtins are used.

; RUN: SATest -BUILD --config=%s.cfg -tsize=32 -cpuarch="skx" -llvm-option=-print-after=vplan-vec 2>&1 | FileCheck %s -check-prefix=CHECK32
; RUN: SATest -BUILD --config=%s.cfg -tsize=64 -cpuarch="skx" -llvm-option=-print-after=vplan-vec 2>&1 | FileCheck %s -check-prefix=CHECK64

; CHECK32: call <32 x i32> @_Z16isfinite_v1widenDv32_f(<32 x float> {{.*}})
; CHECK32: call <32 x i32> @_Z13isinf_v1widenDv32_f(<32 x float> {{.*}})
; CHECK32: [[ISNAN:%.*]] = fcmp uno <32 x float> {{.*}}, zeroinitializer
; CHECK32-NEXT: zext <32 x i1> [[ISNAN]] to <32 x i32>
; CHECK32: call <32 x i32> @_Z16isnormal_v1widenDv32_f(<32 x float> {{.*}})
; CHECK32: [[ISORDERED:%.*]] = fcmp ord <32 x float> {{.*}}, {{.*}}
; CHECK32-NEXT: zext <32 x i1> [[ISORDERED]] to <32 x i32>
; CHECK32: [[ISUNORDERED:%.*]] = fcmp uno <32 x float> {{.*}}, {{.*}}
; CHECK32-NEXT: zext <32 x i1> [[ISUNORDERED]] to <32 x i32>
; CHECK32: [[BITCAST:%.*]] = bitcast <32 x float> {{.*}} to <32 x i32>
; CHECK32-NEXT: lshr <32 x i32> [[BITCAST]], <i32 31, i32 31,
; CHECK32: call <32 x float> @_Z9bitselectDv32_fS_S_(<32 x float> {{.*}}, <32 x float> {{.*}}, <32 x float> {{.*}})
; CHECK32: call <32 x float> @_Z14select_v1widenDv32_fS_Dv32_i(<32 x float> {{.*}}, <32 x float> {{.*}}, <32 x i32> {{.*}})
; CHECK32: call <32 x float> @_Z14select_v1widenDv32_fS_Dv32_j(<32 x float> {{.*}}, <32 x float> {{.*}}, <32 x i32> {{.*}})

; CHECK64: call <64 x i32> @_Z16isfinite_v1widenDv64_f(<64 x float> {{.*}})
; CHECK64: call <64 x i32> @_Z13isinf_v1widenDv64_f(<64 x float> {{.*}})
; CHECK64: [[ISNAN:%.*]] = fcmp uno <64 x float> {{.*}}, zeroinitializer
; CHECK64-NEXT: zext <64 x i1> [[ISNAN]] to <64 x i32>
; CHECK64: call <64 x i32> @_Z16isnormal_v1widenDv64_f(<64 x float> {{.*}})
; CHECK64: [[ISORDERED:%.*]] = fcmp ord <64 x float> {{.*}}, {{.*}}
; CHECK64-NEXT: zext <64 x i1> [[ISORDERED]] to <64 x i32>
; CHECK64: [[ISUNORDERED:%.*]] = fcmp uno <64 x float> {{.*}}, {{.*}}
; CHECK64-NEXT: zext <64 x i1> [[ISUNORDERED]] to <64 x i32>
; CHECK64: [[BITCAST:%.*]] = bitcast <64 x float> {{.*}} to <64 x i32>
; CHECK64-NEXT: lshr <64 x i32> [[BITCAST]], <i32 31, i32 31,
; CHECK64: call <64 x float> @_Z9bitselectDv64_fS_S_(<64 x float> {{.*}}, <64 x float> {{.*}}, <64 x float> {{.*}})
; CHECK64: call <64 x float> @_Z14select_v1widenDv64_fS_Dv64_i(<64 x float> {{.*}}, <64 x float> {{.*}}, <64 x i32> {{.*}})
; CHECK64: call <64 x float> @_Z14select_v1widenDv64_fS_Dv64_j(<64 x float> {{.*}}, <64 x float> {{.*}}, <64 x i32> {{.*}})
