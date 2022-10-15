; RUN: opt < %s -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -disable-output                                                       \
; RUN: -passes='require<dtrans-safetyanalyzer>,require<soatoaosop-approx>,require<soatoaosop-array-methods>' \
; RUN:        -dtrans-soatoaosop-base-ptr-off=3 -dtrans-soatoaosop-mem-off=4                                    \
; RUN:        -debug-only=dtrans-soatoaosop,dtrans-soatoaosop-arrays                                            \
; RUN:        2>&1 | FileCheck %s
; RUN: opt -S < %s -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed                                                                    \
; RUN:        -passes=soatoaosop-arrays-methods-transform                                                     \
; RUN:        -dtrans-soatoaosop-base-ptr-off=3 -dtrans-soatoaosop-mem-off=4                                    \
; RUN:        | FileCheck --check-prefix=CHECK-MOD %s
;
; RUN: opt < %s -opaque-pointers -whole-program-assume -intel-libirc-allowed -disable-output                                                       \
; RUN: -passes='require<dtrans-safetyanalyzer>,require<soatoaosop-approx>,require<soatoaosop-array-methods>' \
; RUN:        -dtrans-soatoaosop-base-ptr-off=3 -dtrans-soatoaosop-mem-off=4                                    \
; RUN:        -debug-only=dtrans-soatoaosop,dtrans-soatoaosop-arrays                                            \
; RUN:        2>&1 | FileCheck --check-prefix=CHECK-OP %s
; RUN: opt -S < %s -opaque-pointers -whole-program-assume -intel-libirc-allowed                                                                    \
; RUN:        -passes=soatoaosop-arrays-methods-transform                                                     \
; RUN:        -dtrans-soatoaosop-base-ptr-off=3 -dtrans-soatoaosop-mem-off=4                                    \
; RUN:        | FileCheck --check-prefix=CHECK-OP-MOD %s
; REQUIRES: asserts
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%class.XMLPlatformUtilsException = type opaque
%class.XMLMsgLoader = type { i32 (...)** }
%class.DatatypeValidator = type opaque
%class.ValueVectorOf = type { i8, i32, i32, %class.DatatypeValidator**, %class.XMLMsgLoader* }
%class.XMLException = type opaque
; CHECK-MOD-DAG: %__SOA_class.ValueVectorOf = type { i8, i32, i32, %__SOA_EL_class.ValueVectorOf*, %class.XMLMsgLoader* }
; CHECK-MOD-DAG: %__SOA_EL_class.ValueVectorOf = type { float*, %class.DatatypeValidator* }
; CHECK-OP-MOD-DAG: %__SOA_class.ValueVectorOf = type { i8, i32, i32, ptr, ptr }
; CHECK-OP-MOD-DAG: %__SOA_EL_class.ValueVectorOf = type { ptr, ptr }

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
; CHECK-OP:; Classification: Get pointer to element method
; CHECK-OP:; Dump instructions needing update. Total = 3
define "intel_dtrans_func_index"="1" %class.DatatypeValidator** @"ValueVectorOf<DatatypeValidator*>::elementAt(unsigned int)"(%class.ValueVectorOf* "intel_dtrans_func_index"="2" %this, i32 %getAt) personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) !intel.dtrans.func.type !11 {
entry:
  %fCurCount = getelementptr inbounds %class.ValueVectorOf, %class.ValueVectorOf* %this, i64 0, i32 1
  %tmp = load i32, i32* %fCurCount
  %cmp = icmp ugt i32 %tmp, %getAt
  br i1 %cmp, label %if.end, label %if.then

if.then:                                          ; preds = %entry
  %exception = tail call i8* @__cxa_allocate_exception(i64 48)
  %tmp1 = bitcast i8* %exception to %class.XMLPlatformUtilsException*
  %fMemoryManager = getelementptr inbounds %class.ValueVectorOf, %class.ValueVectorOf* %this, i64 0, i32 4
  %tmp2 = load %class.XMLMsgLoader*, %class.XMLMsgLoader** %fMemoryManager
  invoke void @"ArrayIndexOutOfBoundsException::ArrayIndexOutOfBoundsException(char const*, unsigned int, XMLExcepts::Codes, MemoryManager*)"(%class.XMLPlatformUtilsException* nonnull %tmp1, i8* getelementptr inbounds ([31 x i8], [31 x i8]* @.str.1.2092, i64 0, i64 0), i32 206, i32 116, %class.XMLMsgLoader* %tmp2)
          to label %invoke.cont unwind label %lpad

invoke.cont:                                      ; preds = %if.then
  tail call void @__cxa_throw(i8* nonnull %exception, i8* null, i8* bitcast (void (%class.XMLException*)* @"XMLException::~XMLException()" to i8*))
  unreachable

lpad:                                             ; preds = %if.then
  %tmp3 = landingpad { i8*, i32 }
          cleanup
  tail call void @__cxa_free_exception(i8* nonnull %exception)
  resume { i8*, i32 } %tmp3

if.end:                                           ; preds = %entry
; CHECK-MOD:  %fElemList = getelementptr inbounds %__SOA_class.ValueVectorOf, %__SOA_class.ValueVectorOf* %this, i64 0, i32 3
; CHECK-OP-MOD:  %fElemList = getelementptr inbounds %__SOA_class.ValueVectorOf, ptr %this, i64 0, i32 3
  %fElemList = getelementptr inbounds %class.ValueVectorOf, %class.ValueVectorOf* %this, i64 0, i32 3
; CHECK:     ; BasePtrInst: Load of base pointer
; CHECK-NEXT:  %tmp4 = load %class.DatatypeValidator**, %class.DatatypeValidator*** %fElemList
; CHECK-MOD:   %tmp4 = load %__SOA_EL_class.ValueVectorOf*, %__SOA_EL_class.ValueVectorOf** %fElemList
; CHECK-OP:     ; BasePtrInst: Load of base pointer
; CHECK-OP-NEXT:  %tmp4 = load ptr, ptr %fElemList
; CHECK-OP-MOD:   %tmp4 = load ptr, ptr %fElemList
  %tmp4 = load %class.DatatypeValidator**, %class.DatatypeValidator*** %fElemList
  %idxprom = zext i32 %getAt to i64
; CHECK:     ; MemInstGEP: Address in ret
; CHECK-NEXT:  %arrayidx = getelementptr inbounds %class.DatatypeValidator*, %class.DatatypeValidator** %tmp4, i64 %idxprom
; CHECK-MOD:   %arrayidx = getelementptr inbounds %__SOA_EL_class.ValueVectorOf, %__SOA_EL_class.ValueVectorOf* %tmp4, i64 %idxprom
; CHECK-OP:     ; MemInstGEP: Address in ret
; CHECK-OP-NEXT:  %arrayidx = getelementptr inbounds ptr, ptr %tmp4, i64 %idxprom
; CHECK-OP-MOD:   %arrayidx = getelementptr inbounds %__SOA_EL_class.ValueVectorOf, ptr %tmp4, i64 %idxprom
  %arrayidx = getelementptr inbounds %class.DatatypeValidator*, %class.DatatypeValidator** %tmp4, i64 %idxprom
; CHECK-MOD-NEXT:  %elem = getelementptr inbounds %__SOA_EL_class.ValueVectorOf, %__SOA_EL_class.ValueVectorOf* %arrayidx, i64 0, i32 1
; CHECK-MOD-NEXT:  ret %class.DatatypeValidator** %elem
; CHECK:     ; MemInst: Address in ret
; CHECK-NEXT:  ret %class.DatatypeValidator** %arrayidx
; CHECK-OP-MOD-NEXT:  %elem = getelementptr inbounds %__SOA_EL_class.ValueVectorOf, ptr %arrayidx, i64 0, i32 1
; CHECK-OP-MOD-NEXT:  ret ptr %elem
; CHECK-OP:     ; MemInst: Address in ret
; CHECK-OP-NEXT:  ret ptr %arrayidx
  ret %class.DatatypeValidator** %arrayidx
}

declare i32 @__gxx_personality_v0(...)

declare !intel.dtrans.func.type !13 noalias nonnull "intel_dtrans_func_index"="1" i8* @__cxa_allocate_exception(i64)

declare !intel.dtrans.func.type !15 void @__cxa_throw(i8* "intel_dtrans_func_index"="1", i8* "intel_dtrans_func_index"="2", i8* "intel_dtrans_func_index"="3")

declare !intel.dtrans.func.type !16 void @__cxa_free_exception(i8* "intel_dtrans_func_index"="1")

declare !intel.dtrans.func.type !17 void @"XMLException::~XMLException()"(%class.XMLException* "intel_dtrans_func_index"="1")

declare !intel.dtrans.func.type !19 void @"ArrayIndexOutOfBoundsException::ArrayIndexOutOfBoundsException(char const*, unsigned int, XMLExcepts::Codes, MemoryManager*)"(%class.XMLPlatformUtilsException* "intel_dtrans_func_index"="1", i8* "intel_dtrans_func_index"="2", i32, i32, %class.XMLMsgLoader* "intel_dtrans_func_index"="3")

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
