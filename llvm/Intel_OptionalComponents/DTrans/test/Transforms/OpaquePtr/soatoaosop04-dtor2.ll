; RUN: opt < %s -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -disable-output                                                       \
; RUN: -passes='require<dtrans-safetyanalyzer>,require<soatoaosop-approx>,require<soatoaosop-array-methods>' \
; RUN:        -dtrans-soatoaosop-base-ptr-off=3 -dtrans-soatoaosop-mem-off=4                                    \
; RUN:        -debug-only=dtrans-soatoaosop,dtrans-soatoaosop-arrays                                            \
; RUN:        -dtrans-soatoaosop-ignore-funcs=_ZN10MemManager10deallocateEPv                                    \
; RUN:        -dtrans-soatoaosop-ignore-funcs=dummyDeallocateEPv                                                \
; RUN:        2>&1 | FileCheck %s
;
; RUN: opt < %s -dtransop-allow-typed-pointers -opaque-pointers -whole-program-assume -intel-libirc-allowed -disable-output                                                       \
; RUN: -passes='require<dtrans-safetyanalyzer>,require<soatoaosop-approx>,require<soatoaosop-array-methods>' \
; RUN:        -dtrans-soatoaosop-base-ptr-off=3 -dtrans-soatoaosop-mem-off=4                                    \
; RUN:        -debug-only=dtrans-soatoaosop,dtrans-soatoaosop-arrays                                            \
; RUN:        -dtrans-soatoaosop-ignore-funcs=_ZN10MemManager10deallocateEPv                                    \
; RUN:        -dtrans-soatoaosop-ignore-funcs=dummyDeallocateEPv                                                \
; RUN:        2>&1 | FileCheck  --check-prefix=CHECK-OP %s
; REQUIRES: asserts

; Test checks deallocation functions handling after devirtualization.
; The test is similar to soatoaosop04-dtor.ll.
; 2 versions of deallocation functions are processed, although no processing of
; deallocation is needed.
; Comparisons of pointers to functions from vtable of XMLMsgLoader is a known
; side-effect.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%class.ValueVectorOf = type { i8, i32, i32, %class.IC_Field**, %class.XMLMsgLoader* }
%class.IC_Field = type opaque
%class.XMLMsgLoader = type { i32 (...)** }
%class.bad_alloc = type { %class.exception }
%class.exception = type { i32 (...)** }

; CHECK:      ; Classification: Dtor method
; CHECK-NEXT: ; Dump instructions needing update. Total = 1
; CHECK-OP:      ; Classification: Dtor method
; CHECK-OP-NEXT: ; Dump instructions needing update. Total = 1
define hidden void @"ValueVectorOf<IC_Field*>::~ValueVectorOf()"(%class.ValueVectorOf* nocapture readonly "intel_dtrans_func_index"="1" %arg) personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) !intel.dtrans.func.type !12 {
bb:
  %tmp = getelementptr inbounds %class.ValueVectorOf, %class.ValueVectorOf* %arg, i64 0, i32 4
  %tmp1 = load %class.XMLMsgLoader*, %class.XMLMsgLoader** %tmp
  %tmp2 = getelementptr inbounds %class.ValueVectorOf, %class.ValueVectorOf* %arg, i64 0, i32 3
  %tmp3 = bitcast %class.IC_Field*** %tmp2 to i8**
; CHECK:      ; BasePtrInst: Load of base pointer
; CHECK-NEXT:   %tmp4 = load i8*, i8** %tmp3
; CHECK-OP:      ; BasePtrInst: Load of base pointer
; CHECK-OP-NEXT:   %tmp4 = load ptr, ptr %tmp3
  %tmp4 = load i8*, i8** %tmp3
  %tmp5 = bitcast %class.XMLMsgLoader* %tmp1 to void (%class.XMLMsgLoader*, i8*)***
  %tmp6 = load void (%class.XMLMsgLoader*, i8*)**, void (%class.XMLMsgLoader*, i8*)*** %tmp5
  %tmp7 = bitcast void (%class.XMLMsgLoader*, i8*)** %tmp6 to i8*
  %tmp8 = getelementptr inbounds void (%class.XMLMsgLoader*, i8*)*, void (%class.XMLMsgLoader*, i8*)** %tmp6, i64 3
  %tmp9 = load void (%class.XMLMsgLoader*, i8*)*, void (%class.XMLMsgLoader*, i8*)** %tmp8
  %tmp10 = bitcast void (%class.XMLMsgLoader*, i8*)* %tmp9 to i8*
  %tmp11 = bitcast void (%class.XMLMsgLoader*, i8*)* @_ZN10MemManager10deallocateEPv to i8*
  %tmp12 = icmp eq i8* %tmp10, %tmp11
; Known side-effect.
  br i1 %tmp12, label %bb13, label %bb14

bb13:                                             ; preds = %bb
  invoke void @_ZN10MemManager10deallocateEPv(%class.XMLMsgLoader* %tmp1, i8* %tmp4)
            to label %bb15 unwind label %terminate

bb14:                                             ; preds = %bb
  invoke void @dummyDeallocateEPv(%class.XMLMsgLoader* %tmp1, i8* %tmp4)
            to label %bb15 unwind label %terminate

bb15:                                             ; preds = %bb14, %bb13
  br label %bb16

bb16:                                             ; preds = %bb15
  ret void

terminate:                                        ; preds = %bb14, %bb13
  %lp = landingpad { i8*, i32 }
          catch i8* null
  %ext = extractvalue { i8*, i32 } %lp, 0
  tail call void @__clang_call_terminate(i8* %ext)
  unreachable
}

