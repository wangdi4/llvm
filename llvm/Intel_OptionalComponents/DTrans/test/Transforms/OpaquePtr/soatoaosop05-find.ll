; RUN: opt < %s -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -disable-output -debug-only=dtrans-soatoaosop-deps          \
; RUN:          -passes='internalize,require<dtrans-safetyanalyzer>,require<soatoaosop-approx>'   \
; RUN:       2>&1 | FileCheck --check-prefix=CHECK-DEP %s
; RUN: opt < %s -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -disable-output              \
; RUN:          -debug-only=dtrans-soatoaosop,dtrans-soatoaosop-struct                            \
; RUN:          -passes='internalize,require<dtrans-safetyanalyzer>,require<soatoaosop-approx>,require<soatoaosop-struct-methods>' \
; RUN:          -dtrans-soatoaosop-mem-off=3                                                      \
; RUN:          -dtrans-soatoaosop-array-type=class.ValueVectorOf.0                               \
; RUN:          -dtrans-soatoaosop-array-type=class.ValueVectorOf.1                               \
; RUN:          -dtrans-soatoaosop-base-ptr-off=3                                                 \
; RUN:       2>&1 | FileCheck --check-prefix=CHECK-TRANS %s
; RUN: opt -S < %s -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed                           \
; RUN:          -passes='internalize,soatoaosop-struct-methods-transform'                         \
; RUN:          -dtrans-soatoaosop-mem-off=3                                                      \
; RUN:          -dtransop-optbase-process-function-declaration                                    \
; RUN:          -dtrans-soatoaosop-array-type=class.ValueVectorOf.0                               \
; RUN:          -dtrans-soatoaosop-array-type=class.ValueVectorOf.1                               \
; RUN:          -dtrans-soatoaosop-base-ptr-off=3                                                 \
; RUN:       | FileCheck --check-prefix=CHECK-MOD %s
;
; RUN: opt < %s -opaque-pointers -whole-program-assume -intel-libirc-allowed -disable-output                            \
; RUN:          -debug-only=dtrans-soatoaosop-deps                                                \
; RUN:          -passes='internalize,require<dtrans-safetyanalyzer>,require<soatoaosop-approx>'   \
; RUN:       2>&1 | FileCheck --check-prefix=CHECK-OP-DEP %s
; RUN: opt < %s -opaque-pointers -whole-program-assume -intel-libirc-allowed -disable-output                            \
; RUN:          -debug-only=dtrans-soatoaosop,dtrans-soatoaosop-struct                            \
; RUN:          -passes='internalize,require<dtrans-safetyanalyzer>,require<soatoaosop-approx>,require<soatoaosop-struct-methods>' \
; RUN:          -dtrans-soatoaosop-mem-off=3                                                      \
; RUN:          -dtrans-soatoaosop-array-type=class.ValueVectorOf.0                               \
; RUN:          -dtrans-soatoaosop-array-type=class.ValueVectorOf.1                               \
; RUN:          -dtrans-soatoaosop-base-ptr-off=3                                                 \
; RUN:       2>&1 | FileCheck --check-prefix=CHECK-OP-TRANS %s
; RUN: opt -S < %s -opaque-pointers -whole-program-assume -intel-libirc-allowed                                         \
; RUN:          -passes='internalize,soatoaosop-struct-methods-transform'                         \
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
; CHECK-TRANS: ; Checking structure's method FieldValueMap::indexOf(IC_Field const*) const
; CHECK-TRANS: ; IR: has only expected side-effects
; CHECK-OP-TRANS: ; Checking structure's method FieldValueMap::indexOf(IC_Field const*) const
; CHECK-OP-TRANS: ; IR: has only expected side-effects

; Checks instructions related to transformations.
; CHECK-TRANS: ; Dump instructions needing update. Total = 4
; CHECK-OP-TRANS: ; Dump instructions needing update. Total = 4

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

