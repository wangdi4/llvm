; RUN: opt < %s -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -disable-output                                                       \
; RUN: -passes='require<dtrans-safetyanalyzer>,require<soatoaosop-approx>,require<soatoaosop-array-methods>' \
; RUN:        -dtrans-soatoaosop-base-ptr-off=3 -dtrans-soatoaosop-mem-off=4                                    \
; RUN:        -debug-only=dtrans-soatoaosop,dtrans-soatoaosop-arrays                                            \
; RUN:        2>&1 | FileCheck %s
; RUN: opt -S < %s -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed                                                                    \
; RUN:        -passes=soatoaosop-arrays-methods-transform                                                     \
; RUN:        -dtrans-soatoaosop-base-ptr-off=3 -dtrans-soatoaosop-mem-off=4                                    \
; RUN:        -dtransop-optbase-process-function-declaration                                                  \
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
; RUN:        -dtransop-optbase-process-function-declaration                                                  \
; RUN:        | FileCheck --check-prefix=CHECK-OP-MOD %s
; REQUIRES: asserts

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%class.ValueVectorOf = type { i8, i32, i32, %"class.IC_Field"**, %"class.XMLMsgLoader"* }
%class.XMLMsgLoader = type { i32 (...)** }
%class.IC_Field = type opaque
; CHECK-MOD-DAG: %__SOA_class.ValueVectorOf = type { i8, i32, i32, %__SOA_EL_class.ValueVectorOf*, %class.XMLMsgLoader* }
; CHECK-MOD-DAG: %__SOA_EL_class.ValueVectorOf = type { float*, %class.IC_Field* }
; CHECK-OP-MOD-DAG: %__SOA_class.ValueVectorOf = type { i8, i32, i32, ptr, ptr }
; CHECK-OP-MOD-DAG: %__SOA_EL_class.ValueVectorOf = type { ptr, ptr }

