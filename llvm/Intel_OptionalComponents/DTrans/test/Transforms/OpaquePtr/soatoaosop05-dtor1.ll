; RUN: opt < %s -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -disable-output                                      \
; RUN:          -debug-only=dtrans-soatoaosop,dtrans-soatoaosop-struct                                                    \
; RUN:          -passes='require<dtrans-safetyanalyzer>,require<soatoaosop-approx>,require<soatoaosop-struct-methods>'    \
; RUN:          -dtrans-soatoaosop-array-type=struct.Arr                                                                  \
; RUN:          -dtrans-soatoaosop-array-type=struct.Arr.0                                                                \
; RUN:          -dtrans-soatoaosop-base-ptr-off=2                                                                         \
; RUN:          -dtrans-soatoaosop-method-call-site-comparison=dtor                                                       \
; RUN:          -dtrans-soatoaosop-array-dtor=_ZN3ArrIPiED2Ev                                                             \
; RUN:          -dtrans-soatoaosop-array-dtor=_ZN3ArrIPfED2Ev                                                             \
; RUN:       2>&1 | FileCheck --check-prefix=CHECK-TRANS %s
;
; RUN: opt < %s -opaque-pointers -whole-program-assume -intel-libirc-allowed -disable-output                                                    \
; RUN:          -debug-only=dtrans-soatoaosop,dtrans-soatoaosop-struct                                                    \
; RUN:          -passes='require<dtrans-safetyanalyzer>,require<soatoaosop-approx>,require<soatoaosop-struct-methods>'    \
; RUN:          -dtrans-soatoaosop-array-type=struct.Arr                                                                  \
; RUN:          -dtrans-soatoaosop-array-type=struct.Arr.0                                                                \
; RUN:          -dtrans-soatoaosop-base-ptr-off=2                                                                         \
; RUN:          -dtrans-soatoaosop-method-call-site-comparison=dtor                                                       \
; RUN:          -dtrans-soatoaosop-array-dtor=_ZN3ArrIPiED2Ev                                                             \
; RUN:          -dtrans-soatoaosop-array-dtor=_ZN3ArrIPfED2Ev                                                             \
; RUN:       2>&1 | FileCheck --check-prefix=CHECK-TRANS %s
; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

%class.F = type { %struct.Arr*, %struct.Arr.0* }
%struct.Arr = type <{ i32, [4 x i8], i32**, i32, [4 x i8] }>
%struct.Arr.0 = type <{ i32, [4 x i8], float**, i32, [4 x i8] }>

declare dso_local void @_ZdlPv(i8*) local_unnamed_addr

; CHECK-TRANS: ; Checking structure's method _ZN1FD2Ev
; CHECK-TRANS: ; IR:  has only expected side-effects
; CHECK-TRANS: ; Dump instructions needing update. Total = 4

define hidden void @_ZN1FD2Ev(%class.F* nocapture readonly  "intel_dtrans_func_index"="1" %arg) !intel.dtrans.func.type !12 {
bb:
  %tmp = getelementptr inbounds %class.F, %class.F* %arg, i64 0, i32 0
; CHECK-TRANS: ; ArrayInst: Load of array
  %tmp1 = load %struct.Arr*, %struct.Arr** %tmp, align 8
  %tmp2 = icmp eq %struct.Arr* %tmp1, null
  br i1 %tmp2, label %bb5, label %bb3

bb3:                                              ; preds = %bb
; CHECK-TRANS: ; ArrayInst: Array method call
  tail call void @_ZN3ArrIPiED2Ev(%struct.Arr* nonnull %tmp1)
  %tmp4 = bitcast %struct.Arr* %tmp1 to i8*
  tail call void @_ZdlPv(i8* %tmp4)
  br label %bb5

bb5:                                              ; preds = %bb3, %bb
  %tmp6 = getelementptr inbounds %class.F, %class.F* %arg, i64 0, i32 1
; CHECK-TRANS: ; ArrayInst: Load of array
  %tmp7 = load %struct.Arr.0*, %struct.Arr.0** %tmp6, align 8
  %tmp8 = icmp eq %struct.Arr.0* %tmp7, null
  br i1 %tmp8, label %bb11, label %bb9

bb9:                                              ; preds = %bb5
; CHECK-TRANS: ; ArrayInst: Array method call
  tail call void @_ZN3ArrIPfED2Ev(%struct.Arr.0* nonnull %tmp7)
  %tmp10 = bitcast %struct.Arr.0* %tmp7 to i8*
  tail call void @_ZdlPv(i8* %tmp10)
  ret void

bb11:                                             ; preds = %bb9, %bb5
  ret void
}

declare !intel.dtrans.func.type !13 hidden void @_ZN3ArrIPiED2Ev(%struct.Arr* nocapture readonly "intel_dtrans_func_index"="1")
declare !intel.dtrans.func.type !14 hidden void @_ZN3ArrIPfED2Ev(%struct.Arr.0* nocapture readonly "intel_dtrans_func_index"="1")

; CHECK-TRANS: ; Seen dtor.
; CHECK-TRANS: ; Array call sites analysis result: required call sites can be merged

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
