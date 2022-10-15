; This test verifies that SOAToAOS can prove the destructor
; calls can be merged even though EH code related to the
; the calls are not exactly same.
; Dtors: _ZN3ArrIPiED2Ev, _ZN3ArrIPfED2Ev and _ZN3ArrIPsED2Ev

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt < %s -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -disable-output                                      \
; RUN:          -debug-only=dtrans-soatoaosop,dtrans-soatoaosop-struct                                                    \
; RUN:          -passes='require<dtrans-safetyanalyzer>,require<soatoaosop-approx>,require<soatoaosop-struct-methods>'    \
; RUN:          -dtrans-soatoaosop-array-type=struct.Arr                                                                  \
; RUN:          -dtrans-soatoaosop-array-type=struct.Arr.0                                                                \
; RUN:          -dtrans-soatoaosop-array-type=struct.Arr.1                                                                \
; RUN:          -dtrans-soatoaosop-base-ptr-off=2                                                                         \
; RUN:          -dtrans-soatoaosop-method-call-site-comparison=dtor                                                       \
; RUN:          -dtrans-soatoaosop-ignore-funcs=_ZN10MemManager10deallocateEPv                                            \
; RUN:          -dtrans-soatoaosop-array-dtor=_ZN3ArrIPiED2Ev                                                             \
; RUN:          -dtrans-soatoaosop-array-dtor=_ZN3ArrIPfED2Ev                                                             \
; RUN:          -dtrans-soatoaosop-array-dtor=_ZN3ArrIPsED2Ev                                                             \
; RUN:       2>&1 | FileCheck  %s

; RUN: opt < %s -opaque-pointers -whole-program-assume -intel-libirc-allowed -disable-output                                                    \
; RUN:          -debug-only=dtrans-soatoaosop,dtrans-soatoaosop-struct                                                    \
; RUN:          -passes='require<dtrans-safetyanalyzer>,require<soatoaosop-approx>,require<soatoaosop-struct-methods>'    \
; RUN:          -dtrans-soatoaosop-array-type=struct.Arr                                                                  \
; RUN:          -dtrans-soatoaosop-array-type=struct.Arr.0                                                                \
; RUN:          -dtrans-soatoaosop-array-type=struct.Arr.1                                                                \
; RUN:          -dtrans-soatoaosop-base-ptr-off=2                                                                         \
; RUN:          -dtrans-soatoaosop-method-call-site-comparison=dtor                                                       \
; RUN:          -dtrans-soatoaosop-ignore-funcs=_ZN10MemManager10deallocateEPv                                            \
; RUN:          -dtrans-soatoaosop-array-dtor=_ZN3ArrIPiED2Ev                                                             \
; RUN:          -dtrans-soatoaosop-array-dtor=_ZN3ArrIPfED2Ev                                                             \
; RUN:          -dtrans-soatoaosop-array-dtor=_ZN3ArrIPsED2Ev                                                             \
; RUN:       2>&1 | FileCheck  %s

; REQUIRES: asserts
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

; CHECK: ; Seen dtor.
; CHECK: ; Array call sites analysis result: required call sites can be merged


%class.F = type { %struct.Arr*, %struct.Arr.0*, %struct.Arr.1* }
%struct.Arr = type <{ i8, [4 x i8], i32**, i32, [4 x i8] }>
%struct.Arr.0 = type <{ i8, [4 x i8], float**, i32, [4 x i8] }>
%struct.Arr.1 = type <{ i8, [4 x i8], i16**, i32, [4 x i8] }>
%class.XMLMsgLoader = type opaque

declare !intel.dtrans.func.type !21 dso_local void @free(i8* "intel_dtrans_func_index"="1") local_unnamed_addr #0

