; CMPLRLLVM-9285 & CMPLRLLVM-9343 (Klocworks issue): SOAToAOS shouldn't
; crash when it doesn't find use of stored pointer as Call instruction
; (constructor). Commenting out "call void @_ZN3ArrIPiEC2Ei()" call in
; "_ZN1FC2Ev" to reproduce the crash. The nullptr access was reported by
; klocworks.

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt < %s -opaque-pointers -whole-program-assume -intel-libirc-allowed -disable-output                                                    \
; RUN:          -passes='require<dtrans-safetyanalyzer>,require<soatoaosop-approx>,require<soatoaosop-struct-methods>'    \
; RUN:          -dtrans-soatoaosop-array-type=struct.Arr                                                                  \
; RUN:          -dtrans-soatoaosop-array-type=struct.Arr.0                                                                \
; RUN:          -dtrans-soatoaosop-base-ptr-off=2                                                                         \
; RUN:          -dtrans-soatoaosop-method-call-site-comparison=ctor                                                       \
; RUN:          -dtrans-soatoaosop-array-ctor=_ZN3ArrIPiEC2Ei                                                             \
; RUN:          -dtrans-soatoaosop-array-ctor=_ZN3ArrIPfEC2Ei                                                             \
; RUN:          2>/dev/null

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

%class.F = type { ptr, ptr }
%struct.Arr = type <{ i32, [4 x i8], ptr, i32, [4 x i8] }>
%struct.Arr.0 = type <{ i32, [4 x i8], ptr, i32, [4 x i8] }>

declare i32 @__gxx_personality_v0(...)

declare noalias nonnull ptr @_Znwm(i64)

define hidden void @_ZN1FC2Ev(ptr nocapture "intel_dtrans_func_index"="1" %arg) personality ptr @__gxx_personality_v0 !intel.dtrans.func.type !10 {
bb:
  %tmp = tail call ptr @_Znwm(i64 24)
  %tmp1 = bitcast ptr %tmp to ptr
  %tmp2 = getelementptr inbounds %class.F, ptr %arg, i64 0, i32 0
  store ptr %tmp1, ptr %tmp2, align 8
;
;  Commenting out the below call to reproduce Segmentation fault.
;  tail call void @_ZN3ArrIPiEC2Ei(%struct.Arr* %tmp1, i32 1)
;
  %tmp3 = tail call ptr @_Znwm(i64 24)
  %tmp4 = bitcast ptr %tmp3 to ptr
  tail call void @_ZN3ArrIPfEC2Ei(ptr %tmp4, i32 1)
  %tmp5 = getelementptr inbounds %class.F, ptr %arg, i64 0, i32 1
  store ptr %tmp4, ptr %tmp5, align 8
  ret void
}

declare !intel.dtrans.func.type !12 hidden void @_ZN3ArrIPfEC2Ei(ptr "intel_dtrans_func_index"="1", i32)

declare !intel.dtrans.func.type !13 hidden void @_ZN3ArrIPiEC2Ei(ptr "intel_dtrans_func_index"="1", i32)

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
!12 = distinct !{!9}
!13 = distinct !{!8}
