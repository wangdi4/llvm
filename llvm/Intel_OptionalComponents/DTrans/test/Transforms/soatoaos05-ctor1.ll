; RUN: opt < %s -whole-program-assume -disable-output -debug-only=dtrans-soatoaos-deps          \
; RUN:          -passes='require<dtransanalysis>,function(require<soatoaos-approx>)'            \
; RUN:       2>&1 | FileCheck --check-prefix=CHECK-DEP %s
; RUN: opt < %s -whole-program-assume -disable-output                                           \
; RUN:          -debug-only=dtrans-soatoaos,dtrans-soatoaos-struct                              \
; RUN:          -passes='require<dtransanalysis>,function(require<soatoaos-approx>,require<soatoaos-struct-methods>)' \
; RUN:          -dtrans-soatoaos-mem-off=3                                                      \
; RUN:          -dtrans-soatoaos-array-type=class.ValueVectorOf.0                               \
; RUN:          -dtrans-soatoaos-array-type=class.ValueVectorOf.1                               \
; RUN:          -dtrans-soatoaos-base-ptr-off=3                                                 \
; RUN:       2>&1 | FileCheck --check-prefix=CHECK-TRANS %s
; RUN: opt -S < %s -whole-program-assume                                                        \
; RUN:          -passes=soatoaos-struct-methods-transform                                       \
; RUN:          -dtrans-soatoaos-mem-off=3                                                      \
; RUN:          -dtrans-optbase-process-function-declaration                                    \
; RUN:          -dtrans-soatoaos-array-type=class.ValueVectorOf.0                               \
; RUN:          -dtrans-soatoaos-array-type=class.ValueVectorOf.1                               \
; RUN:          -dtrans-soatoaos-base-ptr-off=3                                                 \
; RUN:       | FileCheck --check-prefix=CHECK-MOD %s
; REQUIRES: asserts

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Check that approximations work as expected.
; Checks special cases in approximation computations (see inlined checks).
; CHECK-DEP-NOT: ; {{.*}}Unknown{{.*}}Dep
; There should be no unknown GEP
; CHECK-DEP-NOT: ; Func(GEP

%"class.FieldValueMap" = type { %"class.ValueVectorOf.0"*, %"class.ValueVectorOf.1"*, %"class.RefArrayVectorOf"*, %"class.MemoryManager"* }
%"class.ValueVectorOf.0" = type { i8, i32, i32, %"class.IC_Field"**, %"class.MemoryManager"* }
; CHECK-MOD-DAG: %__SOA_class.FieldValueMap = type { %__SOA_AR_class.ValueVectorOf.0*, float*, %class.RefArrayVectorOf*, %class.MemoryManager* }
; CHECK-MOD-DAG: %__SOA_AR_class.ValueVectorOf.0 = type { i8, i32, i32, %__SOA_EL_class.FieldValueMap*, %class.MemoryManager* }
; CHECK-MOD-DAG: %__SOA_EL_class.FieldValueMap = type { %class.IC_Field*, %class.DatatypeValidator* }
; CHECK-MOD-NOT: ValueVectorOf.1
%"class.ValueVectorOf.1" = type { i8, i32, i32, %"class.DatatypeValidator"**, %"class.MemoryManager"* }
%"class.DatatypeValidator" = type opaque
%"class.IC_Field" = type opaque
%"class.RefArrayVectorOf" = type opaque
%"class.MemoryManager" = type opaque

; Checks that all instructions can be dealt with.
; CHECK-TRANS: ; Checking structure's method FieldValueMap::FieldValueMap(MemoryManager*)
; CHECK-TRANS: ; IR: analysed completely

; Checks instructions related to transformations:
; FieldValueMap is not changed: one pointer is not used.
; CHECK-TRANS: ; Dump instructions needing update. Total = 2

; Checks transformation.

; FieldValueMap::FieldValueMap(MemoryManager *const manager)
;     : fFields(0), fValidators(0), fValues(0), fMemoryManager(manager) {}

define hidden void @"FieldValueMap::FieldValueMap(MemoryManager*)"(%"class.FieldValueMap"* nocapture %arg, %"class.MemoryManager"* %arg1) {
; CHECK-MOD:          %tmp = getelementptr inbounds %__SOA_class.FieldValueMap, %__SOA_class.FieldValueMap* %arg, i64 0, i32 0
  %tmp = getelementptr inbounds %class.FieldValueMap, %class.FieldValueMap* %arg, i64 0, i32 0
; CHECK-TRANS:      ; ArrayInst: Nullptr of array
; CHECK-TRANS-NEXT:   store %class.ValueVectorOf.0* null, %class.ValueVectorOf.0** %tmp
; CHECK-MOD-NEXT:     store %__SOA_AR_class.ValueVectorOf.0* null, %__SOA_AR_class.ValueVectorOf.0** %tmp
  store %class.ValueVectorOf.0* null, %class.ValueVectorOf.0** %tmp
; Offset changed to 0.
; CHECK-MOD-NEXT:     %tmp2 = getelementptr inbounds %__SOA_class.FieldValueMap, %__SOA_class.FieldValueMap* %arg, i64 0, i32 0
  %tmp2 = getelementptr inbounds %class.FieldValueMap, %class.FieldValueMap* %arg, i64 0, i32 1
; CHECK-TRANS:      ; ArrayInst: Nullptr of array
; CHECK-TRANS-NEXT:   store %class.ValueVectorOf.1* null, %class.ValueVectorOf.1** %tmp2
; CHECK-MOD-NEXT:     store %__SOA_AR_class.ValueVectorOf.0* null, %__SOA_AR_class.ValueVectorOf.0** %tmp2
  store %class.ValueVectorOf.1* null, %class.ValueVectorOf.1** %tmp2
  %tmp3 = getelementptr inbounds %class.FieldValueMap, %class.FieldValueMap* %arg, i64 0, i32 2
  store %class.RefArrayVectorOf* null, %class.RefArrayVectorOf** %tmp3
  %tmp4 = getelementptr inbounds %class.FieldValueMap, %class.FieldValueMap* %arg, i64 0, i32 3
  store %class.MemoryManager* %arg1, %class.MemoryManager** %tmp4
  ret void
}
; CHECK-TRANS:      ; Seen nullptr init.
; CHECK-TRANS-NEXT: ; Array call sites analysis result: required call sites can be merged


