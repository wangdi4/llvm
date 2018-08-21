; RUN: opt < %s -whole-program-assume -disable-output                                                       \
; RUN: -passes='require<dtransanalysis>,function(require<soatoaos-approx>,require<soatoaos-array-methods>)' \
; RUN:        -dtrans-soatoaos-base-ptr-off=3 -dtrans-soatoaos-mem-off=4                                    \
; RUN:        -debug-only=dtrans-soatoaos,dtrans-soatoaos-arrays                                            \
; RUN:        -dtrans-malloc-functions=class.XMLMsgLoader,2                                                 \
; RUN:        -dtrans-free-functions=class.XMLMsgLoader,3                                                   \
; RUN:        2>&1 | FileCheck %s
; RUN: opt -S < %s -whole-program-assume                                                                    \
; RUN:        -passes=soatoaos-arrays-methods-transform                                                     \
; RUN:        -dtrans-soatoaos-base-ptr-off=3 -dtrans-soatoaos-mem-off=4                                    \
; RUN:        -dtrans-malloc-functions=class.XMLMsgLoader,2                                                 \
; RUN:        -dtrans-free-functions=class.XMLMsgLoader,3                                                   \
; RUN:        | FileCheck --check-prefix=CHECK-MOD %s
; REQUIRES: asserts
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%class.XMLPlatformUtilsException = type opaque
%class.XMLMsgLoader = type { i32 (...)** }
%class.ValueVectorOf = type { i8, i32, i32, %class.DatatypeValidator**, %class.XMLMsgLoader* }
; CHECK-MOD: %__SOA_class.ValueVectorOf = type { i8, i32, i32, %__SOA_EL_class.ValueVectorOf*, %class.XMLMsgLoader* }
; CHECK-MOD: %__SOA_EL_class.ValueVectorOf = type { float*, %class.DatatypeValidator* }
%class.DatatypeValidator = type opaque
%class.XMLException = type opaque

@"typeinfo for ArrayIndexOutOfBoundsException" = external constant { i8*, i8*, i8* }
@.str.1.2092 = external constant [31 x i8]

declare i32 @__gxx_personality_v0(...)

declare noalias nonnull i8* @__cxa_allocate_exception(i64)

declare void @__cxa_throw(i8*, i8*, i8*)

declare void @__cxa_free_exception(i8*)

declare void @"ArrayIndexOutOfBoundsException::ArrayIndexOutOfBoundsException(char const*, unsigned int, XMLExcepts::Codes, MemoryManager*)"(%class.XMLPlatformUtilsException*, i8*, i32, i32, %class.XMLMsgLoader*)

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
define void @"ValueVectorOf<DatatypeValidator*>::setElementAt(DatatypeValidator* const&, unsigned int)"(%class.ValueVectorOf* %this, %class.DatatypeValidator** %toSet, i32 %setAt) personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
entry:
  %fCurCount = getelementptr inbounds %class.ValueVectorOf, %class.ValueVectorOf* %this, i64 0, i32 1
  %tmp = load i32, i32* %fCurCount
  %cmp = icmp ugt i32 %tmp, %setAt
  br i1 %cmp, label %if.end, label %if.then

if.then:                                          ; preds = %entry
  %exception = tail call i8* @__cxa_allocate_exception(i64 48)
  %tmp1 = bitcast i8* %exception to %class.XMLPlatformUtilsException*
; CHECK-MOD:  %fMemoryManager = getelementptr inbounds %__SOA_class.ValueVectorOf, %__SOA_class.ValueVectorOf* %this, i64 0, i32 4
  %fMemoryManager = getelementptr inbounds %class.ValueVectorOf, %class.ValueVectorOf* %this, i64 0, i32 4
  %tmp2 = load %class.XMLMsgLoader*, %class.XMLMsgLoader** %fMemoryManager
  invoke void @"ArrayIndexOutOfBoundsException::ArrayIndexOutOfBoundsException(char const*, unsigned int, XMLExcepts::Codes, MemoryManager*)"(%class.XMLPlatformUtilsException* nonnull %tmp1, i8* getelementptr inbounds ([31 x i8], [31 x i8]* @.str.1.2092, i64 0, i64 0), i32 126, i32 116, %class.XMLMsgLoader* %tmp2)
          to label %invoke.cont unwind label %lpad

invoke.cont:                                      ; preds = %if.then
  tail call void @__cxa_throw(i8* nonnull %exception, i8* bitcast ({ i8*, i8*, i8* }* @"typeinfo for ArrayIndexOutOfBoundsException" to i8*), i8* bitcast (void (%class.XMLException*)* @"XMLException::~XMLException()" to i8*))
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
  %tmp5 = load i64, i64* %tmp4
; CHECK-MOD: %fElemList = getelementptr inbounds %__SOA_class.ValueVectorOf, %__SOA_class.ValueVectorOf* %this, i64 0, i32 3
  %fElemList = getelementptr inbounds %class.ValueVectorOf, %class.ValueVectorOf* %this, i64 0, i32 3
; CHECK-MOD:   %tmp6 = load %__SOA_EL_class.ValueVectorOf*, %__SOA_EL_class.ValueVectorOf** %fElemList
  %tmp6 = load %class.DatatypeValidator**, %class.DatatypeValidator*** %fElemList
  %idxprom = zext i32 %setAt to i64
; CHECK:     ; MemInstGEP: Element set from arg
; CHECK-NEXT:  %arrayidx = getelementptr inbounds %class.DatatypeValidator*, %class.DatatypeValidator** %tmp6, i64 %idxprom
; CHECK-MOD:   %arrayidx = getelementptr inbounds %__SOA_EL_class.ValueVectorOf, %__SOA_EL_class.ValueVectorOf* %tmp6, i64 %idxprom
  %arrayidx = getelementptr inbounds %class.DatatypeValidator*, %class.DatatypeValidator** %tmp6, i64 %idxprom
; CHECK-MOD-NEXT:  %elem = getelementptr inbounds %__SOA_EL_class.ValueVectorOf, %__SOA_EL_class.ValueVectorOf* %arrayidx, i64 0, i32 1
; CHECK-MOD-NEXT:  %tmp7 = bitcast %class.DatatypeValidator** %elem to i64*
  %tmp7 = bitcast %class.DatatypeValidator** %arrayidx to i64*
; CHECK:     ; MemInst: Element set from arg
; CHECK-NEXT:  store i64 %tmp5, i64* %tmp7
  store i64 %tmp5, i64* %tmp7
  ret void
}

declare void @"XMLException::~XMLException()"(%class.XMLException*)
