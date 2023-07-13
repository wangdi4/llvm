; RUN: opt < %s -whole-program-assume -intel-libirc-allowed -disable-output                                                       \
; RUN: -passes='require<dtrans-safetyanalyzer>,require<soatoaosop-approx>,require<soatoaosop-array-methods>' \
; RUN:        -dtrans-soatoaosop-base-ptr-off=3 -dtrans-soatoaosop-mem-off=4                                    \
; RUN:        -debug-only=dtrans-soatoaosop,dtrans-soatoaosop-arrays                                            \
; RUN:        2>&1 | FileCheck %s
; RUN: opt -S < %s -whole-program-assume -intel-libirc-allowed                                                                    \
; RUN:        -passes=soatoaosop-arrays-methods-transform                                                     \
; RUN:        -dtrans-soatoaosop-base-ptr-off=3 -dtrans-soatoaosop-mem-off=4                                    \
; RUN:        | FileCheck --check-prefix=CHECK-MOD %s
; REQUIRES: asserts
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%class.ValueVectorOf = type { i8, i32, i32, ptr, ptr }
%class.DatatypeValidator = type opaque
%class.XMLMsgLoader = type { ptr }
%class.XMLException = type opaque
%class.XMLPlatformUtilsException = type opaque
; CHECK-MOD-DAG: %__SOA_class.ValueVectorOf = type { i8, i32, i32, ptr, ptr }
; CHECK-MOD-DAG: %__SOA_EL_class.ValueVectorOf = type { ptr, ptr }

@.str.1.2092 = external constant [31 x i8]

; The following method should be classified as get-like method.
; Instructions to transform are shown.
; Transformed instructions are shown.
;  template <class TElem>
;  TElem &ValueVectorOf<TElem>::elementAt(const unsigned int getAt) {
;    if (getAt >= fCurCount)
;      ThrowXMLwithMemMgr(ArrayIndexOutOfBoundsException,
;                         XMLExcepts::Vector_BadIndex, fMemoryManager);
;    return fElemList[getAt];
;  }
; CHECK:; Classification: Get pointer to element method
; CHECK:; Dump instructions needing update. Total = 3

define "intel_dtrans_func_index"="1" ptr @"ValueVectorOf<DatatypeValidator*>::elementAt(unsigned int)"(ptr "intel_dtrans_func_index"="2" %this, i32 %getAt) personality ptr @__gxx_personality_v0 !intel.dtrans.func.type !11 {
entry:
  %fCurCount = getelementptr inbounds %class.ValueVectorOf, ptr %this, i64 0, i32 1
  %tmp = load i32, ptr %fCurCount, align 4
  %cmp = icmp ugt i32 %tmp, %getAt
  br i1 %cmp, label %if.end, label %if.then

if.then:                                          ; preds = %entry
  %exception = tail call ptr @__cxa_allocate_exception(i64 48)
  %tmp1 = bitcast ptr %exception to ptr
  %fMemoryManager = getelementptr inbounds %class.ValueVectorOf, ptr %this, i64 0, i32 4
  %tmp2 = load ptr, ptr %fMemoryManager, align 8
  invoke void @"ArrayIndexOutOfBoundsException::ArrayIndexOutOfBoundsException(char const*, unsigned int, XMLExcepts::Codes, MemoryManager*)"(ptr nonnull %tmp1, ptr @.str.1.2092, i32 206, i32 116, ptr %tmp2)
          to label %invoke.cont unwind label %lpad

invoke.cont:                                      ; preds = %if.then
  tail call void @__cxa_throw(ptr nonnull %exception, ptr null, ptr @"XMLException::~XMLException()")
  unreachable

lpad:                                             ; preds = %if.then
  %tmp3 = landingpad { ptr, i32 }
          cleanup
  tail call void @__cxa_free_exception(ptr nonnull %exception)
  resume { ptr, i32 } %tmp3

if.end:                                           ; preds = %entry
; CHECK-MOD:  %fElemList = getelementptr inbounds %__SOA_class.ValueVectorOf, ptr %this, i64 0, i32 3
  %fElemList = getelementptr inbounds %class.ValueVectorOf, ptr %this, i64 0, i32 3
; CHECK:     ; BasePtrInst: Load of base pointer
; CHECK-NEXT:  %tmp4 = load ptr, ptr %fElemList
; CHECK-MOD:   %tmp4 = load ptr, ptr %fElemList
  %tmp4 = load ptr, ptr %fElemList, align 8
  %idxprom = zext i32 %getAt to i64
; CHECK:     ; MemInstGEP: Address in ret
; CHECK-NEXT:  %arrayidx = getelementptr inbounds ptr, ptr %tmp4, i64 %idxprom
; CHECK-MOD:   %arrayidx = getelementptr inbounds %__SOA_EL_class.ValueVectorOf, ptr %tmp4, i64 %idxprom
  %arrayidx = getelementptr inbounds ptr, ptr %tmp4, i64 %idxprom
; CHECK-MOD-NEXT:  %elem = getelementptr inbounds %__SOA_EL_class.ValueVectorOf, ptr %arrayidx, i64 0, i32 1
; CHECK-MOD-NEXT:  ret ptr %elem
; CHECK:     ; MemInst: Address in ret
; CHECK-NEXT:  ret ptr %arrayidx
  ret ptr %arrayidx
}