define hidden void @_ZN1FD2Ev(%class.F* nocapture readonly "intel_dtrans_func_index"="1" %arg) personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) !intel.dtrans.func.type !12 {
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
  tail call void @_ZN10MemManager10deallocateEPv(%class.XMLMsgLoader* null, i8* %tmp4)
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
  tail call void @_ZN10MemManager10deallocateEPv(%class.XMLMsgLoader* null, i8* %tmp10)
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
  %e22 = bitcast %struct.Arr.1* %tmp12 to i8*
  invoke void @_ZN10MemManager10deallocateEPv(%class.XMLMsgLoader* null, i8* %e22)
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
  tail call void @_ZN10MemManager10deallocateEPv(%class.XMLMsgLoader* null, i8* %tmp14)
  br label %bbc

b30:                                               ; preds = %5
  %e31 = landingpad { i8*, i32 }
          cleanup
  %e32 = extractvalue { i8*, i32 } %e31, 0
  %e33 = extractvalue { i8*, i32 } %e31, 1
  %e34 = bitcast %struct.Arr* %tmp1 to i8*
  invoke void @_ZN10MemManager10deallocateEPv(%class.XMLMsgLoader* null, i8* %e34)
          to label %b40 unwind label %b45

b35:                                               ; preds = %12
  %e36 = landingpad { i8*, i32 }
          cleanup
  %e37 = extractvalue { i8*, i32 } %e36, 0
  %e38 = extractvalue { i8*, i32 } %e36, 1
  %e39 = bitcast %struct.Arr.0* %tmp7 to i8*
  invoke void @_ZN10MemManager10deallocateEPv(%class.XMLMsgLoader* null, i8* %e39)
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

define dso_local void @_ZN10MemManager10deallocateEPv(%class.XMLMsgLoader* nocapture "intel_dtrans_func_index"="1" %this, i8* "intel_dtrans_func_index"="2" %p) align 2 !intel.dtrans.func.type !25 {
entry:
  tail call void @free(i8* %p)
  ret void
}

declare !intel.dtrans.func.type !13 hidden void @_ZN3ArrIPiED2Ev(%struct.Arr* nocapture readonly "intel_dtrans_func_index"="1")
declare !intel.dtrans.func.type !14 hidden void @_ZN3ArrIPfED2Ev(%struct.Arr.0* nocapture readonly "intel_dtrans_func_index"="1")
declare !intel.dtrans.func.type !18 hidden void @_ZN3ArrIPsED2Ev(%struct.Arr.1* nocapture readonly "intel_dtrans_func_index"="1")
declare !intel.dtrans.func.type !20 void @__clang_call_terminate(i8* "intel_dtrans_func_index"="1")
declare i32 @__gxx_personality_v0(...)

attributes #0 = { allockind("free") "alloc-family"="malloc" }

!intel.dtrans.types = !{!1, !6, !10, !15, !23}

!1 = !{!"S", %struct.Arr zeroinitializer, i32 5, !3, !2, !4, !5, !2}
!2 = !{!"A", i32 4, !3}
!3 = !{i8 0, i32 0}
!4 = !{i32 0, i32 2}
!5 = !{i32 0, i32 0}
!6 = !{!"S", %struct.Arr.0 zeroinitializer, i32 5, !3, !2, !7, !5, !2}
!7 = !{float 0.000000e+00, i32 2}
!8 = !{%struct.Arr zeroinitializer, i32 1}
!9 = !{%struct.Arr.0 zeroinitializer, i32 1}
!10 = !{!"S", %class.F zeroinitializer, i32 3, !8, !9, !17}
!11 = !{%class.F zeroinitializer, i32 1}
!12 = distinct !{!11}
!13 = distinct !{!8}
!14 = distinct !{!9}
!15 = !{!"S", %struct.Arr.1 zeroinitializer, i32 5, !3, !2, !16, !5, !2}
!16 = !{i16 0, i32 2}
!17 = !{%struct.Arr.1 zeroinitializer, i32 1}
!18 = distinct !{!17}
!19 = !{i8 0, i32 1}
!20 = distinct !{!19}
!21 = distinct !{!22}
!22 = !{i8 0, i32 1}
!23 = !{!"S", %class.XMLMsgLoader zeroinitializer, i32 -1}
!24 = !{%class.XMLMsgLoader zeroinitializer, i32 1}
!25 = distinct !{!24, !19}
