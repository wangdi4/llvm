; RUN: opt < %s -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -disable-output                                      \
; RUN:          -debug-only=dtrans-soatoaosop,dtrans-soatoaosop-struct                                                    \
; RUN:          -passes='require<dtrans-safetyanalyzer>,require<soatoaosop-approx>,require<soatoaosop-struct-methods>'    \
; RUN:          -dtrans-soatoaosop-array-type=struct.Arr                                                                  \
; RUN:          -dtrans-soatoaosop-array-type=struct.Arr.0                                                                \
; RUN:          -dtrans-soatoaosop-base-ptr-off=2                                                                         \
; RUN:          -dtrans-soatoaosop-method-call-site-comparison=ctor                                                       \
; RUN:          -dtrans-soatoaosop-array-ctor=_ZN3ArrIPiEC2Ei                                                             \
; RUN:          -dtrans-soatoaosop-array-ctor=_ZN3ArrIPfEC2Ei                                                             \
; RUN:       2>&1 | FileCheck --check-prefix=CHECK-TRANS %s
;
; RUN: opt < %s -opaque-pointers -whole-program-assume -intel-libirc-allowed -disable-output                                                    \
; RUN:          -debug-only=dtrans-soatoaosop,dtrans-soatoaosop-struct                                                    \
; RUN:          -passes='require<dtrans-safetyanalyzer>,require<soatoaosop-approx>,require<soatoaosop-struct-methods>'    \
; RUN:          -dtrans-soatoaosop-array-type=struct.Arr                                                                  \
; RUN:          -dtrans-soatoaosop-array-type=struct.Arr.0                                                                \
; RUN:          -dtrans-soatoaosop-base-ptr-off=2                                                                         \
; RUN:          -dtrans-soatoaosop-method-call-site-comparison=ctor                                                       \
; RUN:          -dtrans-soatoaosop-array-ctor=_ZN3ArrIPiEC2Ei                                                             \
; RUN:          -dtrans-soatoaosop-array-ctor=_ZN3ArrIPfEC2Ei                                                             \
; RUN:       2>&1 | FileCheck --check-prefix=CHECK-OP-TRANS %s
; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

%class.F = type { %struct.Arr*, %struct.Arr.0* }
%struct.Arr = type <{ i32, [4 x i8], i32**, i32, [4 x i8] }>
%struct.Arr.0 = type <{ i32, [4 x i8], float**, i32, [4 x i8] }>

declare i32 @__gxx_personality_v0(...)

declare noalias nonnull i8* @_Znwm(i64)

; CHECK-TRANS: ; Checking structure's method _ZN1FC2Ev
; CHECK-TRANS: ; IR:  has only expected side-effects
; CHECK-TRANS: ; Dump instructions needing update. Total = 4
; CHECK-OP-TRANS: ; Checking structure's method _ZN1FC2Ev
; CHECK-OP-TRANS: ; IR:  has only expected side-effects
; CHECK-OP-TRANS: ; Dump instructions needing update. Total = 4

define hidden void @_ZN1FC2Ev(%class.F* "intel_dtrans_func_index"="1" nocapture %arg) personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) !intel.dtrans.func.type !12 {
bb:
  %tmp = tail call i8* @_Znwm(i64 24) #3
  %tmp1 = bitcast i8* %tmp to %struct.Arr*
; CHECK-TRANS: ; ArrayInst: Array method call
; CHECK-OP-TRANS: ; ArrayInst: Array method call
  tail call void @_ZN3ArrIPiEC2Ei(%struct.Arr* %tmp1, i32 1)
  %tmp2 = getelementptr inbounds %class.F, %class.F* %arg, i64 0, i32 0
; CHECK-TRANS: ; ArrayInst: Init ptr to array
; CHECK-OP-TRANS: ; ArrayInst: Init ptr to array
  store %struct.Arr* %tmp1, %struct.Arr** %tmp2, align 8
  %tmp3 = tail call i8* @_Znwm(i64 24) #3
  %tmp4 = bitcast i8* %tmp3 to %struct.Arr.0*
; CHECK-TRANS: ; ArrayInst: Array method call
; CHECK-OP-TRANS: ; ArrayInst: Array method call
  tail call void @_ZN3ArrIPfEC2Ei(%struct.Arr.0* %tmp4, i32 1)
  %tmp5 = getelementptr inbounds %class.F, %class.F* %arg, i64 0, i32 1
; CHECK-TRANS: ; ArrayInst: Init ptr to array
; CHECK-OP-TRANS: ; ArrayInst: Init ptr to array
  store %struct.Arr.0* %tmp4, %struct.Arr.0** %tmp5, align 8
  ret void
}

declare !intel.dtrans.func.type !13 hidden void @_ZN3ArrIPiEC2Ei(%struct.Arr* "intel_dtrans_func_index"="1", i32)
declare !intel.dtrans.func.type !14 hidden void @_ZN3ArrIPfEC2Ei(%struct.Arr.0* "intel_dtrans_func_index"="1", i32)

; CHECK-TRANS: ; Seen ctor.
; CHECK-TRANS: ; Array call sites analysis result: required call sites can be merged
; CHECK-OP-TRANS: ; Seen ctor.
; CHECK-OP-TRANS: ; Array call sites analysis result: required call sites can be merged

!intel.dtrans.types = !{!1, !6, !10}

!1 = !{!"S", %struct.Arr zeroinitializer, i32 5, !5, !2, !4, !5, !2}
!2 = !{!"A", i32 4, !3}
!3 = !{i8 0, i32 0}
!4 = !{i32 0, i32 2}
!5 = !{i32 0, i32 0}
!6 = !{!"S", %struct.Arr.0 zeroinitializer, i32 5, !5, !2, !7, !5, !2}
!7 = !{float 0.000000e+00, i32 2}
!8 = !{%struct.Arr zeroinitializer, i32 1}
!9 = !{%struct.Arr.0 zeroinitializer, i32 1}
!10 = !{!"S", %class.F zeroinitializer, i32 2, !8, !9}
!11 = !{%class.F zeroinitializer, i32 1}
!12 = distinct !{!11}
!13 = distinct !{!8}
!14 = distinct !{!9}

