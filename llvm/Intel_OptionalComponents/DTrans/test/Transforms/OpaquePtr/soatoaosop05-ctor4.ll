; Verify that call site analysis should detect that calls in "_ZN1FC2Ev"
; can't be merged since the calls (@Not_Ctor1 and @Not_Ctor2) are not
; constructor calls.

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt < %s -whole-program-assume -intel-libirc-allowed -disable-output                              \
; RUN:          -debug-only=dtrans-soatoaosop,dtrans-soatoaosop-struct                                                    \
; RUN:          -passes='require<dtrans-safetyanalyzer>,require<soatoaosop-approx>,require<soatoaosop-struct-methods>'    \
; RUN:          -dtrans-soatoaosop-array-type=struct.Arr                                                                  \
; RUN:          -dtrans-soatoaosop-array-type=struct.Arr.0                                                                \
; RUN:          -dtrans-soatoaosop-base-ptr-off=2                                                                         \
; RUN:          -dtrans-soatoaosop-method-call-site-comparison=ctor                                                       \
; RUN:          -dtrans-soatoaosop-array-ctor=_ZN3ArrIPiEC2Ei                                                             \
; RUN:          -dtrans-soatoaosop-array-ctor=_ZN3ArrIPfEC2Ei                                                             \
; RUN:       2>&1 | FileCheck %s
; REQUIRES: asserts
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

; CHECK: Array call sites analysis result: problem with call sites required to be merged

%class.F = type { ptr, ptr }
%struct.Arr = type <{ i32, [4 x i8], ptr, i32, [4 x i8] }>
%struct.Arr.0 = type <{ i32, [4 x i8], ptr, i32, [4 x i8] }>

declare i32 @__gxx_personality_v0(...)

declare noalias nonnull ptr @_Znwm(i64)

define hidden void @_ZN1FC2Ev(ptr nocapture "intel_dtrans_func_index"="1" %arg) personality ptr @__gxx_personality_v0 !intel.dtrans.func.type !10 {
bb:
  %tmp = tail call ptr @_Znwm(i64 24)
  %tmp1 = bitcast ptr %tmp to ptr
  tail call void @Not_Ctor1(ptr %tmp1, i32 1)
  %tmp2 = getelementptr inbounds %class.F, ptr %arg, i64 0, i32 0
  store ptr %tmp1, ptr %tmp2, align 8
  %tmp3 = tail call ptr @_Znwm(i64 24)
  %tmp4 = bitcast ptr %tmp3 to ptr
  tail call void @Not_Ctor2(ptr %tmp4, i32 1)
  %tmp5 = getelementptr inbounds %class.F, ptr %arg, i64 0, i32 1
  store ptr %tmp4, ptr %tmp5, align 8
  ret void
}

declare !intel.dtrans.func.type !12 hidden void @Not_Ctor1(ptr "intel_dtrans_func_index"="1", i32)

declare !intel.dtrans.func.type !13 hidden void @Not_Ctor2(ptr "intel_dtrans_func_index"="1", i32)

!intel.dtrans.types = !{!0, !5, !7}

!0 = !{!"S", %struct.Arr zeroinitializer, i32 5, !1, !2, !4, !1, !2}
!1 = !{i32 0, i32 0}
!2 = !{!"A", i32 4, !3}
!3 = !{i8 0, i32 0}
!4 = !{i32 0, i32 2}
!5 = !{!"S", %struct.Arr.0 zeroinitializer, i32 5, !1, !2, !6, !1, !2}
!6 = !{float 0.000000e+00, i32 2}
!7 = !{!"S", %class.F zeroinitializer, i32 2, !8, !9}
!8 = !{%struct.Arr zeroinitializer, i32 1}
!9 = !{%struct.Arr.0 zeroinitializer, i32 1}
!10 = distinct !{!11}
!11 = !{%class.F zeroinitializer, i32 1}
!12 = distinct !{!8}
!13 = distinct !{!9}
