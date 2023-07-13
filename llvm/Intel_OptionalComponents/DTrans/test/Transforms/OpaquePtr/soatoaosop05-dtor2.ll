; This test verifies that SOAToAOS can prove the destructor
; calls can be merged even though EH code related to the
; the calls are not exactly same.
; Dtors: _ZN3ArrIPiED2Ev, _ZN3ArrIPfED2Ev and _ZN3ArrIPsED2Ev

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt < %s -whole-program-assume -intel-libirc-allowed -disable-output                              \
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

%class.F = type { ptr, ptr, ptr }
%struct.Arr = type <{ i8, [4 x i8], ptr, i32, [4 x i8] }>
%struct.Arr.0 = type <{ i8, [4 x i8], ptr, i32, [4 x i8] }>
%struct.Arr.1 = type <{ i8, [4 x i8], ptr, i32, [4 x i8] }>
%class.XMLMsgLoader = type opaque

; Function Attrs: allockind("free")
declare !intel.dtrans.func.type !14 dso_local void @free(ptr "intel_dtrans_func_index"="1") local_unnamed_addr #0

define hidden void @_ZN1FD2Ev(ptr nocapture readonly "intel_dtrans_func_index"="1" %arg) personality ptr @__gxx_personality_v0 !intel.dtrans.func.type !16 {
bb:
  %tmp = getelementptr inbounds %class.F, ptr %arg, i64 0, i32 0
  %tmp1 = load ptr, ptr %tmp, align 8
  %tmp2 = icmp eq ptr %tmp1, null
  br i1 %tmp2, label %bb5, label %bb3

bb3:                                              ; preds = %bb
  invoke void @_ZN3ArrIPiED2Ev(ptr nonnull %tmp1)
          to label %b6 unwind label %b30

b6:                                               ; preds = %bb3
  %tmp4 = getelementptr %struct.Arr, ptr %tmp1, i64 0, i32 0
  tail call void @_ZN10MemManager10deallocateEPv(ptr null, ptr %tmp4)
  br label %bb5

bb5:                                              ; preds = %b6, %bb
  %tmp6 = getelementptr inbounds %class.F, ptr %arg, i64 0, i32 1
  %tmp7 = load ptr, ptr %tmp6, align 8
  %tmp8 = icmp eq ptr %tmp7, null
  br i1 %tmp8, label %bba, label %bb9

bb9:                                              ; preds = %bb5
  invoke void @_ZN3ArrIPfED2Ev(ptr nonnull %tmp7)
          to label %b13 unwind label %b35

b13:                                              ; preds = %bb9
  %tmp10 = getelementptr %struct.Arr.0, ptr %tmp7, i64 0, i32 0
  tail call void @_ZN10MemManager10deallocateEPv(ptr null, ptr %tmp10)
  br label %bba

bba:                                              ; preds = %b13, %bb5
  %tmp11 = getelementptr inbounds %class.F, ptr %arg, i64 0, i32 2
  %tmp12 = load ptr, ptr %tmp11, align 8
  %tmp13 = icmp eq ptr %tmp12, null
  br i1 %tmp13, label %bbc, label %bbb

bbb:                                              ; preds = %bba
  invoke void @_ZN3ArrIPsED2Ev(ptr nonnull %tmp12)
          to label %b27 unwind label %b20

b20:                                              ; preds = %bbb
  %e21 = landingpad { ptr, i32 }
          cleanup
  %e22 = bitcast ptr %tmp12 to ptr
  invoke void @_ZN10MemManager10deallocateEPv(ptr null, ptr %e22)
          to label %b23 unwind label %b24

b23:                                              ; preds = %b20
  resume { ptr, i32 } %e21

b24:                                              ; preds = %b20
  %e25 = landingpad { ptr, i32 }
          catch ptr null
  %e26 = extractvalue { ptr, i32 } %e25, 0
  tail call void @__clang_call_terminate(ptr %e26)
  unreachable

b27:                                              ; preds = %bbb
  %tmp14 = getelementptr %struct.Arr.1, ptr %tmp12, i64 0, i32 0
  tail call void @_ZN10MemManager10deallocateEPv(ptr null, ptr %tmp14)
  br label %bbc

b30:                                              ; preds = %bb3
  %e31 = landingpad { ptr, i32 }
          cleanup
  %e32 = extractvalue { ptr, i32 } %e31, 0
  %e33 = extractvalue { ptr, i32 } %e31, 1
  %e34 = bitcast ptr %tmp1 to ptr
  invoke void @_ZN10MemManager10deallocateEPv(ptr null, ptr %e34)
          to label %b40 unwind label %b45

b35:                                              ; preds = %bb9
  %e36 = landingpad { ptr, i32 }
          cleanup
  %e37 = extractvalue { ptr, i32 } %e36, 0
  %e38 = extractvalue { ptr, i32 } %e36, 1
  %e39 = bitcast ptr %tmp7 to ptr
  invoke void @_ZN10MemManager10deallocateEPv(ptr null, ptr %e39)
          to label %b40 unwind label %b45

b40:                                              ; preds = %b35, %b30
  %e41 = phi ptr [ %e37, %b35 ], [ %e32, %b30 ]
  %e42 = phi i32 [ %e38, %b35 ], [ %e33, %b30 ]
  %e43 = insertvalue { ptr, i32 } undef, ptr %e41, 0
  %e44 = insertvalue { ptr, i32 } %e43, i32 %e42, 1
  resume { ptr, i32 } %e44

b45:                                              ; preds = %b35, %b30
  %e46 = landingpad { ptr, i32 }
          catch ptr null
  %e47 = extractvalue { ptr, i32 } %e46, 0
  tail call void @__clang_call_terminate(ptr %e47)
  unreachable

bbc:                                              ; preds = %b27, %bba
  ret void
}

