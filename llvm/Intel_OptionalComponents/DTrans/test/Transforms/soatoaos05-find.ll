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
; CHECK-IR: ; Checking structure's method FieldValueMap::indexOf(IC_Field const*) const
; CHECK-IR: ; IR: analysed completely

; Checks instructions related to transformations.
; CHECK-TRANS: ; Dump instructions needing update. Total = 5

; int FieldValueMap::indexOf(const IC_Field *const key) const {
;   if (fFields) {
;     unsigned int fieldSize = fFields->size();
;     for (unsigned int i = 0; i < fieldSize; i++) {
;       if (fFields->elementAt(i) == key) {
;         return i;
;       }
;     }
;   }
;   return -1;
; }

%class.FieldValueMap = type { %class.ValueVectorOf.0*, %class.ValueVectorOf.1*, %class.RefArrayVectorOf*, %class.XMLMsgLoader* }
%class.ValueVectorOf.0 = type { i8, i32, i32, %class.IC_Field**, %class.XMLMsgLoader* }
%class.ValueVectorOf.1 = type { i8, i32, i32, %class.DatatypeValidator**, %class.XMLMsgLoader* }
%class.DatatypeValidator = type opaque
%class.RefArrayVectorOf = type { %class.BaseRefVectorOf }
%class.BaseRefVectorOf = type { i32 (...)**, i8, i32, i32, i16**, %class.XMLMsgLoader* }
%class.XMLMsgLoader = type { i32 (...)** }
%class.IC_Field = type opaque

define hidden i32 @"FieldValueMap::indexOf(IC_Field const*) const"(%class.FieldValueMap* %this, %class.IC_Field* %key) {
entry:
  %fFields = getelementptr inbounds %class.FieldValueMap, %class.FieldValueMap* %this, i64 0, i32 0
; CHECK-TRANS:      ; ArrayInst: Load of array
; CHECK-TRANS-NEXT:   %tmp = load %class.ValueVectorOf.0*, %class.ValueVectorOf.0** %fFields
  %tmp = load %class.ValueVectorOf.0*, %class.ValueVectorOf.0** %fFields
; CHECK-TRANS:      ; ArrayInst: Null check
; CHECK-TRANS-NEXT:   %tobool = icmp eq %class.ValueVectorOf.0* %tmp, null
  %tobool = icmp eq %class.ValueVectorOf.0* %tmp, null
  br i1 %tobool, label %return, label %if.then

if.then:                                          ; preds = %entry
; CHECK-TRANS:      ; ArrayInst: Array method call
; CHECK-TRANS-NEXT:   %call = tail call i32 @"ValueVectorOf<IC_Field*>::size() const"(%class.ValueVectorOf.0* %tmp)
  %call = tail call i32 @"ValueVectorOf<IC_Field*>::size() const"(%class.ValueVectorOf.0* %tmp)
  %cmp17 = icmp eq i32 %call, 0
  br i1 %cmp17, label %return, label %for.body

for.body:                                         ; preds = %for.inc, %if.then
  %i.018 = phi i32 [ %inc, %for.inc ], [ 0, %if.then ]
; CHECK-TRANS:      ; ArrayInst: Load of array
; CHECK-TRANS-NEXT:   %tmp1 = load %class.ValueVectorOf.0*, %class.ValueVectorOf.0** %fFields
  %tmp1 = load %class.ValueVectorOf.0*, %class.ValueVectorOf.0** %fFields
; CHECK-TRANS:      ; ArrayInst: Array method call
; CHECK-TRANS-NEXT:   %call4 = tail call %class.IC_Field** @"ValueVectorOf<IC_Field*>::elementAt(unsigned int)"(%class.ValueVectorOf.0* %tmp1, i32 %i.018)
  %call4 = tail call %class.IC_Field** @"ValueVectorOf<IC_Field*>::elementAt(unsigned int)"(%class.ValueVectorOf.0* %tmp1, i32 %i.018)
  %tmp2 = load %class.IC_Field*, %class.IC_Field** %call4
  %cmp5 = icmp eq %class.IC_Field* %tmp2, %key
  br i1 %cmp5, label %return, label %for.inc

for.inc:                                          ; preds = %for.body
  %inc = add nuw i32 %i.018, 1
  %cmp = icmp ult i32 %inc, %call
  br i1 %cmp, label %for.body, label %return

return:                                           ; preds = %for.inc, %for.body, %if.then, %entry
  %retval.1 = phi i32 [ -1, %entry ], [ -1, %if.then ], [ -1, %for.inc ], [ %i.018, %for.body ]
  ret i32 %retval.1
}

declare hidden i32 @"ValueVectorOf<IC_Field*>::size() const"(%class.ValueVectorOf.0*)

declare hidden %class.IC_Field** @"ValueVectorOf<IC_Field*>::elementAt(unsigned int)"(%class.ValueVectorOf.0*, i32)

; CHECK-DEP: Deps computed: 10, Queries: 19

