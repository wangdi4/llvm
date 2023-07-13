; RUN: opt < %s -whole-program-assume -intel-libirc-allowed -disable-output                              \
; RUN:          -debug-only=dtrans-soatoaosop,dtrans-soatoaosop-struct                                                    \
; RUN:          -passes='require<dtrans-safetyanalyzer>,require<soatoaosop-approx>,require<soatoaosop-struct-methods>'    \
; RUN:          -dtrans-soatoaosop-array-type=struct.Arr                                                                  \
; RUN:          -dtrans-soatoaosop-array-type=struct.Arr.0                                                                \
; RUN:          -dtrans-soatoaosop-base-ptr-off=2                                                                         \
; RUN:          -dtrans-soatoaosop-method-call-site-comparison=dtor                                                       \
; RUN:          -dtrans-soatoaosop-array-dtor=_ZN3ArrIPiED2Ev                                                             \
; RUN:          -dtrans-soatoaosop-array-dtor=_ZN3ArrIPfED2Ev                                                             \
; RUN:       2>&1 | FileCheck %s
; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

%class.F = type { ptr, ptr }
%struct.Arr = type <{ i32, [4 x i8], ptr, i32, [4 x i8] }>
%struct.Arr.0 = type <{ i32, [4 x i8], ptr, i32, [4 x i8] }>

; CHECK: ; Checking structure's method _ZN1FD2Ev
; CHECK: ; IR:  has only expected side-effects
; CHECK: ; Dump instructions needing update. Total = 4

declare dso_local void @_ZdlPv(ptr) local_unnamed_addr

define hidden void @_ZN1FD2Ev(ptr nocapture readonly "intel_dtrans_func_index"="1" %arg) !intel.dtrans.func.type !10 {
bb:
  %tmp = getelementptr inbounds %class.F, ptr %arg, i64 0, i32 0
; CHECK: ; ArrayInst: Load of array
  %tmp1 = load ptr, ptr %tmp, align 8
  %tmp2 = icmp eq ptr %tmp1, null
  br i1 %tmp2, label %bb5, label %bb3

bb3:                                              ; preds = %bb
; CHECK: ; ArrayInst: Array method call
  tail call void @_ZN3ArrIPiED2Ev(ptr nonnull %tmp1)
  %tmp4 = bitcast ptr %tmp1 to ptr
  tail call void @_ZdlPv(ptr %tmp4)
  br label %bb5

bb5:                                              ; preds = %bb3, %bb
  %tmp6 = getelementptr inbounds %class.F, ptr %arg, i64 0, i32 1
; CHECK: ; ArrayInst: Load of array
  %tmp7 = load ptr, ptr %tmp6, align 8
  %tmp8 = icmp eq ptr %tmp7, null
  br i1 %tmp8, label %bb11, label %bb9

bb9:                                              ; preds = %bb5
; CHECK: ; ArrayInst: Array method call
  tail call void @_ZN3ArrIPfED2Ev(ptr nonnull %tmp7)
  %tmp10 = bitcast ptr %tmp7 to ptr
  tail call void @_ZdlPv(ptr %tmp10)
  ret void

bb11:                                             ; preds = %bb5
  ret void
}

declare !intel.dtrans.func.type !12 hidden void @_ZN3ArrIPiED2Ev(ptr nocapture readonly "intel_dtrans_func_index"="1")

declare !intel.dtrans.func.type !13 hidden void @_ZN3ArrIPfED2Ev(ptr nocapture readonly "intel_dtrans_func_index"="1")

; CHECK: ; Seen dtor.
; CHECK: ; Array call sites analysis result: required call sites can be merged

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
