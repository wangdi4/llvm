; RUN: opt < %s -whole-program-assume -intel-libirc-allowed -disable-output      \
; RUN:          -debug-only=dtrans-soatoaosop-deps                                                \
; RUN:          -passes='internalize,require<dtrans-safetyanalyzer>,require<soatoaosop-approx>'   \
; RUN:       2>&1 | FileCheck --check-prefix=CHECK-DEP %s
; RUN: opt < %s -whole-program-assume -intel-libirc-allowed -disable-output      \
; RUN:          -debug-only=dtrans-soatoaosop,dtrans-soatoaosop-struct                            \
; RUN:          -passes='internalize,require<dtrans-safetyanalyzer>,require<soatoaosop-approx>,require<soatoaosop-struct-methods>' \
; RUN:          -dtrans-soatoaosop-mem-off=3                                                      \
; RUN:          -dtrans-soatoaosop-array-type=class.ValueVectorOf.0                               \
; RUN:          -dtrans-soatoaosop-array-type=class.ValueVectorOf.1                               \
; RUN:          -dtrans-soatoaosop-base-ptr-off=3                                                 \
; RUN:       2>&1 | FileCheck --check-prefix=CHECK-TRANS %s
; RUN: opt -S < %s -whole-program-assume -intel-libirc-allowed                   \
; RUN:          -passes='internalize,soatoaosop-struct-methods-transform'                         \
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
; CHECK-TRANS: ; Checking structure's method FieldValueMap::indexOf(IC_Field const*) const
; CHECK-TRANS: ; IR: has only expected side-effects

; Checks instructions related to transformations.
; CHECK-TRANS: ; Dump instructions needing update. Total = 4

; Checks transformation. Only types change.
; Access to the first field ValueVectorOf.0 is performed. No update in indexes.

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

; CHECK-MOD: i32 @"FieldValueMap::indexOf(IC_Field const*) const"(ptr "intel_dtrans_func_index"="1" %this, ptr "intel_dtrans_func_index"="2" %key)
define hidden i32 @"FieldValueMap::indexOf(IC_Field const*) const"(ptr "intel_dtrans_func_index"="1" %this, ptr "intel_dtrans_func_index"="2" %key) !intel.dtrans.func.type !20 {
entry:
  %fFields = getelementptr inbounds %class.FieldValueMap, ptr %this, i64 0, i32 0
; CHECK-TRANS:      ; ArrayInst: Load of array
; CHECK-TRANS-NEXT:   %tmp = load ptr, ptr %fFields
; CHECK-MOD:       %tmp = load ptr, ptr %fFields
  %tmp = load ptr, ptr %fFields, align 8
; CHECK-MOD:       %tobool = icmp eq ptr %tmp, null
  %tobool = icmp eq ptr %tmp, null
  br i1 %tobool, label %return, label %if.then

if.then:                                          ; preds = %entry
; CHECK-TRANS:      ; ArrayInst: Array method call
; CHECK-TRANS-NEXT:   %call = tail call i32 @"ValueVectorOf<IC_Field*>::size() const"(ptr %tmp)
; CHECK-MOD:       %call = tail call i32 @"ValueVectorOf<IC_Field*>::size() const"(ptr %tmp)
  %call = tail call i32 @"ValueVectorOf<IC_Field*>::size() const"(ptr %tmp)
  %cmp17 = icmp eq i32 %call, 0
  br i1 %cmp17, label %return, label %for.body

for.body:                                         ; preds = %for.inc, %if.then
  %i.018 = phi i32 [ %inc, %for.inc ], [ 0, %if.then ]
; CHECK-TRANS:      ; ArrayInst: Load of array
; CHECK-TRANS-NEXT:   %tmp1 = load ptr, ptr %fFields
; CHECK-MOD:       %tmp1 = load ptr, ptr %fFields
  %tmp1 = load ptr, ptr %fFields, align 8
; CHECK-TRANS:      ; ArrayInst: Array method call
; CHECK-TRANS-NEXT:   %call4 = tail call ptr @"ValueVectorOf<IC_Field*>::elementAt(unsigned int)"(ptr %tmp1, i32 %i.018)
; CHECK-MOD-NEXT:  %call4 = tail call ptr @"ValueVectorOf<IC_Field*>::elementAt(unsigned int)"(ptr %tmp1, i32 %i.018)
  %call4 = tail call ptr @"ValueVectorOf<IC_Field*>::elementAt(unsigned int)"(ptr %tmp1, i32 %i.018)
  %tmp2 = load ptr, ptr %call4, align 8
  %cmp5 = icmp eq ptr %tmp2, %key
  br i1 %cmp5, label %return, label %for.inc

for.inc:                                          ; preds = %for.body
  %inc = add nuw i32 %i.018, 1
  %cmp = icmp ult i32 %inc, %call
  br i1 %cmp, label %for.body, label %return

return:                                           ; preds = %for.inc, %for.body, %if.then, %entry
  %retval.1 = phi i32 [ -1, %entry ], [ -1, %if.then ], [ -1, %for.inc ], [ %i.018, %for.body ]
  ret i32 %retval.1
}

; CHECK-MOD: i32 @"ValueVectorOf<IC_Field*>::size() const"(ptr "intel_dtrans_func_index"="1")
declare !intel.dtrans.func.type !23 hidden i32 @"ValueVectorOf<IC_Field*>::size() const"(ptr "intel_dtrans_func_index"="1")

declare !intel.dtrans.func.type !23 hidden ptr @"ValueVectorOf<IC_Field*>::elementAt(unsigned int)"(ptr "intel_dtrans_func_index"="1", i32)

; XCHECK-DEP: Deps computed: 10, Queries: 19

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
!21 = !{%class.FieldValueMap zeroinitializer, i32 1}
!22 = !{%class.IC_Field zeroinitializer, i32 1}
!23 = distinct !{!17}
