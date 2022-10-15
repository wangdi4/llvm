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
%class.ValueVectorOf = type { i8, i32, i32, %class.DatatypeValidator**, %class.XMLMsgLoader* }
; CHECK-MOD-DAG: %__SOA_class.ValueVectorOf = type { i8, i32, i32, %__SOA_EL_class.ValueVectorOf*, %class.XMLMsgLoader* }
; CHECK-MOD-DAG: %__SOA_EL_class.ValueVectorOf = type { float*, %class.DatatypeValidator* }
; CHECK-OP-MOD-DAG: %__SOA_class.ValueVectorOf = type { i8, i32, i32, ptr, ptr }
; CHECK-OP-MOD-DAG: %__SOA_EL_class.ValueVectorOf = type { ptr, ptr }
%class.DatatypeValidator = type opaque
%class.XMLException = type opaque

@.str.1.2092 = external constant [31 x i8]

; The following method should be classified as set-like method.
; Instructions to transform are shown.
; Transformed instructions are shown.
;  template <class TElem>
;  void ValueVectorOf<TElem>::setElementAt(const TElem &toSet,
;                                          const unsigned int setAt) {
;    if (setAt >= fCurCount)
;      ThrowXMLwithMemMgr(ArrayIndexOutOfBoundsException,
;                         XMLExcepts::Vector_BadIndex, fMemoryManager);
;    fElemList[setAt] = toSet;
;  }
; CHECK:; Classification: Set element method
; CHECK:; Dump instructions needing update. Total = 4
; CHECK-OP:; Classification: Set element method
; CHECK-OP:; Dump instructions needing update. Total = 4
define void @"ValueVectorOf<DatatypeValidator*>::setElementAt(DatatypeValidator* const&, unsigned int)"(%class.ValueVectorOf* "intel_dtrans_func_index"="1" %this, %class.DatatypeValidator** "intel_dtrans_func_index"="2" %toSet, i32 %setAt) personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) !intel.dtrans.func.type !11 {
entry:
  %fCurCount = getelementptr inbounds %class.ValueVectorOf, %class.ValueVectorOf* %this, i64 0, i32 1
  %tmp = load i32, i32* %fCurCount
  %cmp = icmp ugt i32 %tmp, %setAt
  br i1 %cmp, label %if.end, label %if.then

if.then:                                          ; preds = %entry
  %exception = tail call i8* @__cxa_allocate_exception(i64 48)
  %tmp1 = bitcast i8* %exception to %class.XMLPlatformUtilsException*
; CHECK-MOD:  %fMemoryManager = getelementptr inbounds %__SOA_class.ValueVectorOf, %__SOA_class.ValueVectorOf* %this, i64 0, i32 4
; CHECK-OP-MOD:  %fMemoryManager = getelementptr inbounds %__SOA_class.ValueVectorOf, ptr %this, i64 0, i32 4
  %fMemoryManager = getelementptr inbounds %class.ValueVectorOf, %class.ValueVectorOf* %this, i64 0, i32 4
  %tmp2 = load %class.XMLMsgLoader*, %class.XMLMsgLoader** %fMemoryManager
  invoke void @"ArrayIndexOutOfBoundsException::ArrayIndexOutOfBoundsException(char const*, unsigned int, XMLExcepts::Codes, MemoryManager*)"(%class.XMLPlatformUtilsException* nonnull %tmp1, i8* getelementptr inbounds ([31 x i8], [31 x i8]* @.str.1.2092, i64 0, i64 0), i32 126, i32 116, %class.XMLMsgLoader* %tmp2)
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
  %tmp4 = bitcast %class.DatatypeValidator** %toSet to i64*
; CHECK:     ; Arg: Load from arg
; CHECK-NEXT:  %tmp5 = load i64, i64* %tmp4
; CHECK:     ; BasePtrInst: Load of base pointer
; CHECK-NEXT:  %tmp6 = load %class.DatatypeValidator**, %class.DatatypeValidator*** %fElemList
; CHECK-OP:     ; Arg: Load from arg
; CHECK-OP-NEXT:  %tmp5 = load i64, ptr %tmp4
; CHECK-OP:     ; BasePtrInst: Load of base pointer
; CHECK-OP-NEXT:  %tmp6 = load ptr, ptr %fElemList
  %tmp5 = load i64, i64* %tmp4
; CHECK-MOD: %fElemList = getelementptr inbounds %__SOA_class.ValueVectorOf, %__SOA_class.ValueVectorOf* %this, i64 0, i32 3
; CHECK-OP-MOD: %fElemList = getelementptr inbounds %__SOA_class.ValueVectorOf, ptr %this, i64 0, i32 3
  %fElemList = getelementptr inbounds %class.ValueVectorOf, %class.ValueVectorOf* %this, i64 0, i32 3
; CHECK-MOD:   %tmp6 = load %__SOA_EL_class.ValueVectorOf*, %__SOA_EL_class.ValueVectorOf** %fElemList
; CHECK-OP-MOD:   %tmp6 = load ptr, ptr %fElemList
  %tmp6 = load %class.DatatypeValidator**, %class.DatatypeValidator*** %fElemList
  %idxprom = zext i32 %setAt to i64
; CHECK:     ; MemInstGEP: Element set from arg
; CHECK-NEXT:  %arrayidx = getelementptr inbounds %class.DatatypeValidator*, %class.DatatypeValidator** %tmp6, i64 %idxprom
; CHECK-OP:     ; MemInstGEP: Element set from arg
; CHECK-OP-NEXT:  %arrayidx = getelementptr inbounds ptr, ptr %tmp6, i64 %idxprom
; CHECK-MOD:   %arrayidx = getelementptr inbounds %__SOA_EL_class.ValueVectorOf, %__SOA_EL_class.ValueVectorOf* %tmp6, i64 %idxprom
; CHECK-OP-MOD:   %arrayidx = getelementptr inbounds %__SOA_EL_class.ValueVectorOf, ptr %tmp6, i64 %idxprom
  %arrayidx = getelementptr inbounds %class.DatatypeValidator*, %class.DatatypeValidator** %tmp6, i64 %idxprom
; CHECK-MOD-NEXT:  %elem = getelementptr inbounds %__SOA_EL_class.ValueVectorOf, %__SOA_EL_class.ValueVectorOf* %arrayidx, i64 0, i32 1
; CHECK-MOD-NEXT:  %tmp7 = bitcast %class.DatatypeValidator** %elem to i64*
; CHECK-OP-MOD-NEXT:  %elem = getelementptr inbounds %__SOA_EL_class.ValueVectorOf, ptr %arrayidx, i64 0, i32 1
; CHECK-OP-MOD-NEXT:  %tmp7 = bitcast ptr %elem to ptr
  %tmp7 = bitcast %class.DatatypeValidator** %arrayidx to i64*
; CHECK:     ; MemInst: Element set from arg
; CHECK-NEXT:  store i64 %tmp5, i64* %tmp7
; CHECK-OP:     ; MemInst: Element set from arg
; CHECK-OP-NEXT:  store i64 %tmp5, ptr %tmp7
  store i64 %tmp5, i64* %tmp7
  ret void
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
!11 = distinct !{!12, !7}
!12 = !{%class.ValueVectorOf zeroinitializer, i32 1}
!13 = distinct !{!14}
!14 = !{i8 0, i32 1}
!15 = distinct !{!14, !14, !14}
!16 = distinct !{!14}
!17 = distinct !{!18}
!18 = !{%class.XMLException zeroinitializer, i32 1}
!19 = distinct !{!20, !14, !8}
!20 = !{%class.XMLPlatformUtilsException zeroinitializer, i32 1}
