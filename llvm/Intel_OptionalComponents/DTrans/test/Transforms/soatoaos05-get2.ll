; RUN: opt < %s -whole-program-assume -intel-libirc-allowed -disable-output -debug-only=dtrans-soatoaos-deps          \
; RUN:          -passes='require<dtransanalysis>,require<soatoaos-approx>'                      \
; RUN:       2>&1 | FileCheck --check-prefix=CHECK-DEP %s
; RUN: opt < %s -whole-program-assume -intel-libirc-allowed -disable-output                                           \
; RUN:          -debug-only=dtrans-soatoaos,dtrans-soatoaos-struct                              \
; RUN:          -passes='require<dtransanalysis>,require<soatoaos-approx>,require<soatoaos-struct-methods>' \
; RUN:          -dtrans-soatoaos-mem-off=3                                                      \
; RUN:          -dtrans-soatoaos-array-type=class.ValueVectorOf.0                               \
; RUN:          -dtrans-soatoaos-array-type=class.ValueVectorOf.1                               \
; RUN:          -dtrans-soatoaos-base-ptr-off=3                                                 \
; RUN:       2>&1 | FileCheck --check-prefix=CHECK-TRANS %s
; RUN: opt -S < %s -whole-program-assume -intel-libirc-allowed                                                        \
; RUN:          -passes=soatoaos-struct-methods-transform                                       \
; RUN:          -dtrans-soatoaos-mem-off=3                                                      \
; RUN:          -dtrans-optbase-process-function-declaration                                    \
; RUN:          -dtrans-soatoaos-array-type=class.ValueVectorOf.0                               \
; RUN:          -dtrans-soatoaos-array-type=class.ValueVectorOf.1                               \
; RUN:          -dtrans-soatoaos-base-ptr-off=3                                                 \
; RUN:       | FileCheck --check-prefix=CHECK-MOD %s
; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Check that approximations work as expected.
; Checks special cases in approximation computations (see inlined checks).
; CHECK-DEP-NOT: ; {{.*}}Unknown{{.*}}Dep
; There should be no unknown GEP
; CHECK-DEP-NOT: ; Func(GEP

; Checks that all instructions can be dealt with.
; CHECK-TRANS: ; Checking structure's method FieldValueMap::getDatatypeValidatorAt(unsigned int) const
; CHECK-TRANS: ; IR: has only expected side-effects

; Checks instructions related to transformations:
; CHECK-TRANS: ; Dump instructions needing update. Total = 2

; Checks transformation. Access to second pointer to array is adjusted:
; first pointer to array is accessed.

; inline DatatypeValidator *
; FieldValueMap::getDatatypeValidatorAt(const unsigned int index) const {
;   if (fValidators) {
;     return fValidators->elementAt(index);
;   }
;   return 0;
; }

%class.FieldValueMap = type { %class.ValueVectorOf.0*, %class.ValueVectorOf.1*, %class.RefArrayVectorOf*, %class.XMLMsgLoader* }
%class.ValueVectorOf.0 = type { i8, i32, i32, %class.IC_Field**, %class.XMLMsgLoader* }
; CHECK-MOD-DAG: %__SOA_class.FieldValueMap = type { %__SOA_AR_class.ValueVectorOf.0*, float*, %class.RefArrayVectorOf*, %class.XMLMsgLoader* }
; CHECK-MOD-DAG: %__SOA_AR_class.ValueVectorOf.0 = type { i8, i32, i32, %__SOA_EL_class.FieldValueMap*, %class.XMLMsgLoader* }
; CHECK-MOD-DAG: %__SOA_EL_class.FieldValueMap = type { %class.IC_Field*, %class.DatatypeValidator* }
; CHECK-MOD-NOT: ValueVectorOf.1
%class.ValueVectorOf.1 = type { i8, i32, i32, %class.DatatypeValidator**, %class.XMLMsgLoader* }
%class.DatatypeValidator = type opaque
%class.IC_Field = type opaque
%class.RefArrayVectorOf = type { %class.BaseRefVectorOf }
%class.BaseRefVectorOf = type { i32 (...)**, i8, i32, i32, i16**, %class.XMLMsgLoader* }
%class.XMLMsgLoader = type { i32 (...)** }

; CHECK-MOD: @"FieldValueMap::getDatatypeValidatorAt(unsigned int) const.1"(%__SOA_class.FieldValueMap* %this, i32 %index)
define hidden %class.DatatypeValidator* @"FieldValueMap::getDatatypeValidatorAt(unsigned int) const"(%class.FieldValueMap* %this, i32 %index) {
entry:
; CHECK-MOD:          %fValidators = getelementptr inbounds %__SOA_class.FieldValueMap, %__SOA_class.FieldValueMap* %this, i64 0, i32 0
  %fValidators = getelementptr inbounds %class.FieldValueMap, %class.FieldValueMap* %this, i64 0, i32 1
; CHECK-TRANS:      ; ArrayInst: Load of array
; CHECK-TRANS-NEXT:   %tmp = load %class.ValueVectorOf.1*, %class.ValueVectorOf.1** %fValidators
; CHECK-MOD-NEXT:     %tmp = load %__SOA_AR_class.ValueVectorOf.0*, %__SOA_AR_class.ValueVectorOf.0** %fValidators
  %tmp = load %class.ValueVectorOf.1*, %class.ValueVectorOf.1** %fValidators
; CHECK-MOD-NEXT:     %tobool = icmp eq %__SOA_AR_class.ValueVectorOf.0* %tmp, null
  %tobool = icmp eq %class.ValueVectorOf.1* %tmp, null
  br i1 %tobool, label %return, label %if.then

if.then:                                          ; preds = %entry
; CHECK-TRANS:      ; ArrayInst: Array method call
; CHECK-TRANS-NEXT:   %call = tail call %class.DatatypeValidator** @"ValueVectorOf<DatatypeValidator*>::elementAt(unsigned int)"(%class.ValueVectorOf.1* %tmp, i32 %index)
; CHECK-MOD:          %call = tail call %class.DatatypeValidator** @"ValueVectorOf<DatatypeValidator*>::elementAt(unsigned int){{.*}}"(%__SOA_AR_class.ValueVectorOf.0* %tmp, i32 %index)
  %call = tail call %class.DatatypeValidator** @"ValueVectorOf<DatatypeValidator*>::elementAt(unsigned int)"(%class.ValueVectorOf.1* %tmp, i32 %index)
  %tmp1 = load %class.DatatypeValidator*, %class.DatatypeValidator** %call
  br label %return

return:                                           ; preds = %if.then, %entry
  %retval.0 = phi %class.DatatypeValidator* [ %tmp1, %if.then ], [ null, %entry ]
  ret %class.DatatypeValidator* %retval.0
}

; CHECK-MOD: @"ValueVectorOf<DatatypeValidator*>::elementAt(unsigned int).2"(%__SOA_AR_class.ValueVectorOf.0*, i32)
declare hidden %class.DatatypeValidator** @"ValueVectorOf<DatatypeValidator*>::elementAt(unsigned int)"(%class.ValueVectorOf.1*, i32)
; CHECK-TRANS: ; Seen pointer to element returned.
; XCHECK-DEP: Deps computed: 10, Queries: 11
