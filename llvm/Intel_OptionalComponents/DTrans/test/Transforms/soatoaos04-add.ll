; RUN: opt < %s -whole-program-assume -disable-output                                                       \
; RUN: -passes='require<dtransanalysis>,function(require<soatoaos-approx>,require<soatoaos-array-methods>)' \
; RUN:        -dtrans-soatoaos-base-ptr-off=3 -dtrans-soatoaos-mem-off=4                                    \
; RUN:        -debug-only=dtrans-soatoaos,dtrans-soatoaos-arrays                                            \
; RUN:        -dtrans-malloc-functions=class.XMLMsgLoader,2                                                 \
; RUN:        -dtrans-free-functions=class.XMLMsgLoader,3                                                   \
; RUN:        2>&1 | FileCheck %s
; RUN: opt -S < %s -whole-program-assume                                                                    \
; RUN:        -passes=soatoaos-arrays-methods-transform                                                     \
; RUN:        -dtrans-soatoaos-base-ptr-off=3 -dtrans-soatoaos-mem-off=4                                    \
; RUN:        -dtrans-malloc-functions=class.XMLMsgLoader,2                                                 \
; RUN:        -dtrans-free-functions=class.XMLMsgLoader,3                                                   \
; RUN:        -dtrans-optbase-process-function-declaration                                                  \
; RUN:        | FileCheck --check-prefix=CHECK-MOD %s
; REQUIRES: asserts

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%class.ValueVectorOf = type { i8, i32, i32, %"class.IC_Field"**, %"class.XMLMsgLoader"* }
%class.XMLMsgLoader = type { i32 (...)** }
%class.IC_Field = type opaque
; CHECK-MOD-DAG: %__SOA_class.ValueVectorOf = type { i8, i32, i32, %__SOA_EL_class.ValueVectorOf*, %class.XMLMsgLoader* }
; CHECK-MOD-DAG: %__SOA_EL_class.ValueVectorOf = type { float*, %class.IC_Field* }

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
; CHECK-MOD: @"ValueVectorOf<IC_Field*>::addElement(IC_Field* const&){{.*}}"(%__SOA_class.ValueVectorOf* %this, %class.IC_Field** %toAdd, float** %0)
define void @"ValueVectorOf<IC_Field*>::addElement(IC_Field* const&)"(%"class.ValueVectorOf"* %this, %"class.IC_Field"** %toAdd) {
entry:
  tail call void @"ValueVectorOf<IC_Field*>::ensureExtraCapacity(unsigned int)"(%"class.ValueVectorOf"* %this, i32 1)
; CHECK-MOD:       %copy1 = bitcast float** %0 to i64*
; CHECK-MOD-NEXT:  %tmp = bitcast %class.IC_Field** %toAdd to i64*
  %tmp = bitcast %"class.IC_Field"** %toAdd to i64*
; CHECK:     ; Arg: Load from arg
; CHECK-NEXT:  %tmp1 = load i64, i64* %tmp
; CHECK-MOD-NEXT:   %copy = load i64, i64* %copy1
; CHECK-MOD-NEXT:   %tmp1 = load i64, i64* %tmp
  %tmp1 = load i64, i64* %tmp
  %fElemList = getelementptr inbounds %class.ValueVectorOf, %class.ValueVectorOf* %this, i64 0, i32 3
; CHECK:     ; BasePtrInst: Load of base pointer
; CHECK-NEXT:  %tmp2 = load %class.IC_Field**, %class.IC_Field*** %fElemList
; CHECK-MOD:        %tmp2 = load %__SOA_EL_class.ValueVectorOf*, %__SOA_EL_class.ValueVectorOf** %fElemList
  %tmp2 = load %class.IC_Field**, %class.IC_Field*** %fElemList
  %fCurCount = getelementptr inbounds %class.ValueVectorOf, %class.ValueVectorOf* %this, i64 0, i32 1
  %tmp3 = load i32, i32* %fCurCount
  %idxprom = zext i32 %tmp3 to i64
; CHECK:     ; MemInstGEP: Element set from arg
; CHECK-NEXT:  %arrayidx = getelementptr inbounds %class.IC_Field*, %class.IC_Field** %tmp2, i64 %idxprom
; CHECK-MOD:        %arrayidx = getelementptr inbounds %__SOA_EL_class.ValueVectorOf, %__SOA_EL_class.ValueVectorOf* %tmp2, i64 %idxprom
  %arrayidx = getelementptr inbounds %class.IC_Field*, %class.IC_Field** %tmp2, i64 %idxprom
; CHECK-MOD-NEXT:   %elem3 = getelementptr inbounds %__SOA_EL_class.ValueVectorOf, %__SOA_EL_class.ValueVectorOf* %arrayidx, i64 0, i32 1
; CHECK-MOD-NEXT:   %elem = getelementptr inbounds %__SOA_EL_class.ValueVectorOf, %__SOA_EL_class.ValueVectorOf* %arrayidx, i64 0, i32 0
; CHECK-MOD-NEXT:   %copy2 = bitcast float** %elem to i64*
; CHECK-MOD-NEXT:   %tmp4 = bitcast %class.IC_Field** %elem3 to i64*
  %tmp4 = bitcast %class.IC_Field** %arrayidx to i64*
; CHECK:     ; MemInst: Element set from arg
; CHECK-NEXT:  store i64 %tmp1, i64* %tmp4
; CHECK-MOD-NEXT:   store i64 %copy, i64* %copy2
; CHECK-MOD-NEXT:   store i64 %tmp1, i64* %tmp4
  store i64 %tmp1, i64* %tmp4
  %inc = add i32 %tmp3, 1
  store i32 %inc, i32* %fCurCount
  ret void
}

declare void @"ValueVectorOf<IC_Field*>::ensureExtraCapacity(unsigned int)"(%"class.ValueVectorOf"*, i32)
