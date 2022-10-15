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
; RUN: opt < %s -opaque-pointers -whole-program-assume -intel-libirc-allowed -disable-output -debug-only=dtrans-soatoaosop-deps \
; RUN:          -passes='require<dtrans-safetyanalyzer>,require<soatoaosop-approx>'                        \
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

%"class.FieldValueMap" = type { %"class.ValueVectorOf.0"*, %"class.ValueVectorOf.1"*, %"class.RefArrayVectorOf"*, %"class.MemoryManager"* }
%"class.ValueVectorOf.0" = type { i8, i32, i32, %"class.IC_Field"**, %"class.MemoryManager"* }
; CHECK-MOD-DAG: %__SOA_class.FieldValueMap = type { %__SOA_AR_class.ValueVectorOf.0*, float*, %class.RefArrayVectorOf*, %class.MemoryManager* }
; CHECK-MOD-DAG: %__SOA_AR_class.ValueVectorOf.0 = type { i8, i32, i32, %__SOA_EL_class.FieldValueMap*, %class.MemoryManager* }
; CHECK-MOD-DAG: %__SOA_EL_class.FieldValueMap = type { %class.IC_Field*, %class.DatatypeValidator* }
; CHECK-MOD-NOT: ValueVectorOf.1
; CHECK-OP-MOD-DAG: %__SOA_class.FieldValueMap = type { ptr, ptr, ptr, ptr }
; CHECK-OP-MOD-DAG: %__SOA_AR_class.ValueVectorOf.0 = type { i8, i32, i32, ptr, ptr }
; CHECK-OP-MOD-DAG: %__SOA_EL_class.FieldValueMap = type { ptr, ptr }
; CHECK-OP-MOD-NOT: ValueVectorOf.1
%"class.ValueVectorOf.1" = type { i8, i32, i32, %"class.DatatypeValidator"**, %"class.MemoryManager"* }
%"class.DatatypeValidator" = type opaque
%"class.IC_Field" = type opaque
%"class.RefArrayVectorOf" = type opaque
%"class.MemoryManager" = type opaque

; Checks that all instructions can be dealt with.
; CHECK-TRANS: ; Checking structure's method FieldValueMap::FieldValueMap(MemoryManager*)
; CHECK-TRANS: ; IR: has only expected side-effects
; CHECK-OP-TRANS: ; Checking structure's method FieldValueMap::FieldValueMap
; CHECK-OP-TRANS: ; IR: has only expected side-effects

; Checks instructions related to transformations:
; FieldValueMap is not changed: one pointer is not used.
; CHECK-TRANS: ; Dump instructions needing update. Total = 2
; CHECK-OP-TRANS: ; Dump instructions needing update. Total = 2

; Checks transformation.

; FieldValueMap::FieldValueMap(MemoryManager *const manager)
;     : fFields(0), fValidators(0), fValues(0), fMemoryManager(manager) {}

