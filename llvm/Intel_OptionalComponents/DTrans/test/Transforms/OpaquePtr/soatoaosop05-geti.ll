; RUN: opt < %s -whole-program-assume -intel-libirc-allowed -disable-output      \
; RUN:          -debug-only=dtrans-soatoaosop-deps                                                \
; RUN:          -passes='require<dtrans-safetyanalyzer>,require<soatoaosop-approx>'               \
; RUN:       2>&1 | FileCheck --check-prefix=CHECK-OP-DEP %s
; RUN: opt < %s -whole-program-assume -intel-libirc-allowed -disable-output      \
; RUN:          -debug-only=dtrans-soatoaosop,dtrans-soatoaosop-struct                            \
; RUN:          -passes='require<dtrans-safetyanalyzer>,require<soatoaosop-approx>,require<soatoaosop-struct-methods>' \
; RUN:          -dtrans-soatoaos-mem-off=3                                                        \
; RUN:          -dtrans-soatoaosop-array-type=class.ValueVectorOf.0                               \
; RUN:          -dtrans-soatoaosop-array-type=class.ValueVectorOf.1                               \
; RUN:          -dtrans-soatoaosop-base-ptr-off=3                                                 \
; RUN:       2>&1 | FileCheck --check-prefix=CHECK-OP-TRANS %s
; RUN: opt -S < %s -whole-program-assume -intel-libirc-allowed                   \
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
; CHECK-OP-DEP-NOT: ; {{.*}}Unknown{{.*}}Dep
; There should be no unknown GEP
; CHECK-OP-DEP-NOT: ; Func(GEP

; Checks that all instructions can be dealt with.
; CHECK-OP_TRANS: ; Checking structure's method FieldValueMap::size() const
; CHECK-OP_TRANS: ; IR: has only expected side-effects

; Checks instructions related to transformations:
; CHECK-OP-TRANS: ; Dump instructions needing update. Total = 2

; Checks transformation. Only types change.

; inline unsigned int FieldValueMap::size() const {
;   if (fFields) {
;     return fFields->size();
;   }
;   return 0;
; }

; CHECK-OP-MOD-DAG: %__SOA_class.FieldValueMap = type { ptr, ptr, ptr, ptr }
; CHECK-OP-MOD-DAG: %__SOA_AR_class.ValueVectorOf.0 = type { i8, i32, i32, ptr, ptr }
; CHECK-OP-MOD-DAG: %__SOA_EL_class.FieldValueMap = type { ptr, ptr }
; CHECK-OP-MOD-NOT: ValueVectorOf.1
%class.FieldValueMap = type { ptr, ptr, ptr, ptr }
%class.DatatypeValidator = type opaque
%class.IC_Field = type opaque
%class.XMLMsgLoader = type { ptr }
%class.BaseRefVectorOf = type { ptr, i8, i32, i32, ptr, ptr }
%class.RefArrayVectorOf = type { %class.BaseRefVectorOf }
%class.ValueVectorOf.1 = type { i8, i32, i32, ptr, ptr }
%class.ValueVectorOf.0 = type { i8, i32, i32, ptr, ptr }

; CHECK-OP-MOD: @"ValueVectorOf<IC_Field*>::size() const"(ptr "intel_dtrans_func_index"="1")
declare !intel.dtrans.func.type !20 hidden i32 @"ValueVectorOf<IC_Field*>::size() const"(ptr "intel_dtrans_func_index"="1")

; CHECK-OP-MOD: @"FieldValueMap::size() const"(ptr "intel_dtrans_func_index"="1" %this)
define hidden i32 @"FieldValueMap::size() const"(ptr "intel_dtrans_func_index"="1" %this) !intel.dtrans.func.type !21 {
entry:
; CHECK-OP-MOD:       %fFields = getelementptr inbounds %__SOA_class.FieldValueMap, ptr %this, i64 0, i32 0
  %fFields = getelementptr inbounds %class.FieldValueMap, ptr %this, i64 0, i32 0
; CHECK-OP-TRANS:      ; ArrayInst: Load of array
; CHECK-OP-TRANS-NEXT:   %tmp = load ptr, ptr %fFields
; CHECK-OP-MOD-NEXT:     %tmp = load ptr, ptr %fFields
  %tmp = load ptr, ptr %fFields, align 8
; CHECK-OP-MOD-NEXT:     %tobool = icmp eq ptr %tmp, null
  %tobool = icmp eq ptr %tmp, null
  br i1 %tobool, label %return, label %if.then

if.then:                                          ; preds = %entry
; CHECK-OP-TRANS:      ; ArrayInst: Array method call
; CHECK-OP-TRANS-NEXT:   %call = tail call i32 @"ValueVectorOf<IC_Field*>::size() const"(ptr %tmp)
; CHECK-OP-MOD:       %call = tail call i32 @"ValueVectorOf<IC_Field*>::size() const"(ptr %tmp)
  %call = tail call i32 @"ValueVectorOf<IC_Field*>::size() const"(ptr %tmp)
  br label %return

return:                                           ; preds = %if.then, %entry
  %retval.0 = phi i32 [ %call, %if.then ], [ 0, %entry ]
  ret i32 %retval.0
}

; XCHECK-DEP: Deps computed: 7, Queries: 9

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
!20 = distinct !{!17}
!21 = distinct !{!22}
!22 = !{%class.FieldValueMap zeroinitializer, i32 1}
