; RUN: opt < %s -whole-program-assume -disable-output -debug-only=dtrans-soatoaos-deps          \
; RUN:          -passes='require<dtransanalysis>,function(require<soatoaos-approx>)'            \
; RUN:          -dtrans-malloc-functions=class.XMLMsgLoader,2                                   \
; RUN:          -dtrans-malloc-functions="XMemory::operator new(unsigned long_ MemoryManager*)" \
; RUN:          -dtrans-free-functions=class.XMLMsgLoader,3                                     \
; RUN:          -dtrans-free-functions="XMemory::operator delete(void*_ MemoryManager*)"        \
; RUN:          -dtrans-free-functions="XMemory::operator delete(void*)"                        \
; RUN:       2>&1 | FileCheck --check-prefix=CHECK-DEP %s
; RUN: opt < %s -whole-program-assume -disable-output -debug-only=dtrans-soatoaos               \
; RUN:          -passes='require<dtransanalysis>,function(require<soatoaos-approx>,require<soatoaos-struct-methods>)' \
; RUN:          -dtrans-soatoaos-mem-off=3                                                      \
; RUN:          -dtrans-soatoaos-array-type=class.ValueVectorOf.0                               \
; RUN:          -dtrans-soatoaos-array-type=class.ValueVectorOf.1                               \
; RUN:          -dtrans-malloc-functions=class.XMLMsgLoader,2                                   \
; RUN:          -dtrans-malloc-functions="XMemory::operator new(unsigned long_ MemoryManager*)" \
; RUN:          -dtrans-free-functions=class.XMLMsgLoader,3                                     \
; RUN:          -dtrans-free-functions="XMemory::operator delete(void*_ MemoryManager*)"        \
; RUN:          -dtrans-free-functions="XMemory::operator delete(void*)"                        \
; RUN:       2>&1 | FileCheck --check-prefix=CHECK-IR %s
; RUN: opt < %s -whole-program-assume -disable-output -debug-only=dtrans-soatoaos-struct        \
; RUN:          -passes='require<dtransanalysis>,function(require<soatoaos-approx>,require<soatoaos-struct-methods>)' \
; RUN:          -dtrans-soatoaos-mem-off=3                                                      \
; RUN:          -dtrans-soatoaos-array-type=class.ValueVectorOf.0                               \
; RUN:          -dtrans-soatoaos-array-type=class.ValueVectorOf.1                               \
; RUN:          -dtrans-malloc-functions=class.XMLMsgLoader,2                                   \
; RUN:          -dtrans-malloc-functions="XMemory::operator new(unsigned long_ MemoryManager*)" \
; RUN:          -dtrans-free-functions=class.XMLMsgLoader,3                                     \
; RUN:          -dtrans-free-functions="XMemory::operator delete(void*_ MemoryManager*)"        \
; RUN:          -dtrans-free-functions="XMemory::operator delete(void*)"                        \
; RUN:       2>&1 | FileCheck --check-prefix=CHECK-TRANS %s
; REQUIRES: asserts

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Check that approximations work as expected.
; Checks special cases in approximation computations (see inlined checks).
; CHECK-DEP-NOT: ; {{.*}}Unknown{{.*}}Dep
; There should be no unknown GEP
; CHECK-DEP-NOT: ; Func(GEP

; Checks that all instructions can be dealt with.
; CHECK-IR: ; Checking structure's method FieldValueMap::getValueAt(unsigned int) const
; CHECK-IR: ; IR: analysed completely

; Checks instructions related to transformations:
; No references to arrays of interest.
; CHECK-TRANS: ; Dump instructions needing update. Total = 0

; inline XMLCh *FieldValueMap::getValueAt(const unsigned int index) const {
;   if (fValues) {
;     return fValues->elementAt(index);
;   }
;   return 0;
; }

%class.XMLPlatformUtilsException = type opaque
%class.XMLMsgLoader = type { i32 (...)** }
%class.FieldValueMap = type { %class.ValueVectorOf.0*, %class.ValueVectorOf.1*, %class.RefArrayVectorOf*, %class.XMLMsgLoader* }
%class.ValueVectorOf.0 = type { i8, i32, i32, %class.IC_Field**, %class.XMLMsgLoader* }
%class.IC_Field = type opaque
%class.ValueVectorOf.1 = type { i8, i32, i32, %class.DatatypeValidator**, %class.XMLMsgLoader* }
%class.DatatypeValidator = type opaque
%class.RefArrayVectorOf = type { %class.BaseRefVectorOf }
%class.BaseRefVectorOf = type { i32 (...)**, i8, i32, i32, i16**, %class.XMLMsgLoader* }
%class.XMLException = type opaque

@"typeinfo for ArrayIndexOutOfBoundsException" = external hidden constant { i8*, i8*, i8* }
@.str.2.2091 = external hidden constant [33 x i8]

declare i32 @__gxx_personality_v0(...)

declare noalias i8* @__cxa_allocate_exception(i64)

declare void @__cxa_throw(i8*, i8*, i8*)

declare void @__cxa_free_exception(i8*)

declare hidden void @"ArrayIndexOutOfBoundsException::ArrayIndexOutOfBoundsException(char const*, unsigned int, XMLExcepts::Codes, MemoryManager*)"(%class.XMLPlatformUtilsException*, i8*, i32, i32, %class.XMLMsgLoader*)

define hidden i16* @"FieldValueMap::getValueAt(unsigned int) const"(%class.FieldValueMap* %this, i32 %index) personality i32 (...)* @__gxx_personality_v0 {
entry:
  %fValues = getelementptr inbounds %class.FieldValueMap, %class.FieldValueMap* %this, i64 0, i32 2
  %tmp = load %class.RefArrayVectorOf*, %class.RefArrayVectorOf** %fValues
  %tobool = icmp eq %class.RefArrayVectorOf* %tmp, null
  br i1 %tobool, label %return, label %if.then

if.then:                                          ; preds = %entry
  %fCurCount.i = getelementptr inbounds %class.RefArrayVectorOf, %class.RefArrayVectorOf* %tmp, i64 0, i32 0, i32 2
  %tmp1 = load i32, i32* %fCurCount.i
  %cmp.i = icmp ugt i32 %tmp1, %index
  br i1 %cmp.i, label %"BaseRefVectorOf<unsigned short>::elementAt(unsigned int) [clone .exit]", label %if.then.i

if.then.i:                                        ; preds = %if.then
  %exception.i = tail call i8* @__cxa_allocate_exception(i64 48)
  %tmp2 = bitcast i8* %exception.i to %class.XMLPlatformUtilsException*
  %fMemoryManager.i = getelementptr inbounds %class.RefArrayVectorOf, %class.RefArrayVectorOf* %tmp, i64 0, i32 0, i32 5
  %tmp3 = load %class.XMLMsgLoader*, %class.XMLMsgLoader** %fMemoryManager.i
  invoke void @"ArrayIndexOutOfBoundsException::ArrayIndexOutOfBoundsException(char const*, unsigned int, XMLExcepts::Codes, MemoryManager*)"(%class.XMLPlatformUtilsException* %tmp2, i8* getelementptr inbounds ([33 x i8], [33 x i8]* @.str.2.2091, i64 0, i64 0), i32 249, i32 116, %class.XMLMsgLoader* %tmp3)
          to label %invoke.cont.i unwind label %lpad.i

invoke.cont.i:                                    ; preds = %if.then.i
  tail call void @__cxa_throw(i8* %exception.i, i8* bitcast ({ i8*, i8*, i8* }* @"typeinfo for ArrayIndexOutOfBoundsException" to i8*), i8* bitcast (void (%class.XMLException*)* @"XMLException::~XMLException()" to i8*))
  unreachable

lpad.i:                                           ; preds = %if.then.i
  %tmp4 = landingpad { i8*, i32 }
          cleanup
  tail call void @__cxa_free_exception(i8* %exception.i)
  resume { i8*, i32 } %tmp4

"BaseRefVectorOf<unsigned short>::elementAt(unsigned int) [clone .exit]": ; preds = %if.then
  %fElemList.i = getelementptr inbounds %class.RefArrayVectorOf, %class.RefArrayVectorOf* %tmp, i64 0, i32 0, i32 4
  %tmp5 = load i16**, i16*** %fElemList.i
  %idxprom.i = zext i32 %index to i64
  %arrayidx.i = getelementptr inbounds i16*, i16** %tmp5, i64 %idxprom.i
  %tmp6 = load i16*, i16** %arrayidx.i
  br label %return

return:                                           ; preds = %"BaseRefVectorOf<unsigned short>::elementAt(unsigned int) [clone .exit]", %entry
  %retval.0 = phi i16* [ %tmp6, %"BaseRefVectorOf<unsigned short>::elementAt(unsigned int) [clone .exit]" ], [ null, %entry ]
  ret i16* %retval.0
}

declare hidden void @"XMLException::~XMLException()"(%class.XMLException*)

; CHECK-DEP: Deps computed: 16, Queries: 36