define hidden void @"FieldValueMap::FieldValueMap(MemoryManager*)"(%"class.FieldValueMap"* nocapture "intel_dtrans_func_index"="1" %arg, %"class.MemoryManager"* "intel_dtrans_func_index"="2" %arg1) !intel.dtrans.func.type !17 {
; CHECK-MOD:          %tmp = getelementptr inbounds %__SOA_class.FieldValueMap, %__SOA_class.FieldValueMap* %arg, i64 0, i32 0
; CHECK-OP-MOD:       %tmp = getelementptr inbounds %__SOA_class.FieldValueMap, ptr %arg, i64 0, i32 0
  %tmp = getelementptr inbounds %class.FieldValueMap, %class.FieldValueMap* %arg, i64 0, i32 0
; CHECK-TRANS:      ; ArrayInst: Nullptr of array
; CHECK-TRANS-NEXT:   store %class.ValueVectorOf.0* null, %class.ValueVectorOf.0** %tmp
; CHECK-OP-TRANS:      ; ArrayInst: Nullptr of array
; CHECK-OP-TRANS-NEXT:   store ptr null, ptr %tmp
; CHECK-MOD-NEXT:     store %__SOA_AR_class.ValueVectorOf.0* null, %__SOA_AR_class.ValueVectorOf.0** %tmp
; CHECK-OP-MOD-NEXT:     store ptr null, ptr %tmp
  store %class.ValueVectorOf.0* null, %class.ValueVectorOf.0** %tmp
; Offset changed to 0.
; CHECK-MOD-NEXT:     %tmp2 = getelementptr inbounds %__SOA_class.FieldValueMap, %__SOA_class.FieldValueMap* %arg, i64 0, i32 0
; CHECK-OP-MOD-NEXT:     %tmp2 = getelementptr inbounds %__SOA_class.FieldValueMap, ptr %arg, i64 0, i32 0
  %tmp2 = getelementptr inbounds %class.FieldValueMap, %class.FieldValueMap* %arg, i64 0, i32 1
; CHECK-TRANS:      ; ArrayInst: Nullptr of array
; CHECK-TRANS-NEXT:   store %class.ValueVectorOf.1* null, %class.ValueVectorOf.1** %tmp2
; CHECK-OP-TRANS:      ; ArrayInst: Nullptr of array
; CHECK-OP-TRANS-NEXT:   store ptr null, ptr %tmp2
; CHECK-MOD-NEXT:     store %__SOA_AR_class.ValueVectorOf.0* null, %__SOA_AR_class.ValueVectorOf.0** %tmp2
; CHECK-OP-MOD-NEXT:     store ptr null, ptr %tmp2
  store %class.ValueVectorOf.1* null, %class.ValueVectorOf.1** %tmp2
  %tmp3 = getelementptr inbounds %class.FieldValueMap, %class.FieldValueMap* %arg, i64 0, i32 2
  store %class.RefArrayVectorOf* null, %class.RefArrayVectorOf** %tmp3
  %tmp4 = getelementptr inbounds %class.FieldValueMap, %class.FieldValueMap* %arg, i64 0, i32 3
  store %class.MemoryManager* %arg1, %class.MemoryManager** %tmp4
  ret void
}
; CHECK-TRANS:      ; Seen nullptr init.
; CHECK-TRANS-NEXT: ; Array call sites analysis result: required call sites can be merged
; CHECK-OP-TRANS:      ; Seen nullptr init.
; CHECK-OP-TRANS-NEXT: ; Array call sites analysis result: required call sites can be merged

!intel.dtrans.types = !{!1, !2, !3, !4, !10, !11, !15}

!1 = !{!"S", %"class.DatatypeValidator" zeroinitializer, i32 -1}
!2 = !{!"S", %"class.IC_Field" zeroinitializer, i32 -1}
!3 = !{!"S", %"class.RefArrayVectorOf" zeroinitializer, i32 -1}
!4 = !{!"S", %"class.MemoryManager" zeroinitializer, i32 -1}
!5 = !{i8 0, i32 0}
!6 = !{i32 0, i32 0}
!7 = !{%"class.MemoryManager" zeroinitializer, i32 1}
!8 = !{%"class.IC_Field" zeroinitializer, i32 2}
!9 = !{%"class.DatatypeValidator" zeroinitializer, i32 2}
!10 = !{!"S", %"class.ValueVectorOf.0" zeroinitializer, i32 5, !5, !6, !6, !8, !7}
!11 = !{!"S", %"class.ValueVectorOf.1" zeroinitializer, i32 5, !5, !6, !6, !9, !7}
!12 = !{%"class.ValueVectorOf.0" zeroinitializer, i32 1}
!13 = !{%"class.ValueVectorOf.1" zeroinitializer, i32 1}
!14 = !{%"class.RefArrayVectorOf" zeroinitializer, i32 1}
!15 = !{!"S", %"class.FieldValueMap" zeroinitializer, i32 4, !12, !13, !14, !7}
!16 = !{%"class.FieldValueMap" zeroinitializer, i32 1}
!17 = distinct !{!16, !7}
