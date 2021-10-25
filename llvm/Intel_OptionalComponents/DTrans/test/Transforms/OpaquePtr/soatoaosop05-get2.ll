; RUN: opt < %s -whole-program-assume -disable-output -debug-only=dtrans-soatoaosop-deps          \
; RUN:          -passes='require<dtrans-safetyanalyzer>,function(require<soatoaosop-approx>)'            \
; RUN:       2>&1 | FileCheck --check-prefix=CHECK-DEP %s
; RUN: opt < %s -whole-program-assume -disable-output                                           \
; RUN:          -debug-only=dtrans-soatoaosop,dtrans-soatoaosop-struct                              \
; RUN:          -passes='require<dtrans-safetyanalyzer>,function(require<soatoaosop-approx>,require<soatoaosop-struct-methods>)' \
; RUN:          -dtrans-soatoaos-mem-off=3                                                      \
; RUN:          -dtrans-soatoaosop-array-type=class.ValueVectorOf.0                               \
; RUN:          -dtrans-soatoaosop-array-type=class.ValueVectorOf.1                               \
; RUN:          -dtrans-soatoaosop-base-ptr-off=3                                                 \
; RUN:       2>&1 | FileCheck --check-prefix=CHECK-TRANS %s
; RUN: opt -S < %s -whole-program-assume                                                        \
; RUN:          -passes=soatoaosop-struct-methods-transform                                       \
; RUN:          -dtrans-soatoaosop-mem-off=3                                                      \
; RUN:          -dtransop-optbase-process-function-declaration                                    \
; RUN:          -dtrans-soatoaosop-array-type=class.ValueVectorOf.0                               \
; RUN:          -dtrans-soatoaosop-array-type=class.ValueVectorOf.1                               \
; RUN:          -dtrans-soatoaosop-base-ptr-off=3                                                 \
; RUN:       | FileCheck --check-prefix=CHECK-MOD %s
;
; RUN: opt < %s -force-opaque-pointers -whole-program-assume -disable-output                     \
; RUN:          -debug-only=dtrans-soatoaosop-deps                                               \
; RUN:          -passes='require<dtrans-safetyanalyzer>,function(require<soatoaosop-approx>)'    \
; RUN:       2>&1 | FileCheck --check-prefix=CHECK-OP-DEP %s
; RUN: opt < %s -force-opaque-pointers -whole-program-assume -disable-output                     \
; RUN:          -debug-only=dtrans-soatoaosop,dtrans-soatoaosop-struct                              \
; RUN:          -passes='require<dtrans-safetyanalyzer>,function(require<soatoaosop-approx>,require<soatoaosop-struct-methods>)' \
; RUN:          -dtrans-soatoaos-mem-off=3                                                      \
; RUN:          -dtrans-soatoaosop-array-type=class.ValueVectorOf.0                               \
; RUN:          -dtrans-soatoaosop-array-type=class.ValueVectorOf.1                               \
; RUN:          -dtrans-soatoaosop-base-ptr-off=3                                                 \
; RUN:       2>&1 | FileCheck --check-prefix=CHECK-OP-TRANS %s
; RUN: opt -S < %s -force-opaque-pointers -whole-program-assume                                                        \
; RUN:          -passes=soatoaosop-struct-methods-transform                                       \
; RUN:          -dtrans-soatoaosop-mem-off=3                                                      \
; RUN:          -dtransop-optbase-process-function-declaration                                    \
; RUN:          -dtrans-soatoaosop-array-type=class.ValueVectorOf.0                               \
; RUN:          -dtrans-soatoaosop-array-type=class.ValueVectorOf.1                               \
; RUN:          -dtrans-soatoaosop-base-ptr-off=3                                                 \
; RUN:       | FileCheck --check-prefix=CHECK-OP-MOD %s
; REQUIRES: asserts

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Check that approximations work as expected.
; Checks special cases in approximation computations (see inlined checks).
; CHECK-DEP-NOT: ; {{.*}}Unknown{{.*}}Dep
; CHECK-OP-DEP-NOT: ; {{.*}}Unknown{{.*}}Dep
; There should be no unknown GEP
; CHECK-DEP-NOT: ; Func(GEP
; CHECK-OP-DEP-NOT: ; Func(GEP

; Checks that all instructions can be dealt with.
; CHECK-TRANS: ; Checking structure's method FieldValueMap::getDatatypeValidatorAt(unsigned int) const
; CHECK-TRANS: ; IR: has only expected side-effects
; CHECK-OP-TRANS: ; Checking structure's method FieldValueMap::getDatatypeValidatorAt(unsigned int) const
; CHECK-OP-TRANS: ; IR: has only expected side-effects

; Checks instructions related to transformations:
; CHECK-TRANS: ; Dump instructions needing update. Total = 2
; CHECK-OP-TRANS: ; Dump instructions needing update. Total = 2

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
; CHECK-OP-MOD-DAG: %__SOA_class.FieldValueMap = type { ptr, ptr, ptr, ptr }
; CHECK-OP-MOD-DAG: %__SOA_AR_class.ValueVectorOf.0 = type { i8, i32, i32, ptr, ptr }
; CHECK-OP-MOD-DAG: %__SOA_EL_class.FieldValueMap = type { ptr, ptr }
; CHECK-OP-MOD-NOT: ValueVectorOf.1
%class.ValueVectorOf.1 = type { i8, i32, i32, %class.DatatypeValidator**, %class.XMLMsgLoader* }
%class.DatatypeValidator = type opaque
%class.IC_Field = type opaque
%class.RefArrayVectorOf = type { %class.BaseRefVectorOf }
%class.BaseRefVectorOf = type { i32 (...)**, i8, i32, i32, i16**, %class.XMLMsgLoader* }
%class.XMLMsgLoader = type { i32 (...)** }

; CHECK-MOD: @"FieldValueMap::getDatatypeValidatorAt(unsigned int) const.1"(%__SOA_class.FieldValueMap* "intel_dtrans_func_index"="2" %this, i32 %index)
; CHECK-OP-MOD: @"FieldValueMap::getDatatypeValidatorAt(unsigned int) const"(ptr "intel_dtrans_func_index"="2" %this, i32 %index)
define hidden "intel_dtrans_func_index"="1" %class.DatatypeValidator* @"FieldValueMap::getDatatypeValidatorAt(unsigned int) const"(%class.FieldValueMap* "intel_dtrans_func_index"="2" %this, i32 %index) !intel.dtrans.func.type !25 {
entry:
; CHECK-MOD:          %fValidators = getelementptr inbounds %__SOA_class.FieldValueMap, %__SOA_class.FieldValueMap* %this, i64 0, i32 0
; CHECK-OP-MOD:       %fValidators = getelementptr inbounds %__SOA_class.FieldValueMap, ptr %this, i64 0, i32 0
  %fValidators = getelementptr inbounds %class.FieldValueMap, %class.FieldValueMap* %this, i64 0, i32 1
; CHECK-TRANS:      ; ArrayInst: Load of array
; CHECK-TRANS-NEXT:   %tmp = load %class.ValueVectorOf.1*, %class.ValueVectorOf.1** %fValidators
; CHECK-OP-TRANS:      ; ArrayInst: Load of array
; CHECK-OP-TRANS-NEXT:   %tmp = load ptr, ptr %fValidators
; CHECK-MOD-NEXT:     %tmp = load %__SOA_AR_class.ValueVectorOf.0*, %__SOA_AR_class.ValueVectorOf.0** %fValidators
; CHECK-OP-MOD-NEXT:     %tmp = load ptr, ptr %fValidators
  %tmp = load %class.ValueVectorOf.1*, %class.ValueVectorOf.1** %fValidators
; CHECK-MOD-NEXT:     %tobool = icmp eq %__SOA_AR_class.ValueVectorOf.0* %tmp, null
; CHECK-OP-MOD-NEXT:     %tobool = icmp eq ptr %tmp, null
  %tobool = icmp eq %class.ValueVectorOf.1* %tmp, null
  br i1 %tobool, label %return, label %if.then

if.then:                                          ; preds = %entry
; CHECK-TRANS:      ; ArrayInst: Array method call
; CHECK-TRANS-NEXT:   %call = tail call %class.DatatypeValidator** @"ValueVectorOf<DatatypeValidator*>::elementAt(unsigned int)"(%class.ValueVectorOf.1* %tmp, i32 %index)
; CHECK-OP-TRANS:      ; ArrayInst: Array method call
; CHECK-OP-TRANS-NEXT:   %call = tail call ptr @"ValueVectorOf<DatatypeValidator*>::elementAt(unsigned int)"(ptr %tmp, i32 %index)
; CHECK-MOD:          %call = tail call %class.DatatypeValidator** @"ValueVectorOf<DatatypeValidator*>::elementAt(unsigned int){{.*}}"(%__SOA_AR_class.ValueVectorOf.0* %tmp, i32 %index)
; CHECK-OP-MOD:       %call = tail call ptr @"ValueVectorOf<DatatypeValidator*>::elementAt(unsigned int)"(ptr %tmp, i32 %index)
  %call = tail call %class.DatatypeValidator** @"ValueVectorOf<DatatypeValidator*>::elementAt(unsigned int)"(%class.ValueVectorOf.1* %tmp, i32 %index)
  %tmp1 = load %class.DatatypeValidator*, %class.DatatypeValidator** %call
  br label %return

return:                                           ; preds = %if.then, %entry
  %retval.0 = phi %class.DatatypeValidator* [ %tmp1, %if.then ], [ null, %entry ]
  ret %class.DatatypeValidator* %retval.0
}

; CHECK-MOD: @"ValueVectorOf<DatatypeValidator*>::elementAt(unsigned int).2"(%__SOA_AR_class.ValueVectorOf.0* "intel_dtrans_func_index"="1", i32)
; CHECK-OP-MOD: @"ValueVectorOf<DatatypeValidator*>::elementAt(unsigned int)"(ptr "intel_dtrans_func_index"="1", i32)
declare !intel.dtrans.func.type !26 hidden %class.DatatypeValidator** @"ValueVectorOf<DatatypeValidator*>::elementAt(unsigned int)"(%class.ValueVectorOf.1* "intel_dtrans_func_index"="1", i32)
; CHECK-TRANS: ; Seen pointer to element returned.
; CHECK-OP-TRANS: ; Seen pointer to element returned.
; XCHECK-DEP: Deps computed: 10, Queries: 11

!intel.dtrans.types = !{ !1, !2, !3, !7, !14, !16, !18, !20}

!1 = !{!"S", %"class.DatatypeValidator" zeroinitializer, i32 -1}
!2 = !{!"S", %"class.IC_Field" zeroinitializer, i32 -1}
!3 = !{!"S", %"class.XMLMsgLoader" zeroinitializer, i32 1, !4}
!4 = !{!5, i32 2}
!5 = !{!"F", i1 true, i32 0, !6}
!6 = !{i32 0, i32 0}
!7 = !{!"S", %"class.BaseRefVectorOf" zeroinitializer, i32 6, !9, !12, !11, !11, !8, !13}
!8 = !{i16 0, i32 2}
!9 = !{!10, i32 2}
!10 = !{!"F", i1 true, i32 0, !11}
!11 = !{i32 0, i32 0}
!12 = !{i8 0, i32 0}
!13 = !{%"class.XMLMsgLoader" zeroinitializer, i32 1}
!14 = !{!"S", %"class.RefArrayVectorOf" zeroinitializer, i32 1, !15}
!15 = !{%"class.BaseRefVectorOf" zeroinitializer, i32 0}
!16 = !{!"S", %"class.ValueVectorOf.1" zeroinitializer, i32 5, !12, !11, !11, !17, !13}
!17 = !{%"class.DatatypeValidator" zeroinitializer, i32 2}
!18 = !{!"S", %"class.ValueVectorOf.0" zeroinitializer, i32 5, !12, !11, !11, !19, !13}
!19 = !{%"class.IC_Field" zeroinitializer, i32 2}
!20 = !{!"S", %"class.FieldValueMap" zeroinitializer, i32 4, !21, !22, !23, !13}
!21 = !{%"class.ValueVectorOf.0" zeroinitializer, i32 1}
!22 = !{%"class.ValueVectorOf.1" zeroinitializer, i32 1}
!23 = !{%"class.RefArrayVectorOf" zeroinitializer, i32 1}
!24 = !{%"class.FieldValueMap" zeroinitializer, i32 1}
!25 = distinct !{!27, !24}
!26 = distinct !{!22}
!27 = !{%"class.DatatypeValidator" zeroinitializer, i32 1}
