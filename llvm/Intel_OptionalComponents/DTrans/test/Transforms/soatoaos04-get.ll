; RUN: opt < %s -whole-program-assume \
; RUN:                -disable-output \
; RUN: -passes='require<dtransanalysis>,function(require<soatoaos-approx>,require<soatoaos-array-methods>)' \
; RUN:        -dtrans-soatoaos-base-ptr-off=3 -dtrans-soatoaos-mem-off=4  \
; RUN:        -debug-only=dtrans-soatoaos,dtrans-soatoaos-arrays          \
; RUN:        -dtrans-malloc-functions=class.XMLMsgLoader,2  \
; RUN:        -dtrans-free-functions=class.XMLMsgLoader,3    \
; RUN:        2>&1 | FileCheck %s
; REQUIRES: asserts
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%class.XMLPlatformUtilsException = type opaque
%class.XMLMsgLoader = type { i32 (...)** }
%class.DatatypeValidator = type opaque
%class.ValueVectorOf = type { i8, i32, i32, %class.DatatypeValidator**, %class.XMLMsgLoader* }
%class.XMLException = type { i32 (...)**, i32, i8*, i32, i16*, %class.XMLMsgLoader* }

@"typeinfo for ArrayIndexOutOfBoundsException" = external constant { i8*, i8*, i8* }
@.str.1.2092 = external constant [31 x i8]

declare i32 @__gxx_personality_v0(...)

declare noalias nonnull i8* @__cxa_allocate_exception(i64)

declare void @__cxa_throw(i8*, i8*, i8*)

declare void @__cxa_free_exception(i8*)

declare void @"ArrayIndexOutOfBoundsException::ArrayIndexOutOfBoundsException(char const*, unsigned int, XMLExcepts::Codes, MemoryManager*)"(%class.XMLPlatformUtilsException*, i8*, i32, i32, %class.XMLMsgLoader*)

; The following method should be classified as get-like method.
; Instructions to transform are shown.
;  template <class TElem>
;  TElem &ValueVectorOf<TElem>::elementAt(const unsigned int getAt) {
;    if (getAt >= fCurCount)
;      ThrowXMLwithMemMgr(ArrayIndexOutOfBoundsException,
;                         XMLExcepts::Vector_BadIndex, fMemoryManager);
;    return fElemList[getAt];
;  }
; CHECK:; Classification: Get pointer to element method
; CHECK:; Dump instructions needing update. Total = 2
define %class.DatatypeValidator** @"ValueVectorOf<DatatypeValidator*>::elementAt(unsigned int)"(%class.ValueVectorOf* %this, i32 %getAt) personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
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
  tail call void @__cxa_throw(i8* nonnull %exception, i8* bitcast ({ i8*, i8*, i8* }* @"typeinfo for ArrayIndexOutOfBoundsException" to i8*), i8* bitcast (void (%class.XMLException*)* @"XMLException::~XMLException()" to i8*))
  unreachable

lpad:                                             ; preds = %if.then
  %tmp3 = landingpad { i8*, i32 }
          cleanup
  tail call void @__cxa_free_exception(i8* nonnull %exception)
  resume { i8*, i32 } %tmp3

if.end:                                           ; preds = %entry
  %fElemList = getelementptr inbounds %class.ValueVectorOf, %class.ValueVectorOf* %this, i64 0, i32 3
; CHECK:     ; BasePtrInst: Load of base pointer
; CHECK-NEXT:  %tmp4 = load %class.DatatypeValidator**, %class.DatatypeValidator*** %fElemList
  %tmp4 = load %class.DatatypeValidator**, %class.DatatypeValidator*** %fElemList
  %idxprom = zext i32 %getAt to i64
; CHECK:     ; MemInstGEP: Address in ret
; CHECK-NEXT:  %arrayidx = getelementptr inbounds %class.DatatypeValidator*, %class.DatatypeValidator** %tmp4, i64 %idxprom
  %arrayidx = getelementptr inbounds %class.DatatypeValidator*, %class.DatatypeValidator** %tmp4, i64 %idxprom
  ret %class.DatatypeValidator** %arrayidx
}

declare void @"XMLException::~XMLException()"(%class.XMLException*)