define dso_local void @_ZN10MemManager10deallocateEPv(ptr nocapture "intel_dtrans_func_index"="1" %this, ptr "intel_dtrans_func_index"="2" %p) align 2 !intel.dtrans.func.type !18 {
entry:
  tail call void @free(ptr %p)
  ret void
}

declare !intel.dtrans.func.type !20 hidden void @_ZN3ArrIPiED2Ev(ptr nocapture readonly "intel_dtrans_func_index"="1")

declare !intel.dtrans.func.type !21 hidden void @_ZN3ArrIPfED2Ev(ptr nocapture readonly "intel_dtrans_func_index"="1")

declare !intel.dtrans.func.type !22 hidden void @_ZN3ArrIPsED2Ev(ptr nocapture readonly "intel_dtrans_func_index"="1")

declare !intel.dtrans.func.type !23 void @__clang_call_terminate(ptr "intel_dtrans_func_index"="1")

declare i32 @__gxx_personality_v0(...)

attributes #0 = { allockind("free") "alloc-family"="malloc" }

!intel.dtrans.types = !{!0, !5, !7, !11, !13}

!0 = !{!"S", %struct.Arr zeroinitializer, i32 5, !1, !2, !3, !4, !2}
!1 = !{i8 0, i32 0}
!2 = !{!"A", i32 4, !1}
!3 = !{i32 0, i32 2}
!4 = !{i32 0, i32 0}
!5 = !{!"S", %struct.Arr.0 zeroinitializer, i32 5, !1, !2, !6, !4, !2}
!6 = !{float 0.000000e+00, i32 2}
!7 = !{!"S", %class.F zeroinitializer, i32 3, !8, !9, !10}
!8 = !{%struct.Arr zeroinitializer, i32 1}
!9 = !{%struct.Arr.0 zeroinitializer, i32 1}
!10 = !{%struct.Arr.1 zeroinitializer, i32 1}
!11 = !{!"S", %struct.Arr.1 zeroinitializer, i32 5, !1, !2, !12, !4, !2}
!12 = !{i16 0, i32 2}
!13 = !{!"S", %class.XMLMsgLoader zeroinitializer, i32 -1}
!14 = distinct !{!15}
!15 = !{i8 0, i32 1}
!16 = distinct !{!17}
!17 = !{%class.F zeroinitializer, i32 1}
!18 = distinct !{!19, !15}
!19 = !{%class.XMLMsgLoader zeroinitializer, i32 1}
!20 = distinct !{!8}
!21 = distinct !{!9}
!22 = distinct !{!10}
!23 = distinct !{!15}