%class.FieldValueMap = type { %class.ValueVectorOf.0*, %class.ValueVectorOf.1*, %class.RefArrayVectorOf*, %class.XMLMsgLoader* }
%class.ValueVectorOf.0 = type { i8, i32, i32, %class.IC_Field**, %class.XMLMsgLoader* }
; CHECK-MOD-DAG: %__SOA_class.FieldValueMap = type { %__SOA_AR_class.ValueVectorOf.0*, float*, %class.RefArrayVectorOf*, %class.XMLMsgLoader* }
; CHECK-MOD-DAG: %__SOA_AR_class.ValueVectorOf.0 = type { i8, i32, i32, %__SOA_EL_class.FieldValueMap*, %class.XMLMsgLoader* }
; CHECK-MOD-DAG: %__SOA_EL_class.FieldValueMap = type { %class.IC_Field*, %class.DatatypeValidator* }
; CHECK-OP-MOD-DAG: %__SOA_class.FieldValueMap = type { ptr, ptr, ptr, ptr }
; CHECK-OP-MOD-DAG: %__SOA_AR_class.ValueVectorOf.0 = type { i8, i32, i32, ptr, ptr }
; CHECK-OP-MOD-DAG: %__SOA_EL_class.FieldValueMap = type { ptr, ptr }
%class.ValueVectorOf.1 = type { i8, i32, i32, %class.DatatypeValidator**, %class.XMLMsgLoader* }
; CHECK-MOD-NOT: ValueVectorOf.1
; CHECK-OP-MOD-NOT: ValueVectorOf.1
%class.DatatypeValidator = type opaque
%class.IC_Field = type opaque
%class.RefArrayVectorOf = type { %class.BaseRefVectorOf }
%class.BaseRefVectorOf = type { i32 (...)**, i8, i32, i32, i16**, %class.XMLMsgLoader* }
%class.XMLMsgLoader = type { i32 (...)** }

