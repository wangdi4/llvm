; This test verifies that SOAToAOS can't prove the destructor
; calls can be merged because EH code related to the calls
; are not semantically same.
; This test is same as soatoaos05-dtor2.ll except landingpad
; instruction in %b20 block has extra clause.
; Dtors: _ZN3ArrIPiED2Ev, _ZN3ArrIPfED2Ev and _ZN3ArrIPsED2Ev

; RUN: opt < %s -whole-program-assume -disable-output                                                                   \
; RUN:          -debug-only=dtrans-soatoaos,dtrans-soatoaos-struct                                                      \
; RUN:          -passes='require<dtransanalysis>,require<soatoaos-approx>,require<soatoaos-struct-methods>'             \
; RUN:          -dtrans-soatoaos-array-type=struct.Arr                                                                  \
; RUN:          -dtrans-soatoaos-array-type=struct.Arr.0                                                                \
; RUN:          -dtrans-soatoaos-array-type=struct.Arr.1                                                                \
; RUN:          -dtrans-soatoaos-base-ptr-off=2                                                                         \
; RUN:          -dtrans-soatoaos-method-call-site-comparison=dtor                                                       \
; RUN:          -dtrans-soatoaos-array-dtor=_ZN3ArrIPiED2Ev                                                             \
; RUN:          -dtrans-soatoaos-array-dtor=_ZN3ArrIPfED2Ev                                                             \
; RUN:          -dtrans-soatoaos-array-dtor=_ZN3ArrIPsED2Ev                                                             \
; RUN:          -dtrans-free-functions=_ZdlPv                                                            \
; RUN:       2>&1 | FileCheck  %s

; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

; CHECK: ; Seen dtor.
; CHECK: ; Array call sites analysis result: problem with call sites required to be merged
; CHECK-NOT: ; Array call sites analysis result: required call sites can be merged


%class.F = type { %struct.Arr*, %struct.Arr.0*, %struct.Arr.1* }
%struct.Arr = type <{ i8, [4 x i8], i32**, i32, [4 x i8] }>
%struct.Arr.0 = type <{ i8, [4 x i8], float**, i32, [4 x i8] }>
%struct.Arr.1 = type <{ i8, [4 x i8], i16**, i32, [4 x i8] }>

declare dso_local void @_ZdlPv(i8*) local_unnamed_addr

define hidden void @_ZN1FD2Ev(%class.F* nocapture readonly %arg) personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
bb:
  %tmp = getelementptr inbounds %class.F, %class.F* %arg, i64 0, i32 0
  %tmp1 = load %struct.Arr*, %struct.Arr** %tmp, align 8
  %tmp2 = icmp eq %struct.Arr* %tmp1, null
  br i1 %tmp2, label %bb5, label %bb3

bb3:                                              ; preds = %bb
  invoke void @_ZN3ArrIPiED2Ev(%struct.Arr* nonnull %tmp1)
    to label %b6 unwind label %b30

b6:
  %tmp4 = getelementptr %struct.Arr, %struct.Arr* %tmp1, i64 0, i32 0
  tail call void @_ZdlPv(i8* %tmp4)
  br label %bb5

bb5:                                              ; preds = %bb3, %bb
  %tmp6 = getelementptr inbounds %class.F, %class.F* %arg, i64 0, i32 1
  %tmp7 = load %struct.Arr.0*, %struct.Arr.0** %tmp6, align 8
  %tmp8 = icmp eq %struct.Arr.0* %tmp7, null
  br i1 %tmp8, label %bba, label %bb9

bb9:                                              ; preds = %bb5
  invoke void @_ZN3ArrIPfED2Ev(%struct.Arr.0* nonnull %tmp7)
   to label %b13 unwind label %b35

b13:
  %tmp10 = getelementptr %struct.Arr.0, %struct.Arr.0* %tmp7, i64 0, i32 0
  tail call void @_ZdlPv(i8* %tmp10)
  br label %bba

bba:                                              ; preds = %bb9, %bb5
  %tmp11 = getelementptr inbounds %class.F, %class.F* %arg, i64 0, i32 2
  %tmp12 = load %struct.Arr.1*, %struct.Arr.1** %tmp11, align 8
  %tmp13 = icmp eq %struct.Arr.1* %tmp12, null
  br i1 %tmp13, label %bbc, label %bbb

bbb:                                              ; preds = %bba
  invoke void @_ZN3ArrIPsED2Ev(%struct.Arr.1* nonnull %tmp12)
     to label %b27 unwind label %b20

b20:
  %e21 = landingpad { i8*, i32 }
          cleanup
          catch i8* null
  %e22 = bitcast %struct.Arr.1* %tmp12 to i8*
  invoke void @_ZdlPv(i8* %e22)
          to label %b23 unwind label %b24

b23:                                               ; preds = %b20
  resume { i8*, i32 } %e21

b24:                                               ; preds = %b20
  %e25 = landingpad { i8*, i32 }
          catch i8* null
  %e26 = extractvalue { i8*, i32 } %e25, 0
  tail call void @__clang_call_terminate(i8* %e26)
  unreachable

b27:
  %tmp14 = getelementptr %struct.Arr.1, %struct.Arr.1* %tmp12, i64 0, i32 0
  tail call void @_ZdlPv(i8* %tmp14)
  br label %bbc

b30:                                               ; preds = %5
  %e31 = landingpad { i8*, i32 }
          cleanup
  %e32 = extractvalue { i8*, i32 } %e31, 0
  %e33 = extractvalue { i8*, i32 } %e31, 1
  %e34 = bitcast %struct.Arr* %tmp1 to i8*
  invoke void @_ZdlPv(i8* %e34)
          to label %b40 unwind label %b45

b35:                                               ; preds = %12
  %e36 = landingpad { i8*, i32 }
          cleanup
  %e37 = extractvalue { i8*, i32 } %e36, 0
  %e38 = extractvalue { i8*, i32 } %e36, 1
  %e39 = bitcast %struct.Arr.0* %tmp7 to i8*
  invoke void @_ZdlPv(i8* %e39)
          to label %b40 unwind label %b45

b40:                                               ; preds = %35, %30
  %e41 = phi i8* [ %e37, %b35 ], [ %e32, %b30 ]
  %e42 = phi i32 [ %e38, %b35 ], [ %e33, %b30 ]
  %e43 = insertvalue { i8*, i32 } undef, i8* %e41, 0
  %e44 = insertvalue { i8*, i32 } %e43, i32 %e42, 1
  resume { i8*, i32 } %e44

b45:                                               ; preds = %b35, %b30
  %e46 = landingpad { i8*, i32 }
          catch i8* null
  %e47 = extractvalue { i8*, i32 } %e46, 0
  tail call void @__clang_call_terminate(i8* %e47)
  unreachable

bbc:                                             ; preds = %bbb, %bba
  ret void
}

declare hidden void @_ZN3ArrIPiED2Ev(%struct.Arr* nocapture readonly)
declare hidden void @_ZN3ArrIPfED2Ev(%struct.Arr.0* nocapture readonly)
declare hidden void @_ZN3ArrIPsED2Ev(%struct.Arr.1* nocapture readonly)
declare void @__clang_call_terminate(i8*)
declare i32 @__gxx_personality_v0(...)

