; RUN: opt < %s -whole-program-assume -intel-libirc-allowed -disable-output -debug-only=dtrans-soatoaosop-deps \
; RUN:          -passes='require<dtrans-safetyanalyzer>,require<soatoaosop-approx>'               \
; RUN:       2>&1 | FileCheck --check-prefix=CHECK-DEP %s
; RUN: opt < %s -whole-program-assume -intel-libirc-allowed -disable-output      \
; RUN:          -debug-only=dtrans-soatoaosop,dtrans-soatoaosop-struct                            \
; RUN:          -passes='require<dtrans-safetyanalyzer>,require<soatoaosop-approx>,require<soatoaosop-struct-methods>' \
; RUN:          -dtrans-soatoaosop-mem-off=3                                                      \
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

; CHECK-MOD-DAG: %__SOA_class.FieldValueMap = type { ptr, ptr, ptr, ptr }
; CHECK-MOD-DAG: %__SOA_AR_class.ValueVectorOf.0 = type { i8, i32, i32, ptr, ptr }
; CHECK-MOD-DAG: %__SOA_EL_class.FieldValueMap = type { ptr, ptr }
; CHECK-MOD-NOT: ValueVectorOf.1
%class.FieldValueMap = type { ptr, ptr, ptr, ptr }
%class.DatatypeValidator = type opaque
%class.IC_Field = type opaque
%class.RefArrayVectorOf = type opaque
%class.MemoryManager = type opaque
%class.ValueVectorOf.0 = type { i8, i32, i32, ptr, ptr }
%class.ValueVectorOf.1 = type { i8, i32, i32, ptr, ptr }

; Checks that all instructions can be dealt with.
; CHECK-TRANS: ; Checking structure's method FieldValueMap::FieldValueMap
; CHECK-TRANS: ; IR: has only expected side-effects

; Checks instructions related to transformations:
; FieldValueMap is not changed: one pointer is not used.
; CHECK-TRANS: ; Dump instructions needing update. Total = 2

; Checks transformation.

; FieldValueMap::FieldValueMap(MemoryManager *const manager)
;     : fFields(0), fValidators(0), fValues(0), fMemoryManager(manager) {}

define hidden void @"FieldValueMap::FieldValueMap(MemoryManager*)"(ptr nocapture "intel_dtrans_func_index"="1" %arg, ptr "intel_dtrans_func_index"="2" %arg1) !intel.dtrans.func.type !15 {
bb:
; CHECK-MOD:       %tmp = getelementptr inbounds %__SOA_class.FieldValueMap, ptr %arg, i64 0, i32 0
  %tmp = getelementptr inbounds %class.FieldValueMap, ptr %arg, i64 0, i32 0
; CHECK-TRANS:      ; ArrayInst: Nullptr of array
; CHECK-TRANS-NEXT:   store ptr null, ptr %tmp
; CHECK-MOD-NEXT:     store ptr null, ptr %tmp
  store ptr null, ptr %tmp, align 8
; Offset changed to 0.
; CHECK-MOD-NEXT:     %tmp2 = getelementptr inbounds %__SOA_class.FieldValueMap, ptr %arg, i64 0, i32 0
  %tmp2 = getelementptr inbounds %class.FieldValueMap, ptr %arg, i64 0, i32 1
  store ptr null, ptr %tmp2, align 8
  %tmp3 = getelementptr inbounds %class.FieldValueMap, ptr %arg, i64 0, i32 2
; CHECK-TRANS:      ; ArrayInst: Nullptr of array
; CHECK-TRANS-NEXT:   store ptr null, ptr %tmp2
; CHECK-MOD-NEXT:     store ptr null, ptr %tmp2
  store ptr null, ptr %tmp3, align 8
  %tmp4 = getelementptr inbounds %class.FieldValueMap, ptr %arg, i64 0, i32 3
  store ptr %arg1, ptr %tmp4, align 8
  ret void
}
; CHECK-TRANS:      ; Seen nullptr init.
; CHECK-TRANS-NEXT: ; Array call sites analysis result: required call sites can be merged

!intel.dtrans.types = !{!0, !1, !2, !3, !4, !9, !11}

!0 = !{!"S", %class.DatatypeValidator zeroinitializer, i32 -1}
!1 = !{!"S", %class.IC_Field zeroinitializer, i32 -1}
!2 = !{!"S", %class.RefArrayVectorOf zeroinitializer, i32 -1}
!3 = !{!"S", %class.MemoryManager zeroinitializer, i32 -1}
!4 = !{!"S", %class.ValueVectorOf.0 zeroinitializer, i32 5, !5, !6, !6, !7, !8}
!5 = !{i8 0, i32 0}
!6 = !{i32 0, i32 0}
!7 = !{%class.IC_Field zeroinitializer, i32 2}
!8 = !{%class.MemoryManager zeroinitializer, i32 1}
!9 = !{!"S", %class.ValueVectorOf.1 zeroinitializer, i32 5, !5, !6, !6, !10, !8}
!10 = !{%class.DatatypeValidator zeroinitializer, i32 2}
!11 = !{!"S", %class.FieldValueMap zeroinitializer, i32 4, !12, !13, !14, !8}
!12 = !{%class.ValueVectorOf.0 zeroinitializer, i32 1}
!13 = !{%class.ValueVectorOf.1 zeroinitializer, i32 1}
!14 = !{%class.RefArrayVectorOf zeroinitializer, i32 1}
!15 = distinct !{!16, !8}
!16 = !{%class.FieldValueMap zeroinitializer, i32 1}