; CHECK-MOD: define internal i32 @"FieldValueMap::indexOf(IC_Field const*) const.1"(%__SOA_class.FieldValueMap* "intel_dtrans_func_index"="1" %this, %class.IC_Field* "intel_dtrans_func_index"="2" %key)
; CHECK-OP-MOD: i32 @"FieldValueMap::indexOf(IC_Field const*) const"(ptr "intel_dtrans_func_index"="1" %this, ptr "intel_dtrans_func_index"="2" %key)
define hidden i32 @"FieldValueMap::indexOf(IC_Field const*) const"(%class.FieldValueMap* "intel_dtrans_func_index"="1" %this, %class.IC_Field* "intel_dtrans_func_index"="2" %key) !intel.dtrans.func.type !25 {
entry:
  %fFields = getelementptr inbounds %class.FieldValueMap, %class.FieldValueMap* %this, i64 0, i32 0
; CHECK-TRANS:      ; ArrayInst: Load of array
; CHECK-TRANS-NEXT:   %tmp = load %class.ValueVectorOf.0*, %class.ValueVectorOf.0** %fFields
; CHECK-OP-TRANS:      ; ArrayInst: Load of array
; CHECK-OP-TRANS-NEXT:   %tmp = load ptr, ptr %fFields
; CHECK-MOD:          %tmp = load %__SOA_AR_class.ValueVectorOf.0*, %__SOA_AR_class.ValueVectorOf.0** %fFields
; CHECK-OP-MOD:       %tmp = load ptr, ptr %fFields
  %tmp = load %class.ValueVectorOf.0*, %class.ValueVectorOf.0** %fFields
; CHECK-MOD:          %tobool = icmp eq %__SOA_AR_class.ValueVectorOf.0* %tmp, null
; CHECK-OP-MOD:       %tobool = icmp eq ptr %tmp, null
  %tobool = icmp eq %class.ValueVectorOf.0* %tmp, null
  br i1 %tobool, label %return, label %if.then

if.then:                                          ; preds = %entry
; CHECK-TRANS:      ; ArrayInst: Array method call
; CHECK-TRANS-NEXT:   %call = tail call i32 @"ValueVectorOf<IC_Field*>::size() const"(%class.ValueVectorOf.0* %tmp)
; CHECK-OP-TRANS:      ; ArrayInst: Array method call
; CHECK-OP-TRANS-NEXT:   %call = tail call i32 @"ValueVectorOf<IC_Field*>::size() const"(ptr %tmp)
; CHECK-MOD:          %call = tail call i32 @"ValueVectorOf<IC_Field*>::size() const{{.*}}"(%__SOA_AR_class.ValueVectorOf.0* %tmp)
; CHECK-OP-MOD:       %call = tail call i32 @"ValueVectorOf<IC_Field*>::size() const"(ptr %tmp)
  %call = tail call i32 @"ValueVectorOf<IC_Field*>::size() const"(%class.ValueVectorOf.0* %tmp)
  %cmp17 = icmp eq i32 %call, 0
  br i1 %cmp17, label %return, label %for.body

for.body:                                         ; preds = %for.inc, %if.then
  %i.018 = phi i32 [ %inc, %for.inc ], [ 0, %if.then ]
; CHECK-TRANS:      ; ArrayInst: Load of array
; CHECK-TRANS-NEXT:   %tmp1 = load %class.ValueVectorOf.0*, %class.ValueVectorOf.0** %fFields
; CHECK-OP-TRANS:      ; ArrayInst: Load of array
; CHECK-OP-TRANS-NEXT:   %tmp1 = load ptr, ptr %fFields
; CHECK-MOD:          %tmp1 = load %__SOA_AR_class.ValueVectorOf.0*, %__SOA_AR_class.ValueVectorOf.0** %fFields
; CHECK-OP-MOD:       %tmp1 = load ptr, ptr %fFields
  %tmp1 = load %class.ValueVectorOf.0*, %class.ValueVectorOf.0** %fFields
; CHECK-TRANS:      ; ArrayInst: Array method call
; CHECK-TRANS-NEXT:   %call4 = tail call %class.IC_Field** @"ValueVectorOf<IC_Field*>::elementAt(unsigned int)"(%class.ValueVectorOf.0* %tmp1, i32 %i.018)
; CHECK-OP-TRANS:      ; ArrayInst: Array method call
; CHECK-OP-TRANS-NEXT:   %call4 = tail call ptr @"ValueVectorOf<IC_Field*>::elementAt(unsigned int)"(ptr %tmp1, i32 %i.018)
; CHECK-MOD-NEXT:     %call4 = tail call %class.IC_Field** @"ValueVectorOf<IC_Field*>::elementAt(unsigned int){{.*}}"(%__SOA_AR_class.ValueVectorOf.0* %tmp1, i32 %i.018)
; CHECK-OP-MOD-NEXT:  %call4 = tail call ptr @"ValueVectorOf<IC_Field*>::elementAt(unsigned int)"(ptr %tmp1, i32 %i.018)
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

; CHECK-MOD: declare hidden i32 @"ValueVectorOf<IC_Field*>::size() const.2"(%__SOA_AR_class.ValueVectorOf.0* "intel_dtrans_func_index"="1")
; CHECK-OP-MOD: i32 @"ValueVectorOf<IC_Field*>::size() const"(ptr "intel_dtrans_func_index"="1")
declare !intel.dtrans.func.type !27 hidden i32 @"ValueVectorOf<IC_Field*>::size() const"(%class.ValueVectorOf.0* "intel_dtrans_func_index"="1")

declare !intel.dtrans.func.type !27 hidden %class.IC_Field** @"ValueVectorOf<IC_Field*>::elementAt(unsigned int)"(%class.ValueVectorOf.0* "intel_dtrans_func_index"="1", i32)

; XCHECK-DEP: Deps computed: 10, Queries: 19

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
!25 = distinct !{!24, !26}
!26 = !{%"class.IC_Field" zeroinitializer, i32 1}
!27 = distinct !{!21}
