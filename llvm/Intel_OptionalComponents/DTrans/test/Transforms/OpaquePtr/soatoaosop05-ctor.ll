; RUN: opt < %s -whole-program-assume -intel-libirc-allowed -disable-output -debug-only=dtrans-soatoaosop-deps  \
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

; Checks that all instructions can be dealt with.
; CHECK-TRANS: ; Checking structure's method FieldValueMap::FieldValueMap(
; CHECK-TRANS: ; IR: has only expected side-effects

; Checks instructions related to transformations:
; FieldValueMap is not changed: one pointer is not used.
; CHECK-TRANS: ; Dump instructions needing update. Total = 1

; Checks transformation. Only types change.

; FieldValueMap::FieldValueMap(MemoryManager *const manager)
;     : fFields(0), fValidators(0), fValues(0), fMemoryManager(manager) {}

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

define hidden void @"FieldValueMap::FieldValueMap(MemoryManager*)"(ptr "intel_dtrans_func_index"="1" %this, ptr "intel_dtrans_func_index"="2" %manager) !intel.dtrans.func.type !20 {
entry:
; CHECK-MOD:  %fMemoryManager = getelementptr inbounds %__SOA_class.FieldValueMap, ptr %this, i64 0, i32 3
  %fMemoryManager = getelementptr inbounds %class.FieldValueMap, ptr %this, i64 0, i32 3
  %tmp = bitcast ptr %this to ptr
; CHECK-TRANS:      ; ArrayInst: Nullptr with memset of array
; CHECK-TRANS-NEXT:   call void @llvm.memset.p0.i64(ptr %tmp, i8 0, i64 24, i1 false)
; Layout did not change, no changes in size argument of memset.
; CHECK-MOD:  call void @llvm.memset.p0.i64(ptr %tmp, i8 0, i64 24, i1 false)
  call void @llvm.memset.p0.i64(ptr %tmp, i8 0, i64 24, i1 false)
  store ptr %manager, ptr %fMemoryManager, align 8
  ret void
}

; Function Attrs: nocallback nofree nounwind willreturn memory(argmem: write)
declare void @llvm.memset.p0.i64(ptr nocapture writeonly, i8, i64, i1 immarg) #0

attributes #0 = { nocallback nofree nounwind willreturn memory(argmem: write) }

; CHECK-TRANS: ; Seen nullptr init with memset.
; XCHECK-DEP: Deps computed: 7, Queries: 10

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
!20 = distinct !{!21, !9}
!21 = !{%class.FieldValueMap zeroinitializer, i32 1}
