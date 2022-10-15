; RUN: opt < %s -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -disable-output                                                       \
; RUN: -passes='require<dtrans-safetyanalyzer>,require<soatoaosop-approx>,require<soatoaosop-array-methods>' \
; RUN:        -dtrans-soatoaosop-base-ptr-off=3 -dtrans-soatoaosop-mem-off=4                                    \
; RUN:        -debug-only=dtrans-soatoaosop,dtrans-soatoaosop-arrays                                            \
; RUN:        2>&1 | FileCheck %s
; RUN: opt -S < %s -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed                                                                    \
; RUN:        -passes=soatoaosop-arrays-methods-transform                                                     \
; RUN:        -dtrans-soatoaosop-base-ptr-off=3 -dtrans-soatoaosop-mem-off=4                                    \
; RUN:        | FileCheck --check-prefix=CHECK-MOD %s
;
; RUN: opt < %s -opaque-pointers -whole-program-assume -intel-libirc-allowed -disable-output                                                       \
; RUN: -passes='require<dtrans-safetyanalyzer>,require<soatoaosop-approx>,require<soatoaosop-array-methods>' \
; RUN:        -dtrans-soatoaosop-base-ptr-off=3 -dtrans-soatoaosop-mem-off=4                                    \
; RUN:        -debug-only=dtrans-soatoaosop,dtrans-soatoaosop-arrays                                            \
; RUN:        2>&1 | FileCheck --check-prefix=CHECK-OP %s
; RUN: opt -S < %s -opaque-pointers -whole-program-assume -intel-libirc-allowed                                                                    \
; RUN:        -passes=soatoaosop-arrays-methods-transform                                                     \
; RUN:        -dtrans-soatoaosop-base-ptr-off=3 -dtrans-soatoaosop-mem-off=4                                    \
; RUN:        | FileCheck --check-prefix=CHECK-OP-MOD %s
; REQUIRES: asserts
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%class.ValueVectorOf = type { i8, i32, i32, %class.IC_Field**, %class.XMLMsgLoader* }
%class.IC_Field = type opaque
%class.XMLMsgLoader = type { i32 (...)** }
; CHECK-MOD-DAG: %__SOA_class.ValueVectorOf = type { i8, i32, i32, %__SOA_EL_class.ValueVectorOf*, %class.XMLMsgLoader* }
; CHECK-MOD-DAG: %__SOA_EL_class.ValueVectorOf = type { float*, %class.IC_Field* }
; CHECK-OP-MOD-DAG: %__SOA_class.ValueVectorOf = type { i8, i32, i32, ptr, ptr }
; CHECK-OP-MOD-DAG: %__SOA_EL_class.ValueVectorOf = type { ptr, ptr }

; The following method should be classified as get-integer-field-like method.
; Instructions to transform are shown.
; Transformed instructions are shown.
;  template <class TElem> unsigned int ValueVectorOf<TElem>::size() const {
;    return fCurCount;
;  }
; CHECK:; Classification: Get integer field method
; CHECK:; Dump instructions needing update. Total = 0
; CHECK-OP:; Classification: Get integer field method
; CHECK-OP:; Dump instructions needing update. Total = 0
define i32 @"ValueVectorOf<IC_Field*>::size() const"(%class.ValueVectorOf* "intel_dtrans_func_index"="1" %this) !intel.dtrans.func.type !9 {
entry:
; CHECK-MOD:  %fCurCount = getelementptr inbounds %__SOA_class.ValueVectorOf, %__SOA_class.ValueVectorOf* %this, i64 0, i32 1
; CHECK-OP-MOD:  %fCurCount = getelementptr inbounds %__SOA_class.ValueVectorOf, ptr %this, i64 0, i32 1
  %fCurCount = getelementptr inbounds %class.ValueVectorOf, %class.ValueVectorOf* %this, i64 0, i32 1
  %tmp = load i32, i32* %fCurCount
  ret i32 %tmp
}

!intel.dtrans.types = !{!0, !1, !5}

!0 = !{!"S", %class.IC_Field zeroinitializer, i32 -1}
!1 = !{!"S", %class.XMLMsgLoader zeroinitializer, i32 1, !2}
!2 = !{!3, i32 2}
!3 = !{!"F", i1 true, i32 0, !4}
!4 = !{i32 0, i32 0}
!5 = !{!"S", %class.ValueVectorOf zeroinitializer, i32 5, !6, !4, !4, !7, !8}
!6 = !{i8 0, i32 0}
!7 = !{%class.IC_Field zeroinitializer, i32 2}
!8 = !{%class.XMLMsgLoader zeroinitializer, i32 1}
!9 = distinct !{!10}
!10 = !{%class.ValueVectorOf zeroinitializer, i32 1}
