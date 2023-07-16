; RUN: opt < %s -whole-program-assume -intel-libirc-allowed -disable-output      \
; RUN:          -debug-only=dtrans-soatoaosop-deps                                                \
; RUN:          -passes='require<dtrans-safetyanalyzer>,require<soatoaosop-approx>'               \
; RUN:       2>&1 | FileCheck --check-prefix=CHECK-DEP %s
; RUN: opt < %s -whole-program-assume -intel-libirc-allowed -disable-output      \
; RUN:          -debug-only=dtrans-soatoaosop,dtrans-soatoaosop-struct                            \
; RUN:          -passes='require<dtrans-safetyanalyzer>,require<soatoaosop-approx>,require<soatoaosop-struct-methods>' \
; RUN:          -dtrans-soatoaosop-mem-off=3                                                        \
; RUN:          -dtrans-soatoaosop-array-type=class.ValueVectorOf.0                               \
; RUN:          -dtrans-soatoaosop-array-type=class.ValueVectorOf.1                               \
; RUN:          -dtrans-soatoaosop-base-ptr-off=3                                                 \
; RUN:       2>&1 | FileCheck --check-prefix=CHECK-TRANS %s
; RUN: opt -S < %s -whole-program-assume -intel-libirc-allowed                   \
; RUN:          -passes=soatoaosop-struct-methods-transform                                       \
; RUN:          -dtrans-soatoaosop-mem-off=3                                                      \
; RUN:          -dtransop-optbase-process-function-declaration                                    \
; RUN:          -dtrans-soatoaosop-array-type=class.ValueVectorOf.0                               \
; RUN:          -dtrans-soatoaosop-array-type=class.ValueVectorOf.1                               \
; RUN:          -dtrans-soatoaosop-base-ptr-off=3                                                 \
; RUN:       | FileCheck --check-prefix=CHECK-MOD %s
; REQUIRES: asserts

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

; CHECK-MOD-DAG: %__SOA_class.FieldValueMap = type { ptr, ptr, ptr, ptr }
; CHECK-MOD-DAG: %__SOA_AR_class.ValueVectorOf.0 = type { i8, i32, i32, ptr, ptr }
; CHECK-MOD-DAG: %__SOA_EL_class.FieldValueMap = type { ptr, ptr }
; CHECK-MOD-NOT: ValueVectorOf.1
%class.FieldValueMap = type { ptr, ptr, ptr, ptr }
%class.DatatypeValidator = type opaque
%class.IC_Field = type opaque
%class.XMLMsgLoader = type { ptr }
%class.BaseRefVectorOf = type { ptr, i8, i32, i32, ptr, ptr }
%class.RefArrayVectorOf = type { %class.BaseRefVectorOf }
%class.ValueVectorOf.1 = type { i8, i32, i32, ptr, ptr }
%class.ValueVectorOf.0 = type { i8, i32, i32, ptr, ptr }

; CHECK-MOD: @"FieldValueMap::getDatatypeValidatorAt(unsigned int) const"(ptr "intel_dtrans_func_index"="2" %this, i32 %index)
define hidden "intel_dtrans_func_index"="1" ptr @"FieldValueMap::getDatatypeValidatorAt(unsigned int) const"(ptr "intel_dtrans_func_index"="2" %this, i32 %index) !intel.dtrans.func.type !20 {
entry:
; CHECK-MOD:       %fValidators = getelementptr inbounds %__SOA_class.FieldValueMap, ptr %this, i64 0, i32 0
  %fValidators = getelementptr inbounds %class.FieldValueMap, ptr %this, i64 0, i32 1
; CHECK-TRANS:      ; ArrayInst: Load of array
; CHECK-TRANS-NEXT:   %tmp = load ptr, ptr %fValidators
; CHECK-MOD-NEXT:     %tmp = load ptr, ptr %fValidators
  %tmp = load ptr, ptr %fValidators, align 8
; CHECK-MOD-NEXT:     %tobool = icmp eq ptr %tmp, null
  %tobool = icmp eq ptr %tmp, null
  br i1 %tobool, label %return, label %if.then

if.then:                                          ; preds = %entry
; CHECK-TRANS:      ; ArrayInst: Array method call
; CHECK-TRANS-NEXT:   %call = tail call ptr @"ValueVectorOf<DatatypeValidator*>::elementAt(unsigned int)"(ptr %tmp, i32 %index)
; CHECK-MOD:       %call = tail call ptr @"ValueVectorOf<DatatypeValidator*>::elementAt(unsigned int)"(ptr %tmp, i32 %index)
  %call = tail call ptr @"ValueVectorOf<DatatypeValidator*>::elementAt(unsigned int)"(ptr %tmp, i32 %index)
  %tmp1 = load ptr, ptr %call, align 8
  br label %return

return:                                           ; preds = %if.then, %entry
  %retval.0 = phi ptr [ %tmp1, %if.then ], [ null, %entry ]
  ret ptr %retval.0
}

; CHECK-MOD: @"ValueVectorOf<DatatypeValidator*>::elementAt(unsigned int)"(ptr "intel_dtrans_func_index"="1", i32)
declare !intel.dtrans.func.type !23 hidden ptr @"ValueVectorOf<DatatypeValidator*>::elementAt(unsigned int)"(ptr "intel_dtrans_func_index"="1", i32)
; CHECK-TRANS: ; Seen pointer to element returned.
; XCHECK-DEP: Deps computed: 10, Queries: 11

!intel.dtrans.types = !{!0, !1, !2, !6, !10, !12, !14, !16}

!0 = !{!"S", %class.DatatypeValidator zeroinitializer, i32 -1}
!1 = !{!"S", %class.IC_Field zeroinitializer, i32 -1}
!2 = !{!"S", %class.XMLMsgLoader zeroinitializer, i32 1, !3}
!3 = !{!4, i32 2}
!4 = !{!"F", i1 true, i32 0, !5}
!5 = !{i32 0, i32 0}
!6 = !{!"S", %class.BaseRefVectorOf zeroinitializer, i32 6, !3, !7, !5, !5, !8, !9}
!7 = !{i8 0, i32 0}
!8 = !{i16 0, i32 2}
!9 = !{%class.XMLMsgLoader zeroinitializer, i32 1}
!10 = !{!"S", %class.RefArrayVectorOf zeroinitializer, i32 1, !11}
!11 = !{%class.BaseRefVectorOf zeroinitializer, i32 0}
!12 = !{!"S", %class.ValueVectorOf.1 zeroinitializer, i32 5, !7, !5, !5, !13, !9}
!13 = !{%class.DatatypeValidator zeroinitializer, i32 2}
!14 = !{!"S", %class.ValueVectorOf.0 zeroinitializer, i32 5, !7, !5, !5, !15, !9}
!15 = !{%class.IC_Field zeroinitializer, i32 2}
!16 = !{!"S", %class.FieldValueMap zeroinitializer, i32 4, !17, !18, !19, !9}
!17 = !{%class.ValueVectorOf.0 zeroinitializer, i32 1}
!18 = !{%class.ValueVectorOf.1 zeroinitializer, i32 1}
!19 = !{%class.RefArrayVectorOf zeroinitializer, i32 1}
!20 = distinct !{!21, !22}
!21 = !{%class.DatatypeValidator zeroinitializer, i32 1}
!22 = !{%class.FieldValueMap zeroinitializer, i32 1}
!23 = distinct !{!18}
