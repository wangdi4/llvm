; RUN: opt < %s -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -disable-output                                                       \
; RUN: -passes='require<dtrans-safetyanalyzer>,require<soatoaosop-approx>,require<soatoaosop-array-methods>' \
; RUN:        -dtrans-soatoaosop-base-ptr-off=3 -dtrans-soatoaosop-mem-off=4                                    \
; RUN:        -debug-only=dtrans-soatoaosop,dtrans-soatoaosop-arrays                                            \
; RUN:        -dtrans-soatoaosop-ignore-funcs=_ZN10MemManager10deallocateEPv                                   \
; RUN:        2>&1 | FileCheck %s
; RUN: opt -S < %s -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed                                                                    \
; RUN:        -passes=soatoaosop-arrays-methods-transform                                                     \
; RUN:        -dtrans-soatoaosop-base-ptr-off=3 -dtrans-soatoaosop-mem-off=4                                    \
; RUN:        -dtrans-soatoaosop-ignore-funcs=_ZN10MemManager10deallocateEPv                                   \
; RUN:        | FileCheck --check-prefix=CHECK-MOD %s
;
; RUN: opt < %s -opaque-pointers -whole-program-assume -intel-libirc-allowed -disable-output                                                       \
; RUN: -passes='require<dtrans-safetyanalyzer>,require<soatoaosop-approx>,require<soatoaosop-array-methods>' \
; RUN:        -dtrans-soatoaosop-base-ptr-off=3 -dtrans-soatoaosop-mem-off=4                                    \
; RUN:        -debug-only=dtrans-soatoaosop,dtrans-soatoaosop-arrays                                            \
; RUN:        -dtrans-soatoaosop-ignore-funcs=_ZN10MemManager10deallocateEPv                                   \
; RUN:        2>&1 | FileCheck --check-prefix=CHECK-OP %s
; RUN: opt -S < %s -opaque-pointers -whole-program-assume -intel-libirc-allowed                                                                    \
; RUN:        -passes=soatoaosop-arrays-methods-transform                                                     \
; RUN:        -dtrans-soatoaosop-base-ptr-off=3 -dtrans-soatoaosop-mem-off=4                                    \
; RUN:        -dtrans-soatoaosop-ignore-funcs=_ZN10MemManager10deallocateEPv                                   \
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

; The following method should be classified as dtor.
; Instructions to transform are shown.
; Transformed instructions are shown.
;  template <class TElem> ValueVectorOf<TElem>::~ValueVectorOf() {
;    if (fCallDestructor) {
;      for (int index = fMaxCount - 1; index >= 0; index--)
;        fElemList[index].~TElem();
;    }
;    fMemoryManager->deallocate(fElemList);
;  }
; CHECK: ; Classification: Dtor method
; CHECK: ; Dump instructions needing update. Total = 1
; CHECK-OP: ; Classification: Dtor method
; CHECK-OP: ; Dump instructions needing update. Total = 1
define void @"ValueVectorOf<IC_Field*>::~ValueVectorOf()"(%class.ValueVectorOf* "intel_dtrans_func_index"="1" %this) !intel.dtrans.func.type !9 {
entry:
; CHECK-MOD:  %fMemoryManager = getelementptr inbounds %__SOA_class.ValueVectorOf, %__SOA_class.ValueVectorOf* %this, i64 0, i32 4
; CHECK-OP-MOD:  %fMemoryManager = getelementptr inbounds %__SOA_class.ValueVectorOf, ptr %this, i64 0, i32 4
  %fMemoryManager = getelementptr inbounds %class.ValueVectorOf, %class.ValueVectorOf* %this, i64 0, i32 4
  %tmp = load %class.XMLMsgLoader*, %class.XMLMsgLoader** %fMemoryManager
; CHECK-MOD:  %fElemList2 = getelementptr inbounds %__SOA_class.ValueVectorOf, %__SOA_class.ValueVectorOf* %this, i64 0, i32 3
; CHECK-OP-MOD:  %fElemList2 = getelementptr inbounds %__SOA_class.ValueVectorOf, ptr %this, i64 0, i32 3
  %fElemList2 = getelementptr inbounds %class.ValueVectorOf, %class.ValueVectorOf* %this, i64 0, i32 3
; CHECK-MOD:  %tmp1 = bitcast %__SOA_EL_class.ValueVectorOf** %fElemList2 to i8**
; CHECK-OP-MOD:  %tmp1 = bitcast ptr %fElemList2 to ptr
  %tmp1 = bitcast %class.IC_Field*** %fElemList2 to i8**
; CHECK:     ; BasePtrInst: Load of base pointer
; CHECK-NEXT:  %tmp2 = load i8*, i8** %tmp1
; CHECK-MOD:   %tmp2 = load i8*, i8** %tmp1
; CHECK-OP:     ; BasePtrInst: Load of base pointer
; CHECK-OP-NEXT:  %tmp2 = load ptr, ptr %tmp1
; CHECK-OP-MOD:   %tmp2 = load ptr, ptr %tmp1
  %tmp2 = load i8*, i8** %tmp1
  tail call void @_ZN10MemManager10deallocateEPv(%class.XMLMsgLoader* %tmp, i8* %tmp2)
  ret void
}

define dso_local void @_ZN10MemManager10deallocateEPv(%class.XMLMsgLoader* nocapture "intel_dtrans_func_index"="1" %this, i8* "intel_dtrans_func_index"="2" %p) align 2 !intel.dtrans.func.type !11 {
entry:
  tail call void @free(i8* %p)
  ret void
}

declare !intel.dtrans.func.type !13 void @free(i8* "intel_dtrans_func_index"="1") #0

attributes #0 = { allockind("free") "alloc-family"="malloc" }

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
!11 = distinct !{!8, !12}
!12 = !{i8 0, i32 1}
!13 = distinct !{!12}
