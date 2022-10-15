; RUN: opt < %s -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -disable-output -debug-only=dtrans-soatoaosop-deps          \
; RUN:          -passes='require<dtrans-safetyanalyzer>,require<soatoaosop-approx>'               \
; RUN:       2>&1 | FileCheck --check-prefix=CHECK-DEP %s
; RUN: opt < %s -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -disable-output              \
; RUN:          -debug-only=dtrans-soatoaosop,dtrans-soatoaosop-struct                            \
; RUN:          -passes='require<dtrans-safetyanalyzer>,require<soatoaosop-approx>,require<soatoaosop-struct-methods>' \
; RUN:          -dtrans-soatoaosop-mem-off=3                                                      \
; RUN:          -dtrans-soatoaosop-array-type=class.ValueVectorOf.0                               \
; RUN:          -dtrans-soatoaosop-array-type=class.ValueVectorOf.1                               \
; RUN:          -dtrans-soatoaosop-base-ptr-off=3                                                 \
; RUN:       2>&1 | FileCheck --check-prefix=CHECK-TRANS %s
; RUN: opt -S < %s -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed                           \
; RUN:          -passes=soatoaosop-struct-methods-transform                                       \
; RUN:          -dtrans-soatoaosop-mem-off=3                                                      \
; RUN:          -dtransop-optbase-process-function-declaration                                    \
; RUN:          -dtrans-soatoaosop-array-type=class.ValueVectorOf.0                               \
; RUN:          -dtrans-soatoaosop-array-type=class.ValueVectorOf.1                               \
; RUN:          -dtrans-soatoaosop-base-ptr-off=3                                                 \
; RUN:       | FileCheck --check-prefix=CHECK-MOD %s
;
; RUN: opt < %s -opaque-pointers -whole-program-assume -intel-libirc-allowed -disable-output                            \
; RUN:          -debug-only=dtrans-soatoaosop-deps                                                \
; RUN:          -passes='require<dtrans-safetyanalyzer>,require<soatoaosop-approx>'               \
; RUN:       2>&1 | FileCheck --check-prefix=CHECK-OP-DEP %s
; RUN: opt < %s -opaque-pointers -whole-program-assume -intel-libirc-allowed -disable-output                            \
; RUN:          -debug-only=dtrans-soatoaosop,dtrans-soatoaosop-struct                            \
; RUN:          -passes='require<dtrans-safetyanalyzer>,require<soatoaosop-approx>,require<soatoaosop-struct-methods>' \
; RUN:          -dtrans-soatoaosop-mem-off=3                                                      \
; RUN:          -dtrans-soatoaosop-array-type=class.ValueVectorOf.0                               \
; RUN:          -dtrans-soatoaosop-array-type=class.ValueVectorOf.1                               \
; RUN:          -dtrans-soatoaosop-base-ptr-off=3                                                 \
; RUN:       2>&1 | FileCheck --check-prefix=CHECK-OP-TRANS %s
; RUN: opt -S < %s -opaque-pointers -whole-program-assume -intel-libirc-allowed                                         \
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
; CHECK-TRANS: ; Checking structure's method FieldValueMap::~FieldValueMap()
; CHECK-TRANS: ; IR: no need to analyze: no accesses to arrays
; CHECK-OP-TRANS: ; Checking structure's method FieldValueMap::~FieldValueMap()
; CHECK-OP-TRANS: ; IR: no need to analyze: no accesses to arrays

; Checks instructions related to transformations:
; CHECK-TRANS: ; Dump instructions needing update. Total = 0
; CHECK-OP-TRANS: ; Dump instructions needing update. Total = 0

; Checks transformation. Only types change.

; FieldValueMap::~FieldValueMap() {
;     cleanUp();
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

; CHECK-MOD: @"FieldValueMap::cleanUp().1"(%__SOA_class.FieldValueMap* "intel_dtrans_func_index"="1")
; CHECK-OP-MOD: @"FieldValueMap::cleanUp()"(ptr "intel_dtrans_func_index"="1")
declare !intel.dtrans.func.type !25 hidden void @"FieldValueMap::cleanUp()"(%class.FieldValueMap* "intel_dtrans_func_index"="1")

; CHECK-MOD: void @"FieldValueMap::~FieldValueMap().2"(%__SOA_class.FieldValueMap* "intel_dtrans_func_index"="1" %this)
; CHECK-OP-MOD: void @"FieldValueMap::~FieldValueMap()"(ptr "intel_dtrans_func_index"="1" %this)
define hidden void @"FieldValueMap::~FieldValueMap()"(%class.FieldValueMap* "intel_dtrans_func_index"="1" %this) !intel.dtrans.func.type !25 {
entry:
; CHECK-MOD:  tail call void @"FieldValueMap::cleanUp(){{.*}}"(%__SOA_class.FieldValueMap* %this)
; CHECK-OP-MOD:  tail call void @"FieldValueMap::cleanUp()"(ptr %this)
  tail call void @"FieldValueMap::cleanUp()"(%class.FieldValueMap* %this)
  ret void
}

; XCHECK-DEP: Deps computed: 4, Queries: 5

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
!25 = distinct !{!24}