declare i32 @__gxx_personality_v0(...)

declare !intel.dtrans.func.type !13 noalias nonnull "intel_dtrans_func_index"="1" ptr @__cxa_allocate_exception(i64)

declare !intel.dtrans.func.type !15 void @__cxa_throw(ptr "intel_dtrans_func_index"="1", ptr "intel_dtrans_func_index"="2", ptr "intel_dtrans_func_index"="3")

declare !intel.dtrans.func.type !16 void @__cxa_free_exception(ptr "intel_dtrans_func_index"="1")

declare !intel.dtrans.func.type !17 void @"XMLException::~XMLException()"(ptr "intel_dtrans_func_index"="1")

declare !intel.dtrans.func.type !19 void @"ArrayIndexOutOfBoundsException::ArrayIndexOutOfBoundsException(char const*, unsigned int, XMLExcepts::Codes, MemoryManager*)"(ptr "intel_dtrans_func_index"="1", ptr "intel_dtrans_func_index"="2", i32, i32, ptr "intel_dtrans_func_index"="3")

!intel.dtrans.types = !{!0, !1, !5, !9, !10}

!0 = !{!"S", %class.DatatypeValidator zeroinitializer, i32 -1}
!1 = !{!"S", %class.XMLMsgLoader zeroinitializer, i32 1, !2}
!2 = !{!3, i32 2}
!3 = !{!"F", i1 true, i32 0, !4}
!4 = !{i32 0, i32 0}
!5 = !{!"S", %class.ValueVectorOf zeroinitializer, i32 5, !6, !4, !4, !7, !8}
!6 = !{i8 0, i32 0}
!7 = !{%class.DatatypeValidator zeroinitializer, i32 2}
!8 = !{%class.XMLMsgLoader zeroinitializer, i32 1}
!9 = !{!"S", %class.XMLException zeroinitializer, i32 -1}
!10 = !{!"S", %class.XMLPlatformUtilsException zeroinitializer, i32 -1}
!11 = distinct !{!7, !12}
!12 = !{%class.ValueVectorOf zeroinitializer, i32 1}
!13 = distinct !{!14}
!14 = !{i8 0, i32 1}
!15 = distinct !{!14, !14, !14}
!16 = distinct !{!14}
!17 = distinct !{!18}
!18 = !{%class.XMLException zeroinitializer, i32 1}
!19 = distinct !{!20, !14, !8}
!20 = !{%class.XMLPlatformUtilsException zeroinitializer, i32 1}