define dso_local void @_ZN10MemManager10deallocateEPv(%class.XMLMsgLoader* nocapture "intel_dtrans_func_index"="1" %this, i8* "intel_dtrans_func_index"="2" %p) align 2 !intel.dtrans.func.type !14 {
entry:
  tail call void @free(i8* %p)
  ret void
}

define void @dummyDeallocateEPv(%class.XMLMsgLoader* "intel_dtrans_func_index"="1", i8* "intel_dtrans_func_index"="2") !intel.dtrans.func.type !16 {
  %call = tail call i8* @__cxa_allocate_exception(i64 8)
  %bc = bitcast i8* %call to %class.bad_alloc*
  %gep = getelementptr inbounds %class.bad_alloc, %class.bad_alloc* %bc, i64 0, i32 0, i32 0
  store i32 (...)** null, i32 (...)*** %gep, align 8
  tail call void @__cxa_throw(i8* nonnull %call, i8* null, i8* null)
  unreachable
}

declare i32 @__gxx_personality_v0(...)
declare !intel.dtrans.func.type !12 dso_local void @__clang_call_terminate(i8* "intel_dtrans_func_index"="1") unnamed_addr
declare !intel.dtrans.func.type !17 void @free(i8* "intel_dtrans_func_index"="1") #0
declare !intel.dtrans.func.type !18 "intel_dtrans_func_index"="1" i8* @__cxa_allocate_exception(i64)
declare !intel.dtrans.func.type !19 void @__cxa_throw(i8* "intel_dtrans_func_index"="1", i8* "intel_dtrans_func_index"="2", i8* "intel_dtrans_func_index"="3")

attributes #0 = { allockind("free") "alloc-family"="malloc" }

!intel.dtrans.types = !{!0, !1, !5, !9, !11}

!0 = !{!"S", %class.IC_Field zeroinitializer, i32 -1}
!1 = !{!"S", %class.XMLMsgLoader zeroinitializer, i32 1, !2}
!2 = !{!3, i32 2}
!3 = !{!"F", i1 true, i32 0, !4}
!4 = !{i32 0, i32 0}
!5 = !{!"S", %class.ValueVectorOf zeroinitializer, i32 5, !6, !4, !4, !7, !8}
!6 = !{i8 0, i32 0}
!7 = !{%class.IC_Field zeroinitializer, i32 2}
!8 = !{%class.XMLMsgLoader zeroinitializer, i32 1}
!9 = !{!"S", %class.bad_alloc zeroinitializer, i32 1, !10}
!10 = !{%class.exception zeroinitializer, i32 0}
!11 = !{!"S", %class.exception zeroinitializer, i32 1, !2}
!12 = distinct !{!13}
!13 = !{%class.ValueVectorOf zeroinitializer, i32 1}
!14 = distinct !{!8, !15}
!15 = !{i8 0, i32 1}
!16 = distinct !{!8, !15}
!17 = distinct !{!15}
!18 = distinct !{!15}
!19 = distinct !{!15, !15, !15}