; The following method should be classified as append-like.
; Instructions to transform are shown.
;  template <class TElem>
;  void ValueVectorOf<TElem>::addElement(const TElem &toAdd) {
;    ensureExtraCapacity(1);
;    fElemList[fCurCount] = toAdd;
;    fCurCount++;
;  }
; CHECK: ; Classification: Append element method
; CHECK: ; Dump instructions needing update. Total = 4
; CHECK-MOD: define internal void @"ValueVectorOf<IC_Field*>::addElement(IC_Field* const&).{{[0-9]+}}"(%__SOA_class.ValueVectorOf* "intel_dtrans_func_index"="1" %this, %class.IC_Field** "intel_dtrans_func_index"="2" %toAdd, float** "intel_dtrans_func_index"="3" %0) !intel.dtrans.func.type
; CHECK-OP: ; Classification: Append element method
; CHECK-OP: ; Dump instructions needing update. Total = 4
; CHECK-OP-MOD: define internal void @"ValueVectorOf<IC_Field*>::addElement(IC_Field* const&).{{[0-9]+}}"(ptr "intel_dtrans_func_index"="1" %this, ptr "intel_dtrans_func_index"="2" %toAdd, ptr "intel_dtrans_func_index"="3" %0) !intel.dtrans.func.type
define void @"ValueVectorOf<IC_Field*>::addElement(IC_Field* const&)"(%"class.ValueVectorOf"* "intel_dtrans_func_index"="1" %this, %"class.IC_Field"** "intel_dtrans_func_index"="2" %toAdd) !intel.dtrans.func.type !9 {
entry:
  tail call void @"ValueVectorOf<IC_Field*>::ensureExtraCapacity(unsigned int)"(%"class.ValueVectorOf"* %this, i32 1)
; CHECK-MOD:       %1 = bitcast float** %0 to i64*
; CHECK-MOD-NEXT:  %tmp = bitcast %class.IC_Field** %toAdd to i64*
; CHECK-OP-MOD:       %1 = bitcast ptr %0 to ptr
; CHECK-OP-MOD-NEXT:  %tmp = bitcast ptr %toAdd to ptr
  %tmp = bitcast %"class.IC_Field"** %toAdd to i64*
; CHECK:     ; Arg: Load from arg
; CHECK-NEXT:  %tmp1 = load i64, i64* %tmp
; CHECK-MOD-NEXT:   %copy = load i64, i64* %1
; CHECK-MOD-NEXT:   %tmp1 = load i64, i64* %tmp
; CHECK-OP:     ; Arg: Load from arg
; CHECK-OP-NEXT:  %tmp1 = load i64, ptr %tmp
; CHECK-OP-MOD-NEXT:   %copy = load i64, ptr %1
; CHECK-OP-MOD-NEXT:   %tmp1 = load i64, ptr %tmp
  %tmp1 = load i64, i64* %tmp
  %fElemList = getelementptr inbounds %class.ValueVectorOf, %class.ValueVectorOf* %this, i64 0, i32 3
; CHECK:     ; BasePtrInst: Load of base pointer
; CHECK-NEXT:  %tmp2 = load %class.IC_Field**, %class.IC_Field*** %fElemList
; CHECK-MOD:        %tmp2 = load %__SOA_EL_class.ValueVectorOf*, %__SOA_EL_class.ValueVectorOf** %fElemList
; CHECK-OP:     ; BasePtrInst: Load of base pointer
; CHECK-OP-NEXT:  %tmp2 = load ptr, ptr %fElemList
; CHECK-OP-MOD:   %tmp2 = load ptr, ptr %fElemList
  %tmp2 = load %class.IC_Field**, %class.IC_Field*** %fElemList
  %fCurCount = getelementptr inbounds %class.ValueVectorOf, %class.ValueVectorOf* %this, i64 0, i32 1
  %tmp3 = load i32, i32* %fCurCount
  %idxprom = zext i32 %tmp3 to i64
; CHECK:     ; MemInstGEP: Element set from arg
; CHECK-NEXT:  %arrayidx = getelementptr inbounds %class.IC_Field*, %class.IC_Field** %tmp2, i64 %idxprom
; CHECK-MOD:        %arrayidx = getelementptr inbounds %__SOA_EL_class.ValueVectorOf, %__SOA_EL_class.ValueVectorOf* %tmp2, i64 %idxprom
; CHECK-OP:     ; MemInstGEP: Element set from arg
; CHECK-OP-NEXT:  %arrayidx = getelementptr inbounds ptr, ptr %tmp2, i64 %idxprom
; CHECK-OP-MOD:        %arrayidx = getelementptr inbounds %__SOA_EL_class.ValueVectorOf, ptr %tmp2, i64 %idxprom
  %arrayidx = getelementptr inbounds %class.IC_Field*, %class.IC_Field** %tmp2, i64 %idxprom
; CHECK-MOD-NEXT:   %elem1 = getelementptr inbounds %__SOA_EL_class.ValueVectorOf, %__SOA_EL_class.ValueVectorOf* %arrayidx, i64 0, i32 1
; CHECK-MOD-NEXT:   %elem = getelementptr inbounds %__SOA_EL_class.ValueVectorOf, %__SOA_EL_class.ValueVectorOf* %arrayidx, i64 0, i32 0
; CHECK-MOD-NEXT:   %2 = bitcast float** %elem to i64*
; CHECK-MOD-NEXT:   %tmp4 = bitcast %class.IC_Field** %elem1 to i64*
; CHECK-OP-MOD-NEXT:   %elem1 = getelementptr inbounds %__SOA_EL_class.ValueVectorOf, ptr %arrayidx, i64 0, i32 1
; CHECK-OP-MOD-NEXT:   %elem = getelementptr inbounds %__SOA_EL_class.ValueVectorOf, ptr %arrayidx, i64 0, i32 0
; CHECK-OP-MOD-NEXT:   %2 = bitcast ptr %elem to ptr
; CHECK-OP-MOD-NEXT:   %tmp4 = bitcast ptr %elem1 to ptr
  %tmp4 = bitcast %class.IC_Field** %arrayidx to i64*
; CHECK:     ; MemInst: Element set from arg
; CHECK-NEXT:  store i64 %tmp1, i64* %tmp4
; CHECK-MOD-NEXT:   store i64 %copy, i64* %2
; CHECK-MOD-NEXT:   store i64 %tmp1, i64* %tmp4
; CHECK-OP:     ; MemInst: Element set from arg
; CHECK-OP-NEXT:  store i64 %tmp1, ptr %tmp4
; CHECK-OP-MOD-NEXT:   store i64 %copy, ptr %2
; CHECK-OP-MOD-NEXT:   store i64 %tmp1, ptr %tmp4
  store i64 %tmp1, i64* %tmp4
  %inc = add i32 %tmp3, 1
  store i32 %inc, i32* %fCurCount
  ret void
}

declare !intel.dtrans.func.type !11 void @"ValueVectorOf<IC_Field*>::ensureExtraCapacity(unsigned int)"(%class.ValueVectorOf* "intel_dtrans_func_index"="1", i32)

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
!9 = distinct !{!10, !7}
!10 = !{%class.ValueVectorOf zeroinitializer, i32 1}
!11 = distinct !{!10}
