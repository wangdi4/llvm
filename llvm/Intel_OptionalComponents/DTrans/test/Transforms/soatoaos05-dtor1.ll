; RUN: opt < %s -whole-program-assume -disable-output                                                                   \
; RUN:          -debug-only=dtrans-soatoaos,dtrans-soatoaos-struct                                                      \
; RUN:          -passes='require<dtransanalysis>,function(require<soatoaos-approx>,require<soatoaos-struct-methods>)'   \
; RUN:          -dtrans-soatoaos-array-type=struct.Arr                                                                  \
; RUN:          -dtrans-soatoaos-array-type=struct.Arr.0                                                                \
; RUN:          -dtrans-soatoaos-base-ptr-off=2                                                                         \
; RUN:          -dtrans-soatoaos-method-call-site-comparison=dtor                                                       \
; RUN:          -dtrans-soatoaos-array-dtor=_ZN3ArrIPiED2Ev                                                             \
; RUN:          -dtrans-soatoaos-array-dtor=_ZN3ArrIPfED2Ev                                                             \
; RUN:       2>&1 | FileCheck --check-prefix=CHECK-TRANS %s
; REQUIRES: asserts
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

%class.F = type { %struct.Arr*, %struct.Arr.0* }
%struct.Arr = type <{ i32, [4 x i8], i32**, i32, [4 x i8] }>
%struct.Arr.0 = type <{ i32, [4 x i8], float**, i32, [4 x i8] }>

declare dso_local void @_ZdlPv(i8*) local_unnamed_addr

; CHECK-TRANS: ; Checking structure's method _ZN1FD2Ev
; CHECK-TRANS: ; IR:  has only expected side-effects
; CHECK-TRANS: ; Dump instructions needing update. Total = 4

define hidden void @_ZN1FD2Ev(%class.F* nocapture readonly %arg) {
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

declare hidden void @_ZN3ArrIPiED2Ev(%struct.Arr* nocapture readonly)
declare hidden void @_ZN3ArrIPfED2Ev(%struct.Arr.0* nocapture readonly)

; CHECK-TRANS: ; Seen dtor.
; CHECK-TRANS: ; Array call sites analysis result: required call sites can be merged
